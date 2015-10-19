/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Render/2D/Sprite.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Render/RenderManager.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"
#include "Render/2D/TextBlock.h"
#include "Core/Core.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/TextBlockSoftwareRender.h"
#include "Render/2D/TextBlockGraphicsRender.h"
#include "Render/2D/TextBlockDistanceRender.h"

#include "Utils/StringUtils.h"
#include "Concurrency/LockGuard.h"
#include "fribidi/fribidi-bidi-types.h"
#include "fribidi/fribidi-unicode.h"
#include "TextLayout.h"


#include "UI/UIControlSystem.h"

namespace DAVA 
{

bool TextBlock::isBiDiSupportEnabled = true;    //!< Enable BiDi support by default

struct TextBlockData
{
    TextBlockData(): font(NULL) { };
    ~TextBlockData() { SafeRelease(font); };

    Font *font;
};

static Set<TextBlock*> registredBlocks;

#define NEW_RENDER 1

void RegisterTextBlock(TextBlock *tbl)
{
	registredBlocks.insert(tbl);
}

void UnregisterTextBlock(TextBlock *tbl)
{
	registredBlocks.erase(tbl);
}

void TextBlock::ScreenResolutionChanged()
{
	Logger::FrameworkDebug("Regenerate text blocks");
    for (auto textBlock : registredBlocks)
    {
        textBlock->NeedPrepare();
    }
}

TextBlock * TextBlock::Create(const Vector2 & size)
{
    TextBlock * textSprite = new TextBlock();
    textSprite->SetRectSize(size);
    return textSprite;
}

TextBlock::TextBlock()
    : scale(1.f, 1.f)
    , cacheFinalSize(0.f, 0.f)
    , cacheTextSize(0.f, 0.f)
    , renderSize(1.f)
    , cacheDx(0)
    , cacheDy(0)
    , cacheW(0)
    , cacheOx(0)
    , cacheOy(0)
    , textureForInvalidation(NULL)
    , angle(0.f)
    , needCalculateCacheParams(false)
{
    font = NULL;
    isMultilineEnabled = false;
    useRtlAlign = RTL_DONT_USE;
    fittingType = FITTING_DISABLED;

	originalFontSize = 0.1f;
	align = ALIGN_HCENTER|ALIGN_VCENTER;
	RegisterTextBlock(this);

	isMultilineBySymbolEnabled = false;
    treatMultilineAsSingleLine = false;
    isRtl = false;
    
	textBlockRender = NULL;
	needPrepareInternal = false;
    textureInvalidater = NULL;
#if defined(LOCALIZATION_DEBUG)
    fittingTypeUsed = FITTING_DISABLED;
    visualTextCroped = false;
#endif //LOCALIZATION_DEBUG
}

TextBlock::TextBlock(const TextBlock& src)
    : font(nullptr)
    , textureForInvalidation(nullptr)
    , textBlockRender(nullptr)
    , textureInvalidater(nullptr)
    //Basic params
    , scale(src.scale)
    , rectSize(src.rectSize)
    , isMultilineEnabled(src.isMultilineEnabled)
    , isMultilineBySymbolEnabled(src.isMultilineBySymbolEnabled)
    , align(src.align)
    , fittingType(src.fittingType)
    , useRtlAlign(src.useRtlAlign)
    , angle(src.angle)
    , pivot(src.pivot)
    //SetText(...) equivalent
    , logicalText(src.logicalText)
    , requestedSize(src.requestedSize)
    //Cache params
    , visualText(src.visualText)
    , isRtl(src.isRtl)
    , cacheFinalSize(src.cacheFinalSize)
    , cacheW(src.cacheW)
    , cacheDx(src.cacheDx)
    , cacheDy(src.cacheDy)
    , cacheOx(src.cacheOx)
    , cacheOy(src.cacheOy)
    , cacheSpriteOffset(src.cacheSpriteOffset)
    , cacheTextSize(src.cacheTextSize)
    , renderSize(src.renderSize)
    , multilineStrings(src.multilineStrings)
    , stringSizes(src.stringSizes)
    , isPredrawed(src.isPredrawed)
    , cacheUseJustify(src.cacheUseJustify)
    , treatMultilineAsSingleLine(src.treatMultilineAsSingleLine)
    , needCalculateCacheParams(src.needCalculateCacheParams)
    , needPrepareInternal(src.needPrepareInternal)
#if defined(LOCALIZATION_DEBUG)
    , fittingTypeUsed(src.fittingTypeUsed)
    , visualTextCroped(src.visualTextCroped)
#endif //LOCALIZATION_DEBUG
{
    //SetFont without Prepare
    if (nullptr != src.font)
    {
        SetFontInternal(src.font);
    }

    RegisterTextBlock(this);
}

TextBlock::~TextBlock()
{
	SafeRelease(textureForInvalidation);
    SafeRelease(textBlockRender);
    SafeDelete(textureInvalidater);
    SafeRelease(font);
    UnregisterTextBlock(this);
}

// Setters // Getters

void TextBlock::SetFontInternal(Font* _font)
{
    SafeRelease(font);
    font = SafeRetain(_font);

    originalFontSize = font->GetSize();
    renderSize = originalFontSize;

    SafeRelease(textBlockRender);
    SafeDelete(textureInvalidater);
    switch (font->GetFontType())
    {
    case Font::TYPE_FT:
        textBlockRender = new TextBlockSoftwareRender(this);
        textureInvalidater = new TextBlockSoftwareTexInvalidater(this);
        break;
    case Font::TYPE_GRAPHICAL:
        textBlockRender = new TextBlockGraphicsRender(this);
        break;
    case Font::TYPE_DISTANCE:
        textBlockRender = new TextBlockDistanceRender(this);
        break;

    default:
        DVASSERT(!"Unknown font type");
        break;
    }
}

void TextBlock::SetFont(Font* _font)
{
    if (_font && _font != font)
    {
        SetFontInternal(_font);
        NeedPrepare();
    }
}

void TextBlock::SetRectSize(const Vector2& size)
{
    if (rectSize != size)
    {
        rectSize = size;
        NeedPrepare();
    }
}

void TextBlock::SetScale(const Vector2 & _scale)
{
    if (scale != _scale)
    {
        scale = _scale;
        NeedPrepare();
    }
}

void TextBlock::SetText(const WideString & _string, const Vector2 &requestedTextRectSize)
{
    if (logicalText != _string || requestedSize != requestedTextRectSize)
    {
        requestedSize = requestedTextRectSize;
        logicalText = _string;
        NeedPrepare();
    }
}

void TextBlock::SetMultiline(bool _isMultilineEnabled, bool bySymbol)
{
    if (isMultilineEnabled != _isMultilineEnabled || isMultilineBySymbolEnabled != bySymbol)
    {
        isMultilineBySymbolEnabled = bySymbol;
		isMultilineEnabled = _isMultilineEnabled;
        NeedPrepare();
    }
}

void TextBlock::SetFittingOption(int32 _fittingType)
{
    if (fittingType != _fittingType)
    {
        fittingType = _fittingType;
        NeedPrepare();
    }
}

void TextBlock::SetRenderSize(float32 _renderSize)
{
    if (renderSize != _renderSize)
    {
        renderSize = Max(_renderSize, 0.1f);
        NeedPrepare();
    }
}

void TextBlock::SetAlign(int32 _align)
{
    if (align != _align)
    {
        align = _align;
        NeedPrepare();
    }
}

void TextBlock::SetUseRtlAlign(eUseRtlAlign _useRtlAlign)
{
    if (useRtlAlign != _useRtlAlign)
    {
        useRtlAlign = _useRtlAlign;
        NeedPrepare();
    }
}

const Vector<WideString>& TextBlock::GetMultilineStrings()
{
    CalculateCacheParamsIfNeed();
    return multilineStrings;
}

const WideString& TextBlock::GetVisualText()
{
    CalculateCacheParamsIfNeed();
    return visualText;
}

float32 TextBlock::GetRenderSize()
{
    CalculateCacheParamsIfNeed();
    return renderSize;
}

const Vector2& TextBlock::GetTextSize()
{
    CalculateCacheParamsIfNeed();
    return cacheTextSize;
}

const Vector<int32>& TextBlock::GetStringSizes()
{
    CalculateCacheParamsIfNeed();
    return stringSizes;
}

const Vector2& TextBlock::GetSpriteOffset()
{
    CalculateCacheParamsIfNeed();
    return cacheSpriteOffset;
}

bool TextBlock::IsRtl()
{
    CalculateCacheParamsIfNeed();
    return isRtl;
}

int32 TextBlock::GetVisualAlign()
{
    CalculateCacheParamsIfNeed();

    if (((align & (ALIGN_LEFT | ALIGN_RIGHT)) != 0) &&
        ((useRtlAlign == RTL_USE_BY_CONTENT && isRtl) ||
         (useRtlAlign == RTL_USE_BY_SYSTEM && UIControlSystem::Instance()->IsRtl())))
    {
        // Mirror left/right align
        return align ^ (ALIGN_LEFT | ALIGN_RIGHT);
    }
    return align;
}

#if defined(LOCALIZATION_DEBUG)
int32 TextBlock::GetFittingOptionUsed()
{
    CalculateCacheParamsIfNeed();
    return fittingTypeUsed;
}

bool TextBlock::IsVisualTextCroped()
{
    CalculateCacheParamsIfNeed();
    return visualTextCroped;
}
#endif

Vector2 TextBlock::GetPreferredSizeForWidth(float32 width)
{
    if (!font)
        return Vector2();

    Vector2 result;
    if (requestedSize.dx < 0.0f && requestedSize.dy < 0.0f && fittingType == FITTING_DISABLED)
    {
        CalculateCacheParamsIfNeed();
        result = cacheTextSize;
    }
    else
    {
        Vector2 oldRequestedSize = requestedSize;
        int32 oldFitting = fittingType;

        requestedSize = Vector2(width, -1.0f);
        fittingType = FITTING_DISABLED;
        CalculateCacheParams();

        result = cacheTextSize;

        requestedSize = oldRequestedSize;
        fittingType = oldFitting;
        CalculateCacheParams();
    }

    return result;
}

Sprite * TextBlock::GetSprite()
{
    if (textBlockRender)
    {
        return textBlockRender->GetSprite();
    }
    return nullptr;
}

void TextBlock::ForcePrepare(Texture* texture)
{
    NeedPrepare(texture);
}

void TextBlock::NeedPrepare(Texture* texture /*=NULL*/)
{
    if (texture != textureForInvalidation)
    {
        SafeRelease(textureForInvalidation);
        textureForInvalidation = SafeRetain(texture);
    }
    needCalculateCacheParams = true;
    needPrepareInternal = true;
}
	
void TextBlock::PrepareInternal()
{
	DVASSERT(Thread::IsMainThread());

    needPrepareInternal = false;
    if (textBlockRender)
    {
        font->SetSize(renderSize);
        textBlockRender->Prepare(textureForInvalidation);
        font->SetSize(originalFontSize);

        SafeRelease(textureForInvalidation);
    }
}

void TextBlock::CalculateCacheParams()
{
    needCalculateCacheParams = false;
    stringSizes.clear();
    multilineStrings.clear();

#if defined(LOCALIZATION_DEBUG)
    fittingTypeUsed = FITTING_DISABLED;
    visualTextCroped = false;
#endif

    if (logicalText.empty() || font == nullptr)
    {
        visualText.clear();
        isRtl = false;
        cacheFinalSize = Vector2(0.f,0.f);
        cacheW = 0;
        cacheDx = 0;
        cacheDy = 0;
        cacheOx = 0;
        cacheOy = 0;
        cacheSpriteOffset = Vector2(0.f,0.f);
        cacheTextSize = Vector2(0.f,0.f);
        return;
    }

    Vector2 drawSize = rectSize;
    if (requestedSize.dx > 0)
    {
        drawSize.x = requestedSize.dx;
    }
    if (requestedSize.dy > 0)
    {
        drawSize.y = requestedSize.dy;
    }
    bool useJustify = ((align & ALIGN_HJUSTIFY) != 0);

    renderSize = originalFontSize * scale.y;
    font->SetSize(renderSize);

    TextLayout textLayout(isBiDiSupportEnabled);
    textLayout.Reset(logicalText);
    isRtl = textLayout.IsRtlText();
    visualText = textLayout.GetVisualText(false);

    Vector<float32> charSizes;
    Font::StringMetrics textMetrics = font->GetStringMetrics(visualText, &charSizes);

    if (visualText != textLayout.GetPreparedText())
    {
        textLayout.CalculateCharSizes(*font);
    }
    else
    {
        textLayout.SetCharSizes(charSizes);
    }

    // This is a temporary fix to correctly handle long multiline texts
    // which can't be broken to the separate lines.
    if (isMultilineEnabled)
    {
        // We can wrap by symbols because it's only check that the text placed in a single line
        textLayout.NextBySymbols(drawSize.dx);
        treatMultilineAsSingleLine = textLayout.IsEndOfText();
    }

    if(!isMultilineEnabled || treatMultilineAsSingleLine)
    {
        for (float32& val : charSizes)
        {
            val = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(val);
        }
        
        WideString pointsStr;
        if ((fittingType & FITTING_POINTS) && (drawSize.x < textMetrics.width))
        {
            uint32 length = charSizes.size();
            Font::StringMetrics pointsMetric = font->GetStringMetrics(L"...");
            float32 fullWidth = static_cast<float32>(textMetrics.width + pointsMetric.width);
            for (uint32 i = length - 1; i > 0U; --i)
            {
                if(fullWidth <= drawSize.x)
                {
#if defined(LOCALIZATION_DEBUG)
                    fittingTypeUsed = FITTING_POINTS;
#endif
                    pointsStr.clear();
                    pointsStr.append(visualText, 0, i + 1);
                    pointsStr += L"...";
                    break;
                }
                fullWidth -= charSizes[i];
            }
        }
        else if (!((fittingType & FITTING_REDUCE) || (fittingType & FITTING_ENLARGE)) && (drawSize.x < textMetrics.width) && (requestedSize.x >= 0))
        {
            uint32 length = charSizes.size();
            float32 fullWidth = static_cast<float32>(textMetrics.width);
            if(ALIGN_RIGHT & align)
            {
                for(uint32 i = 0U; i < length; ++i)
                {
                    if(fullWidth <= drawSize.x)
                    {
                        pointsStr.clear();
                        pointsStr.append(visualText, i, length - i);
                        break;
                    }
                    fullWidth -= charSizes[i];
                }
            }
            else if(ALIGN_HCENTER & align)
            {
                uint32 left = 0U;
                uint32 right = length - 1;
                bool cutFromBegin = false;
                
                while (left != right)
                {
                    if (fullWidth <= drawSize.x)
                    {
                        pointsStr.clear();
                        pointsStr.append(visualText, left, right - left + 1);
                        break;
                    }

                    if (cutFromBegin)
                    {
                        fullWidth -= charSizes[left++];
                    }
                    else
                    {
                        fullWidth -= charSizes[right--];
                    }
                    cutFromBegin = !cutFromBegin;
                }
            }
            else if (ALIGN_LEFT & align)
            {
                for (uint32 i = 1U; i < length; ++i)
                {
                    fullWidth -= charSizes[length - i];
                    if (fullWidth <= drawSize.x)
                    {
                        pointsStr.clear();
                        pointsStr.append(visualText, 0, length - i);
                        break;
                    }
                }
            }
        }
        else if(((fittingType & FITTING_REDUCE) || (fittingType & FITTING_ENLARGE)) && (requestedSize.dy >= 0 || requestedSize.dx >= 0))
        {
            bool isChanged = false;
            float32 prevFontSize = renderSize;
            while (true)
            {
                float32 yMul = 1.0f;
                float32 xMul = 1.0f;

                bool xBigger = false;
                bool xLower = false;
                bool yBigger = false;
                bool yLower = false;
                if(requestedSize.dy >= 0)
                {
                    if ((isChanged || fittingType & FITTING_REDUCE) && textMetrics.height > drawSize.y)
                    {
                        if (prevFontSize < renderSize)
                        {
                            renderSize = prevFontSize;
                            font->SetSize(renderSize);
                            textMetrics = font->GetStringMetrics(visualText);
                            break;
                        }
                        yBigger = true;
                        yMul = drawSize.y / textMetrics.height;
                    }
                    else if ((isChanged || fittingType & FITTING_ENLARGE) && textMetrics.height < drawSize.y * 0.9)
                    {
                        yLower = true;
                        yMul = (drawSize.y * 0.9f) / textMetrics.height;
                        if(yMul < 1.01f)
                        {
                            yLower = false;
                        }
                    }
                }

                if(requestedSize.dx >= 0)
                {
                    if ((isChanged || fittingType & FITTING_REDUCE) && textMetrics.width > drawSize.x)
                    {
                        if (prevFontSize < renderSize)
                        {
                            renderSize = prevFontSize;
                            font->SetSize(renderSize);
                            textMetrics = font->GetStringMetrics(visualText);
                            break;
                        }
                        xBigger = true;
                        xMul = drawSize.x / textMetrics.width;
                    }
                    else if ((isChanged || fittingType & FITTING_ENLARGE) && textMetrics.width < drawSize.x * 0.95)
                    {
                        xLower = true;
                        xMul = (drawSize.x * 0.95f) / textMetrics.width;
                        if(xMul < 1.01f)
                        {
                            xLower = false;
                        }
                    }
                }


                if (((!xBigger && !yBigger) && (!xLower || !yLower)) || FLOAT_EQUAL(renderSize, 0.f))
                {
                    break;
                }

                float32 finalSize = renderSize;
                prevFontSize = finalSize;
                isChanged = true;
                if(xMul < yMul)
                {
                    finalSize *= xMul;
                }
                else
                {
                    finalSize *= yMul;
                }
#if defined(LOCALIZATION_DEBUG)
                {
                    float mult = DAVA::Min(xMul, yMul);
                    if (mult > 1.0f)
                        fittingTypeUsed |= FITTING_ENLARGE;
                    else if (mult < 1.0f)
                        fittingTypeUsed |= FITTING_REDUCE;
                }
#endif
                renderSize = finalSize;
                font->SetSize(renderSize);
                textMetrics = font->GetStringMetrics(visualText);
            }
        }

        if(!pointsStr.empty())
        {
            visualText = pointsStr;
            textMetrics = font->GetStringMetrics(visualText);
#if defined(LOCALIZATION_DEBUG)
            visualTextCroped = true;
#endif
        }

        if (treatMultilineAsSingleLine)
        {
            // Another temporary solution to return correct multiline strings/
            // string sizes.
            multilineStrings.clear();
            stringSizes.clear();
            multilineStrings.push_back(visualText);
            stringSizes.push_back(textMetrics.width);
        }
    }
    else //if(!isMultilineEnabled)
    {
        int32 yOffset = font->GetVerticalSpacing();
        int32 fontHeight = 0;
        textMetrics.width = textMetrics.drawRect.dx = 0;

        multilineStrings.clear();
        textLayout.Seek(0);
        textLayout.FillList(multilineStrings, drawSize.dx, isMultilineBySymbolEnabled, true);
        fontHeight = font->GetFontHeight() + yOffset;
        textMetrics.height = textMetrics.drawRect.dy = fontHeight * (int32)multilineStrings.size() - yOffset;

        if (fittingType && (requestedSize.dy >= 0/* || requestedSize.dx >= 0*/) && visualText.size() > 3)
        {
            float32 lastSize = renderSize;
            bool isChanged = false;
            while (true)
            {
                float32 yMul = 1.0f;

                bool yBigger = false;
                bool yLower = false;
                if (requestedSize.dy >= 0)
                {
                    if ((isChanged || fittingType & FITTING_REDUCE) && textMetrics.height > drawSize.y)
                    {
                        yBigger = true;
                        yMul = drawSize.y / textMetrics.height;
                        if (lastSize < renderSize)
                        {
                            renderSize = lastSize;
                            font->SetSize(renderSize);

                            multilineStrings.clear();
                            textLayout.CalculateCharSizes(*font);
                            textLayout.Seek(0);
                            textLayout.FillList(multilineStrings, drawSize.dx, isMultilineBySymbolEnabled, true);
                            fontHeight = font->GetFontHeight() + yOffset;
                            textMetrics.height = textMetrics.drawRect.dy = fontHeight * (int32)multilineStrings.size() - yOffset;
                            break;
                        }
                    }
                    else if ((isChanged || fittingType & FITTING_ENLARGE) && textMetrics.height < drawSize.y * 0.95)
                    {
                        yLower = true;
                        if (textMetrics.height < drawSize.y * 0.75f)
                        {
                            yMul = (drawSize.y * 0.75f) / textMetrics.height;
                        }
                        else if (textMetrics.height < drawSize.y * 0.8f)
                        {
                            yMul = (drawSize.y * 0.8f) / textMetrics.height;
                        }
                        else if (textMetrics.height < drawSize.y * 0.85f)
                        {
                            yMul = (drawSize.y * 0.85f) / textMetrics.height;
                        }
                        else if (textMetrics.height < drawSize.y * 0.9f)
                        {
                            yMul = (drawSize.y * 0.9f) / textMetrics.height;
                        }
                        else
                        {
                            yMul = (drawSize.y * 0.95f) / textMetrics.height;
                        }
                        if (FLOAT_EQUAL(yMul, 1.0f))
                        {
                            yMul = 1.05f;
                        }
                    }
                }

                if ((!yBigger && !yLower) || FLOAT_EQUAL(renderSize, 0.f))
                {
                    break;
                }

                float32 finalSize = lastSize = renderSize;
                isChanged = true;
                finalSize *= yMul;

#if defined(LOCALIZATION_DEBUG)
                if (yMul > 1.0f)
                {
                    fittingTypeUsed |= FITTING_ENLARGE;
                }
                if (yMul < 1.0f)
                {
                    fittingTypeUsed |= FITTING_REDUCE;
                }
#endif

                renderSize = finalSize;
                font->SetSize(renderSize);

                multilineStrings.clear();
                textLayout.CalculateCharSizes(*font);
                textLayout.Seek(0);
                textLayout.FillList(multilineStrings, drawSize.dx, isMultilineBySymbolEnabled, true);
                fontHeight = font->GetFontHeight() + yOffset;
                textMetrics.height = textMetrics.drawRect.dy = fontHeight * (int32)multilineStrings.size() - yOffset;
            };
        }

        if (textMetrics.height > drawSize.y && requestedSize.y >= 0.f)
        {
            int32 needLines = Min((int32)multilineStrings.size(), (int32)ceilf(drawSize.y / fontHeight) + 1);
            Vector<WideString> oldLines;
            multilineStrings.swap(oldLines);
            if(align & ALIGN_TOP)
            {
                multilineStrings.assign(oldLines.begin(), oldLines.begin() + needLines);
            }
            else if(align & ALIGN_VCENTER)
            {
                int32 startIndex = ((int32)oldLines.size() - needLines + 1) / 2;
                multilineStrings.assign(oldLines.begin() + startIndex, oldLines.begin() + startIndex + needLines);
            }
            else //if(ALIGN_BOTTOM)
            {
                int32 startIndex = (int32)oldLines.size() - needLines;
                multilineStrings.assign(oldLines.begin() + startIndex, oldLines.end());
            }
            textMetrics.height = textMetrics.drawRect.dy = fontHeight * (int32)multilineStrings.size() - yOffset;
        }

        stringSizes.reserve(multilineStrings.size());
        for (int32 line = 0; line < (int32)multilineStrings.size(); ++line)
        {
            Font::StringMetrics stringSize = font->GetStringMetrics(multilineStrings[line]);
            stringSizes.push_back(stringSize.width);

            textMetrics.drawRect.dx = Max(textMetrics.drawRect.dx, stringSize.drawRect.dx + stringSize.drawRect.x);
            textMetrics.drawRect.x = Min(textMetrics.drawRect.x, stringSize.drawRect.x);

            if (requestedSize.dx >= 0)
            {
                textMetrics.width = Max(textMetrics.width, Min(stringSize.width, (int32)drawSize.x));
            }
            else
            {
                textMetrics.width = Max(textMetrics.width, stringSize.width);
            }

            if(0 == line)
            {
                textMetrics.drawRect.y = stringSize.drawRect.y;
            }

#if defined(LOCALIZATION_DEBUG)
            if (textMetrics.width < stringSize.width)
            {
                visualTextCroped = true;
            }
#endif
        }
        // Translate right/bottom edge to width/height
        textMetrics.drawRect.dx -= textMetrics.drawRect.x;
        textMetrics.drawRect.dy -= textMetrics.drawRect.y;
    }

    if (requestedSize.dx >= 0 && useJustify)
    {
        textMetrics.drawRect.dx = Max(textMetrics.drawRect.dx, (int)drawSize.dx);
    }

    //calculate texture size
    int32 dx = (int32)ceilf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX((float32)textMetrics.drawRect.dx));
    int32 dy = (int32)ceilf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY((float32)textMetrics.drawRect.dy));
    int32 ox = (int32)ceilf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX((float32)textMetrics.drawRect.x));
    int32 oy = (int32)ceilf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY((float32)textMetrics.drawRect.y));

    cacheUseJustify = useJustify;
    cacheDx = dx;
    EnsurePowerOf2(cacheDx);

    cacheDy = dy;
    EnsurePowerOf2(cacheDy);

    cacheOx = ox;
    cacheOy = oy;

    cacheW = textMetrics.drawRect.dx;
    cacheFinalSize.x = (float32)textMetrics.drawRect.dx;
    cacheFinalSize.y = (float32)textMetrics.drawRect.dy;
    cacheTextSize = Vector2((float32)textMetrics.width, (float32)textMetrics.height);

    // Align sprite offset
    if(align & ALIGN_RIGHT)
    {
        cacheSpriteOffset.x = (float32)(textMetrics.drawRect.dx - textMetrics.width + textMetrics.drawRect.x);
    }
    else if(align & ALIGN_HCENTER)
    {
        cacheSpriteOffset.x = ((float32)(textMetrics.drawRect.dx - textMetrics.width) * 0.5f + textMetrics.drawRect.x);
    }
    else
    {
        cacheSpriteOffset.x = (float32)textMetrics.drawRect.x;
    }
    if(align & ALIGN_BOTTOM)
    {
        cacheSpriteOffset.y = (float32)(textMetrics.drawRect.dy - textMetrics.height + textMetrics.drawRect.y);
    }
    else if(align & ALIGN_VCENTER)
    {
        cacheSpriteOffset.y = ((float32)(textMetrics.drawRect.dy - textMetrics.height) * 0.5f + textMetrics.drawRect.y);
    }
	else
	{
        cacheSpriteOffset.y = (float32)textMetrics.drawRect.y;
    }

    // Restore font size
    font->SetSize(originalFontSize);
}

void TextBlock::PreDraw()
{
    CalculateCacheParamsIfNeed();

    if (needPrepareInternal)
    {
        PrepareInternal();
	}
    
	if (textBlockRender)
	{
        font->SetSize(renderSize);
        textBlockRender->PreDraw();
        font->SetSize(originalFontSize);
	}
}

void TextBlock::Draw(const Color& textColor, const Vector2* offset/* = NULL*/)
{
    if (textBlockRender)
    {
        font->SetSize(renderSize);
        textBlockRender->Draw(textColor, offset);
        font->SetSize(originalFontSize);
    }
}

TextBlock * TextBlock::Clone()
{
    return new TextBlock(*this);
}

};
