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
    : TextLayout(false)
{
}

TextLayout::TextLayout(const bool useBiDi)
    : useBiDi(useBiDi)
    , isRtl(false)
    , fromPos(0)
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

        StringUtils::GetLineBreaks(preparedText, breaks);
        DVASSERT(breaks.size() == preparedText.length());
    }
    
    // Update characters sizes from font always
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

bool TextLayout::HasNext()
{
    return fromPos < preparedText.length();
}

void TextLayout::Next(const float32 lineWidth, const bool bySymbols /*= false*/)
{
    if (bySymbols)
    {
        NextBySymbols(lineWidth);
    }
    else
    {
        NextByWords(lineWidth);
    }
}

void TextLayout::NextByWords(const float32 lineWidth)
{
    float32 targetWidth = std::floor(lineWidth);
    float32 currentWidth = 0;
    uint32 textLength = (uint32)preparedText.length();
    uint32 lastPossibleBreak = 0;

    for (uint32 pos = fromPos; pos < textLength; ++pos)
    {
        char16 ch = preparedText[pos];
        uint8 canBreak = breaks[pos];

        currentWidth += VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(characterSizes[pos]);

        // Check that targetWidth defined and currentWidth less than targetWidth.
        // If symbol is whitespace skip it and go to next (add all whitespace to current line)
        if (targetWidth == 0.f || currentWidth <= targetWidth || StringUtils::IsWhitespace(ch))
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
        else // If not then split by previous symbol
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
    int32 totalSize = preparedText.length();
    int32 currentLineEnd = 0;

    for (int pos = fromPos; pos < totalSize; pos++)
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

        if (t == L'\\' && tNext == L'n')
        {
            preparedLine = preparedText.substr(fromPos, currentLineEnd - fromPos);

            fromPos = pos + 2;
            return;
        }

        // Use additional condition to prevent endless loop, when target size is less than
        // size of one symbol (sizes[pos] > targetWidth)
        // To keep initial index logic we should always perform action currentLineDx += sizes[pos]
        // before entering this condition, so currentLineDx > 0.
        if ((currentLineDx > 0) && ((currentLineDx + VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(characterSizes[pos])) > targetWidth))
        {
            preparedLine = preparedText.substr(fromPos, currentLineEnd - fromPos);

            fromPos = pos;
            return;
        }
        else
        {
            currentLineDx += VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(characterSizes[pos]);
        }
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