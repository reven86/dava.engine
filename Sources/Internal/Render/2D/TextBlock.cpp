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
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"
#include "Render/2D/TextBlock.h"
#include "Core/Core.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/TextBlockSoftwareRender.h"
#include "Render/2D/TextBlockGraphicRender.h"
#include "Concurrency/Thread.h"
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

static Set<TextBlock *> registredBlocks;

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
	for(Set<TextBlock *>::iterator it = registredBlocks.begin(), endIt = registredBlocks.end(); it != endIt; ++it)
	{
		(*it)->Prepare();
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
    , cacheTextSize(0.f,0.f)
    , renderSize(1.f)
    , cacheDx(0)
    , cacheDy(0)
    , cacheW(0)
    , cacheOx(0)
    , cacheOy(0)
    , textureForInvalidation(NULL)
	, angle(0.f)
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

TextBlock::~TextBlock()
{
	SafeRelease(textureForInvalidation);
    SafeRelease(textBlockRender);
    SafeDelete(textureInvalidater);
    SafeRelease(font);
    UnregisterTextBlock(this);
}

// Setters // Getters

void TextBlock::SetFont(Font * _font)
{
    mutex.Lock();

    if (!_font || _font == font)
    {
        mutex.Unlock();
        return;
    }

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
		case Font::TYPE_GRAPHIC:
        case Font::TYPE_DISTANCE:
			textBlockRender = new TextBlockGraphicRender(this);
			break;
		default:
			DVASSERT(!"Unknown font type");
			break;
	}
	
    mutex.Unlock();
    Prepare();
}

void TextBlock::SetRectSize(const Vector2 & size)
{
    mutex.Lock();

	if (rectSize != size)
	{
		rectSize = size;

		mutex.Unlock();
		Prepare();
        return;
    }
    mutex.Unlock();
}

void TextBlock::SetPosition(const Vector2& position)
{
    this->position = position;
}

void TextBlock::SetAngle(const float _angle)
{
	angle = _angle;
}

void TextBlock::SetPivot(const Vector2 & _pivot)
{
	pivot = _pivot;
}

void TextBlock::SetScale(const Vector2 & _scale)
{
    mutex.Lock();

    if (scale != _scale)
    {
        scale = _scale;

        mutex.Unlock();
        Prepare();
        return;
    }
    mutex.Unlock();
}

void TextBlock::SetText(const WideString & _string, const Vector2 &requestedTextRectSize)
{
    mutex.Lock();

    if (logicalText == _string && requestedSize == requestedTextRectSize)
    {
        mutex.Unlock();
		return;
	}
	requestedSize = requestedTextRectSize;
    logicalText = _string;

    mutex.Unlock();
    Prepare();
}

void TextBlock::SetMultiline(bool _isMultilineEnabled, bool bySymbol)
{
    mutex.Lock();
    if (isMultilineEnabled != _isMultilineEnabled || isMultilineBySymbolEnabled != bySymbol)
    {
        isMultilineBySymbolEnabled = bySymbol;
		isMultilineEnabled = _isMultilineEnabled;

        mutex.Unlock();
        Prepare();
        return;
    }
    mutex.Unlock();
}

void TextBlock::SetFittingOption(int32 _fittingType)
{
    mutex.Lock();
	if (fittingType != _fittingType)
	{
		fittingType = _fittingType;

        mutex.Unlock();
        Prepare();
        return;
    }
    mutex.Unlock();
}

Vector2 TextBlock::GetPreferredSizeForWidth(float32 width)
{
    if(!font)
        return Vector2();

    Vector2 result;
    
    mutex.Lock();
    
    if (requestedSize.dx < 0.0f && requestedSize.dy < 0.0f && fittingType == FITTING_DISABLED)
    {
        result = cacheTextSize;
        mutex.Unlock();
    }
    else
    {
        Vector2 oldRequestedSize = requestedSize;
        int32 oldFitting = fittingType;
        
        requestedSize = Vector2(width, -1.0f);
        fittingType = FITTING_DISABLED;
        
        mutex.Unlock();
        
        CalculateCacheParams();

        mutex.Lock();
        result = cacheTextSize;
        requestedSize = oldRequestedSize;
        fittingType = oldFitting;
        mutex.Unlock();
        
        CalculateCacheParams();
    }

    return result;
}

