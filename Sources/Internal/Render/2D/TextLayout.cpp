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

#include "Render/2D/TextLayout.h"
#include "Utils/BiDiHelper.h"
#include "Systems/VirtualCoordinatesSystem.h"
#include "Utils/StringUtils.h"

namespace DAVA 
{

TextLayout::TextLayout()
    : TextLayout(WRAP_BY_WORDS, false)
{
}

TextLayout::TextLayout(const WrapMode _wrapMode, const bool _useBiDi)
    : useBiDi(_useBiDi)
    , isRtl(false)
    , fromPos(0)
    , wrapMode(_wrapMode)
{
}

TextLayout::~TextLayout()
{
}

void TextLayout::Reset(const WideString& _input, const Font& _font)
{
    if (inputText != _input)
    {
        // Update prepared text and line breaks only if input text was changed 
        inputText = _input;
        preparedText = _input;
        if (useBiDi)
        {
            bidiHelper.PrepareString(inputText, preparedText, &isRtl);
        }

        if(wrapMode == WRAP_BY_WORDS)
        {
            StringUtils::GetLineBreaks(preparedText, breaks);
            DVASSERT_MSG(breaks.size() == preparedText.length(), "Incorrect breaks information");    
        }
    }
    
    // Update characters sizes from font
    characterSizes.clear();
    _font.GetStringMetrics(preparedText, &characterSizes);
    DVASSERT(characterSizes.size() == preparedText.length());

    if (useBiDi)
    {
        for (uint32 i = 0; i < characterSizes.size(); ++i)
        {
            if (bidiHelper.IsBiDiSpecialCharacter(preparedText[i]))
            {
                characterSizes[i] = 0.0f;
            }
        }
    }
    
    Seek(0);
}

void TextLayout::Seek(const uint32 _position)
{
    fromPos = _position;
    preparedLine.clear();
}

void TextLayout::SetWrapMode(const WrapMode mode)
{
    if (wrapMode != mode)
    {
        wrapMode = mode;
        if (wrapMode == WRAP_BY_WORDS)
        {
            StringUtils::GetLineBreaks(preparedText, breaks);
            DVASSERT_MSG(breaks.size() == preparedText.length(), "Incorrect breaks information");
        }
    }
}

bool TextLayout::HasNext()
{
    return fromPos < preparedText.length();
}

void TextLayout::Next(const float32 lineWidth)
{
    switch (wrapMode)
    {
    case WRAP_BY_WORDS:
        NextByWords(lineWidth);
        break;
    case WRAP_BY_SYMBOLS:
        NextBySymbols(lineWidth);
        break;
    default:
        DVASSERT_MSG(false, "Use correct WrapMode");
    }
}

void TextLayout::NextByWords(const float32 lineWidth)
{
    float32 targetWidth = std::floor(lineWidth);
    float32 currentWidth = 0;
    uint32 textLength = (uint32)preparedText.length();
    size_t lastPossibleBreak = 0;

    DVASSERT_MSG(textLength == breaks.size(), "Reset with wrong wrap mode");

    for (size_t pos = fromPos; pos < textLength; ++pos)
    {
        char16 ch = preparedText[pos];
        uint8 canBreak = breaks[pos];

        currentWidth += VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(characterSizes[pos]);

        // Check that targetWidth defined and currentWidth less than targetWidth.
        // If symbol is whitespace skip it and go to next (add all whitespace to current line)
        if (currentWidth <= targetWidth || StringUtils::IsWhitespace(ch))
        {
            if (canBreak == StringUtils::LB_MUSTBREAK) // If symbol is line breaker then split string
            {
                preparedLine = preparedText.substr(fromPos, pos - fromPos + 1);
                
                currentWidth = 0.f;
                lastPossibleBreak = 0;
                fromPos = pos + 1;
                return;
            }
            else if (canBreak == StringUtils::LB_ALLOWBREAK) // Store breakable symbol position
            {
                lastPossibleBreak = pos;
            }
            continue;
        }

        if (lastPossibleBreak > 0) // If we have any breakable symbol in current substring then split by it
        {
            pos = lastPossibleBreak;
        }
        else if (pos > fromPos) // If not then split by previous symbol which position bigger or equal that fromPos
        {
            pos--;
        }

        preparedLine = preparedText.substr(fromPos, pos - fromPos + 1);
        
        currentWidth = 0.f;
        lastPossibleBreak = 0;
        fromPos = pos + 1;
        return;
    }

    DVASSERT_MSG(fromPos == textLength, "Incorrect line split");
}

void TextLayout::NextBySymbols(const float32 lineWidth)
{
    float32 targetWidth = std::floor(lineWidth);
    float32 currentLineDx = 0;
    size_t totalSize = preparedText.length();
    size_t currentLineEnd = 0;

    for (size_t pos = fromPos; pos < totalSize; pos++)
    {
        char16 t = preparedText[pos];
        char16 tNext = 0;
        if (pos + 1 < totalSize)
        {
            tNext = preparedText[pos + 1];
        }

        currentLineEnd = pos;

        if (t == L'\n')
        {
            preparedLine = preparedText.substr(fromPos, currentLineEnd - fromPos);
            fromPos = pos + 1;
            return;
        }

        float32 characterSize = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(characterSizes[pos]);

        // Use additional condition to prevent endless loop, when target size is less than
        // size of one symbol (sizes[pos] > targetWidth)
        // To keep initial index logic we should always perform action currentLineDx += sizes[pos]
        // before entering this condition, so currentLineDx > 0.
        if ((currentLineDx > 0) && ((currentLineDx + characterSize) > targetWidth))
        {
            preparedLine = preparedText.substr(fromPos, currentLineEnd - fromPos);
            fromPos = pos;
            return;
        }
        
        currentLineDx += characterSize;
    }

    preparedLine = preparedText.substr(fromPos, currentLineEnd - fromPos + 1);
    fromPos = totalSize;
}

const WideString TextLayout::GetVisualText(const bool trimEnd) const
{
    return BuildVisualString(preparedText, trimEnd);
}

const WideString TextLayout::GetVisualLine(const bool trimEnd) const
{
    return BuildVisualString(preparedLine, trimEnd);
}

const WideString TextLayout::BuildVisualString(const WideString& _input, const bool trimEnd) const
{
    WideString output = _input;
    if (useBiDi)
    {
        bidiHelper.ReorderString(output, isRtl);
    }
    output = StringUtils::RemoveNonPrintable(output, 1);
    if (trimEnd)
    {
        output = !isRtl ? StringUtils::TrimRight(output) : StringUtils::TrimLeft(output);
    }
    return output;
}

}