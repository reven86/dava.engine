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


#include "Render/2D/TextBlockRender.h"
#include "Core/Core.h"

namespace DAVA 
{
    
TextBlockRender::TextBlockRender(TextBlock* textBlock)
{
	this->textBlock = textBlock;
	sprite = NULL;
}
	
TextBlockRender::~TextBlockRender()
{
	SafeRelease(sprite);
}
	
void TextBlockRender::Prepare(Texture *texture /*= NULL*/)
{
	SafeRelease(sprite);
}
	
void TextBlockRender::DrawText()
{
	if (!textBlock->isMultilineEnabled || textBlock->treatMultilineAsSingleLine)
	{
        DrawTextSL(textBlock->visualText, textBlock->cacheDx, textBlock->cacheDy, textBlock->cacheW);
	}
	else
	{
		uint32 yOffset = 0;
        int32 stringSize = 0;
        int32 blockWidth = 0;
		int32 fontHeight = textBlock->font->GetFontHeight() + textBlock->font->GetVerticalSpacing();
        int32 stringsCnt = (int32)textBlock->multilineStrings.size();
		for (int32 line = 0; line < stringsCnt; ++line)
		{
			if (line == (int32)textBlock->multilineStrings.size() - 1)
			{
				textBlock->cacheUseJustify = false;
			}
			int32 xOffset = 0;
            int32 align = textBlock->GetVisualAlign();
            if (align & ALIGN_RIGHT)
			{
                xOffset = (int32)(textBlock->cacheTextSize.dx - textBlock->stringSizes[line] + textBlock->cacheSpriteOffset.x);
				if(xOffset < 0)
				{
					xOffset = 0;
				}
			}
			else if(align & ALIGN_HCENTER)
			{
                xOffset = (int32)(textBlock->cacheTextSize.dx - textBlock->stringSizes[line] + textBlock->cacheSpriteOffset.x) / 2;
				if(xOffset < 0)
				{
					xOffset = 0;
				}
			}
            if(align & ALIGN_HJUSTIFY && textBlock->cacheUseJustify)
            {
                stringSize = textBlock->stringSizes[line];
                blockWidth =textBlock->cacheW;
            }
            else
            {
                stringSize = 0;
                blockWidth = 0;
            }
            DrawTextML(textBlock->multilineStrings[line],
                       textBlock->cacheDx,
                       textBlock->cacheDy,
                       blockWidth,
                       xOffset,
                       yOffset,
                       stringSize);
            
			yOffset += fontHeight;
		}	
	}
}
	
};