Font * TextBlock::GetFont()
{
    LockGuard<Mutex> guard(mutex);
    return font;
}

const Vector<WideString> & TextBlock::GetMultilineStrings()
{
    LockGuard<Mutex> guard(mutex);
    return multilineStrings;
}

const WideString & TextBlock::GetText()
{
    LockGuard<Mutex> guard(mutex);
    return logicalText;
}

const WideString & TextBlock::GetVisualText()
{
    LockGuard<Mutex> guard(mutex);
    return visualText;
}

bool TextBlock::GetMultiline()
{
    LockGuard<Mutex> guard(mutex);
    return isMultilineEnabled;
}

bool TextBlock::GetMultilineBySymbol()
{
    LockGuard<Mutex> guard(mutex);
    return isMultilineBySymbolEnabled;
}

int32 TextBlock::GetFittingOption()
{
    LockGuard<Mutex> guard(mutex);
    return fittingType;
}

float32 TextBlock::GetRenderSize()
{
    LockGuard<Mutex> guard(mutex);
    return renderSize;
}

void TextBlock::SetRenderSize(float32 _renderSize)
{
    mutex.Lock();
    if (renderSize != _renderSize)
    {
        renderSize = Max(_renderSize, 0.1f);

        mutex.Unlock();
        Prepare();
        return;
    }
    mutex.Unlock();
}

#if defined(LOCALIZATION_DEBUG)
int32 TextBlock::GetFittingOptionUsed()
{
    LockGuard<Mutex> guard(mutex);
    return fittingTypeUsed;
}

bool  TextBlock::IsVisualTextCroped()
{
    LockGuard<Mutex> guard(mutex);
	return visualTextCroped;
}
#endif

void TextBlock::SetAlign(int32 _align)
{
    mutex.Lock();
	if (align != _align) 
	{
		align = _align;

        mutex.Unlock();
        Prepare();
        return;
    }
    mutex.Unlock();
}

void TextBlock::SetUseRtlAlign(eUseRtlAlign useRtlAlign)
{
    mutex.Lock();
	if(this->useRtlAlign != useRtlAlign)
	{
		this->useRtlAlign = useRtlAlign;
		mutex.Unlock();
		Prepare();
		return;
	}
    mutex.Unlock();
}

TextBlock::eUseRtlAlign TextBlock::GetUseRtlAlign()
{
    LockGuard<Mutex> guard(mutex);
    return useRtlAlign;
}
    
bool TextBlock::IsRtl()
{
    LockGuard<Mutex> guard(mutex);
    return isRtl;
}

int32 TextBlock::GetAlign()
{
    LockGuard<Mutex> guard(mutex);
    return align;
}
	
int32 TextBlock::GetVisualAlign()
{
    LockGuard<Mutex> guard(mutex);
    return GetVisualAlignNoMutexLock();
}
	
int32 TextBlock::GetVisualAlignNoMutexLock() const
{
	if (((align & (ALIGN_LEFT | ALIGN_RIGHT)) != 0) &&
        ((useRtlAlign == RTL_USE_BY_CONTENT && isRtl) ||
         (useRtlAlign == RTL_USE_BY_SYSTEM && UIControlSystem::Instance()->IsRtl())))
    {
        // Mirror left/right align
        return align ^ (ALIGN_LEFT | ALIGN_RIGHT);
    }
	return align;
}

Sprite * TextBlock::GetSprite()
{
    LockGuard<Mutex> guard(mutex);

    Sprite* sprite = NULL;
    if (textBlockRender)
    {
        sprite = textBlockRender->GetSprite();

    }

    return sprite;
}

bool TextBlock::IsSpriteReady()
{
	return (GetSprite() != NULL);
}

