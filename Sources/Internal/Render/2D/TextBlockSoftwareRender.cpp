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


#include "Render/2D/TextBlockSoftwareRender.h"
#include "Core/Core.h"
#include "Utils/Utils.h"

namespace DAVA 
{
    
TextBlockSoftwareRender::TextBlockSoftwareRender(TextBlock* textBlock) :
	TextBlockRender(textBlock)
{
	buf = NULL;
	ftFont = (FTFont*)textBlock->font;
}
	
void TextBlockSoftwareRender::Prepare()
{
	TextBlockRender::Prepare();
	
	int bsz = textBlock->cacheDx * textBlock->cacheDy;
	buf = new int16[bsz];
    memset(buf, 0, bsz * sizeof(int16));
	
	DrawText();
	
	String addInfo;
	if (!textBlock->isMultilineEnabled)
	{
		addInfo = WStringToString(textBlock->text.c_str());
	}
	else
	{
		if (textBlock->multilineStrings.size() >= 1)
		{
			addInfo = WStringToString(textBlock->multilineStrings[0].c_str());
		}else
		{
			addInfo = "empty";
		}
	}
	
	Texture *tex = Texture::CreateTextFromData(FORMAT_A8, (uint8*)buf, textBlock->cacheDx, textBlock->cacheDy, false, addInfo.c_str());
    sprite = Sprite::CreateFromTexture(tex, 0, 0, textBlock->cacheFinalSize.dx, textBlock->cacheFinalSize.dy);
    
	SafeDeleteArray(buf);
	SafeRelease(tex);
}
	
Size2i TextBlockSoftwareRender::DrawTextSL(const WideString& drawText, int32 x, int32 y, int32 w)
{
	return ftFont->DrawStringToBuffer(buf, x, y, 0, 0, 0, 0, drawText, true);
}
	
Size2i TextBlockSoftwareRender::DrawTextML(const WideString& drawText, int32 x, int32 y, int32 w, int32 xOffset, uint32 yOffset, int32 lineSize)
{
	if (textBlock->cacheUseJustify)
	{
		return ftFont->DrawStringToBuffer(buf, x, y,
										  (int32)(Core::GetVirtualToPhysicalFactor() * xOffset),
										  (int32)(Core::GetVirtualToPhysicalFactor() * yOffset),
										  (int32)ceilf(Core::GetVirtualToPhysicalFactor() * w),
										  (int32)ceilf(Core::GetVirtualToPhysicalFactor() * lineSize),
										  drawText,
										  true);
	}

	return ftFont->DrawStringToBuffer(buf, x, y,
								     (int32)(Core::GetVirtualToPhysicalFactor() * xOffset),
								     (int32)(Core::GetVirtualToPhysicalFactor() * yOffset),
									 0,
									 0,
									 drawText,
									 true);
}

};