void TextBlock::Prepare(Texture *texture /*=NULL*/)
{
	if(!font)
	{
		return;
	}
	
	CalculateCacheParams();

	{
		LockGuard<Mutex> guard(mutex);
		SafeRelease(textureForInvalidation);
		textureForInvalidation = SafeRetain(texture);
		needPrepareInternal = true;
	}
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
    LockGuard<Mutex> guard(mutex);
#if defined(LOCALIZATION_DEBUG)
    fittingTypeUsed = FITTING_DISABLED;
    visualTextCroped = false;
#endif

    if (logicalText.empty())
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

    visualText = logicalText;
    
    TextLayout textLayout(isBiDiSupportEnabled);
    Vector<float32> charSizes;
    
    textLayout.Reset(logicalText, *font);
    isRtl = textLayout.IsRtlText();
    
    visualText = textLayout.GetVisualText(false);

    bool useJustify = ((align & ALIGN_HJUSTIFY) != 0);
    renderSize = originalFontSize * scale.y;
    font->SetSize(renderSize);
    Vector2 drawSize = rectSize;

    if(requestedSize.dx > 0)
    {
        drawSize.x = requestedSize.dx;
    }
    if(requestedSize.dy > 0)
    {
        drawSize.y = requestedSize.dy;
    }

    Font::StringMetrics textSize;
    stringSizes.clear();

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
        Vector<float32> charSizes;
        textSize = font->GetStringMetrics(visualText, &charSizes);
        DVASSERT(charSizes.size() == visualText.length());
        
        for (float32& val : charSizes)
        {
            val = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(val);
        }
        
        WideString pointsStr;
        if((fittingType & FITTING_POINTS) && (drawSize.x < textSize.width))
        {
            uint32 length = charSizes.size();
            Font::StringMetrics pointsMetric = font->GetStringMetrics(L"...");
            float32 fullWidth = static_cast<float32>(textSize.width + pointsMetric.width);
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
        else if(!((fittingType & FITTING_REDUCE) || (fittingType & FITTING_ENLARGE)) && (drawSize.x < textSize.width) && (requestedSize.x >= 0))
        {
            uint32 length = charSizes.size();
            float32 fullWidth = static_cast<float32>(textSize.width);
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
                    if((isChanged || fittingType & FITTING_REDUCE) && textSize.height > drawSize.y)
                    {
                        if (prevFontSize < renderSize)
                        {
                            renderSize = prevFontSize;
                            font->SetSize(renderSize);
                            textSize = font->GetStringMetrics(visualText);
                            break;
                        }
                        yBigger = true;
                        yMul = drawSize.y / textSize.height;
                    }
                    else if((isChanged || fittingType & FITTING_ENLARGE) && textSize.height < drawSize.y * 0.9)
                    {
                        yLower = true;
                        yMul = (drawSize.y * 0.9f) / textSize.height;
                        if(yMul < 1.01f)
                        {
                            yLower = false;
                        }
                    }
                }

                if(requestedSize.dx >= 0)
                {
                    if((isChanged || fittingType & FITTING_REDUCE) && textSize.width > drawSize.x)
                    {
                        if (prevFontSize < renderSize)
                        {
                            renderSize = prevFontSize;
                            font->SetSize(renderSize);
                            textSize = font->GetStringMetrics(visualText);
                            break;
                        }
                        xBigger = true;
                        xMul = drawSize.x / textSize.width;
                    }
                    else if((isChanged || fittingType & FITTING_ENLARGE) && textSize.width < drawSize.x * 0.95)
                    {
                        xLower = true;
                        xMul = (drawSize.x * 0.95f) / textSize.width;
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
                textSize = font->GetStringMetrics(visualText);
            }
        }

        if(!pointsStr.empty())
        {
            visualText = pointsStr;
            textSize = font->GetStringMetrics(visualText);
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
            stringSizes.push_back(textSize.width);
        }
    }
    else //if(!isMultilineEnabled)
    {
        if (fittingType && (requestedSize.dy >= 0/* || requestedSize.dx >= 0*/) && visualText.size() > 3)
        {
            multilineStrings.clear();
            textLayout.Seek(0);
            while (!textLayout.IsEndOfText())
            {
                if(isMultilineBySymbolEnabled || !textLayout.NextByWords(drawSize.dx))
                {
                    textLayout.NextBySymbols(drawSize.dx);
                }
                multilineStrings.push_back(textLayout.GetVisualLine(true));
            }
        
            int32 yOffset = font->GetVerticalSpacing();
            int32 fontHeight = font->GetFontHeight() + yOffset;
            float32 lastSize = renderSize;

            textSize.width = 0;
            textSize.height = fontHeight * (int32)multilineStrings.size() - yOffset;

            bool isChanged = false;
            while (true)
            {
                float32 yMul = 1.0f;

                bool yBigger = false;
                bool yLower = false;
                if(requestedSize.dy >= 0)
                {
                    if((isChanged || fittingType & FITTING_REDUCE) && textSize.height > drawSize.y)
                    {
                        yBigger = true;
                        yMul = drawSize.y / textSize.height;
                        if (lastSize < renderSize)
                        {
                            renderSize = lastSize;
                            font->SetSize(renderSize);
                            break;
                        }
                    }
                    else if((isChanged || fittingType & FITTING_ENLARGE) && textSize.height < drawSize.y * 0.95)
                    {
                        yLower = true;
                        if(textSize.height < drawSize.y * 0.75f)
                        {
                            yMul = (drawSize.y * 0.75f) / textSize.height;
                        }
                        else if(textSize.height < drawSize.y * 0.8f)
                        {
                            yMul = (drawSize.y * 0.8f) / textSize.height;
                        }
                        else if(textSize.height < drawSize.y * 0.85f)
                        {
                            yMul = (drawSize.y * 0.85f) / textSize.height;
                        }
                        else if(textSize.height < drawSize.y * 0.9f)
                        {
                            yMul = (drawSize.y * 0.9f) / textSize.height;
                        }
                        else
                        {
                            yMul = (drawSize.y * 0.95f) / textSize.height;
                        }
                        if (yMul == 1.0f)
                        {
                            yMul = 1.05f;
                        }
                    }
                }

                if((!yBigger && !yLower) || FLOAT_EQUAL(renderSize,0.f))
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
                textLayout.Reset(logicalText, *font);
                while (!textLayout.IsEndOfText())
                {
                    if(isMultilineBySymbolEnabled || !textLayout.NextByWords(drawSize.dx))
                    {
                        textLayout.NextBySymbols(drawSize.dx);
                    }
                    multilineStrings.push_back(textLayout.GetVisualLine(true));
                }

                yOffset = font->GetVerticalSpacing();
                fontHeight = font->GetFontHeight() + yOffset;
                textSize.height = fontHeight * (int32)multilineStrings.size() - yOffset;

            };
        }

        multilineStrings.clear();
        textLayout.Reset(logicalText, *font);
        while (!textLayout.IsEndOfText())
        {
            if(isMultilineBySymbolEnabled || !textLayout.NextByWords(drawSize.dx))
            {
                textLayout.NextBySymbols(drawSize.dx);
            }
            multilineStrings.push_back(textLayout.GetVisualLine(true));
        }

        int32 yOffset = font->GetVerticalSpacing();
        int32 fontHeight = font->GetFontHeight() + yOffset;

        textSize.width = textSize.drawRect.dx = 0;
        textSize.height = textSize.drawRect.dy = fontHeight * (int32)multilineStrings.size() - yOffset;	

        if (textSize.height > drawSize.y && requestedSize.y >= 0.f)
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
            textSize.height = textSize.drawRect.dy = fontHeight * (int32)multilineStrings.size() - yOffset;	
        }

        stringSizes.reserve(multilineStrings.size());
        for (int32 line = 0; line < (int32)multilineStrings.size(); ++line)
        {
            Font::StringMetrics stringSize = font->GetStringMetrics(multilineStrings[line]);
            stringSizes.push_back(stringSize.width);

            textSize.drawRect.dx = Max(textSize.drawRect.dx, stringSize.drawRect.dx + stringSize.drawRect.x);
            textSize.drawRect.x = Min(textSize.drawRect.x, stringSize.drawRect.x);

            if (requestedSize.dx >= 0)
            {
                textSize.width = Max(textSize.width, Min(stringSize.width, (int32)drawSize.x));
            }
            else
            {
                textSize.width = Max(textSize.width, stringSize.width);
            }

            if(0 == line)
            {
                textSize.drawRect.y = stringSize.drawRect.y;
            }

#if defined(LOCALIZATION_DEBUG)
            if(textSize.width < stringSize.width)
            {
                visualTextCroped = true;
            }
#endif
        }
        // Translate right/bottom edge to width/height
        textSize.drawRect.dx -= textSize.drawRect.x;
        textSize.drawRect.dy -= textSize.drawRect.y;
    }

    if (requestedSize.dx >= 0 && useJustify)
    {
        textSize.drawRect.dx = Max(textSize.drawRect.dx, (int)drawSize.dx);
    }

    //calculate texture size
    int32 dx = (int32)ceilf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX((float32)textSize.drawRect.dx));
    int32 dy = (int32)ceilf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY((float32)textSize.drawRect.dy));
    int32 ox = (int32)ceilf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX((float32)textSize.drawRect.x));
    int32 oy = (int32)ceilf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY((float32)textSize.drawRect.y));

    cacheUseJustify = useJustify;
    cacheDx = dx;
    EnsurePowerOf2(cacheDx);

    cacheDy = dy;
    EnsurePowerOf2(cacheDy);

    cacheOx = ox;
    cacheOy = oy;

    cacheW = textSize.drawRect.dx;
    cacheFinalSize.x = (float32)textSize.drawRect.dx;
    cacheFinalSize.y = (float32)textSize.drawRect.dy;
    cacheTextSize = Vector2((float32)textSize.width, (float32)textSize.height);

    // Align sprite offset
    if(align & ALIGN_RIGHT)
    {
        cacheSpriteOffset.x = (float32)(textSize.drawRect.dx - textSize.width + textSize.drawRect.x);
    }
    else if(align & ALIGN_HCENTER)
    {
        cacheSpriteOffset.x = ((float32)(textSize.drawRect.dx - textSize.width) * 0.5f + textSize.drawRect.x);
    }
    else
    {
        cacheSpriteOffset.x = (float32)textSize.drawRect.x;
    }
    if(align & ALIGN_BOTTOM)
    {
        cacheSpriteOffset.y = (float32)(textSize.drawRect.dy - textSize.height + textSize.drawRect.y);
    }
    else if(align & ALIGN_VCENTER)
    {
        cacheSpriteOffset.y = ((float32)(textSize.drawRect.dy - textSize.height) * 0.5f + textSize.drawRect.y);
    }
	else
	{
		cacheSpriteOffset.y = (float32)textSize.drawRect.y;
	}

    // Restore font size
    font->SetSize(originalFontSize);
}

void TextBlock::PreDraw()
{
	if(needPrepareInternal)
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
    TextBlock *block = new TextBlock();

    block->SetScale(scale);
    block->SetRectSize(rectSize);
    block->SetMultiline(GetMultiline(), GetMultilineBySymbol());
    block->SetAlign(align);
    block->SetFittingOption(fittingType);
    block->SetUseRtlAlign(useRtlAlign);

    if (GetFont())
    {
        block->SetFont(GetFont());
    }
    block->SetText(GetText(), requestedSize);

    return block;
}

void TextBlock::ForcePrepare(Texture *texture)
{
    Prepare(texture);
}

void TextBlock::SetBiDiSupportEnabled(bool value)
{
    isBiDiSupportEnabled = value;
}

bool TextBlock::IsBiDiSupportEnabled()
{
    return isBiDiSupportEnabled;
}

const Vector2 & TextBlock::GetTextSize()
{
    LockGuard<Mutex> guard(mutex);
    return cacheTextSize;
}

const Vector<int32> & TextBlock::GetStringSizes() const
{
    return stringSizes;
}

const Vector2& TextBlock::GetSpriteOffset()
{
    LockGuard<Mutex> guard(mutex);
    return cacheSpriteOffset;
}

};
