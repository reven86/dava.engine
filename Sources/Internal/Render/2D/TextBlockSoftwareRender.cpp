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
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/RenderCallbacks.h"
#include "Core/Core.h"
#include "Utils/Utils.h"

namespace DAVA
{
TextBlockSoftwareRender::TextBlockSoftwareRender(TextBlock* textBlock) :
	TextBlockRender(textBlock)
{
    RenderCallbacks::RegisterResourceRestoreCallback(MakeFunction(this, &TextBlockSoftwareRender::Restore));
    buf = NULL;
	ftFont = (FTFont*)textBlock->font;
#if defined(LOCALIZATION_DEBUG)
    textOffsetTL.x = std::numeric_limits<float32>::max();
    textOffsetTL.y = std::numeric_limits<float32>::max();
    textOffsetBR.x = 0;
    textOffsetBR.y = 0;
#endif
}
TextBlockSoftwareRender::~TextBlockSoftwareRender()
{
    RenderCallbacks::UnRegisterResourceRestoreCallback(MakeFunction(this, &TextBlockSoftwareRender::Restore));
}
void TextBlockSoftwareRender::Prepare()
{
    TextBlockRender::Prepare();

    int32 width = Max(textBlock->cacheDx, 1);
    int32 height = Max(textBlock->cacheDy, 1);
#if defined(LOCALIZATION_DEBUG)
    bufHeight = height;
    bufWidth = width ;
    textOffsetTL.x = static_cast<float32>( width - 1);
    textOffsetTL.y = static_cast<float32>(height - 1);
    textOffsetBR.x = 0;
    textOffsetBR.y = 0;
#endif
	int32 bsz = width * height;
    buf = new int8[bsz];
    memset(buf, 0, bsz * sizeof(int8));
	
	DrawText();
	
	String addInfo;
	if (!textBlock->isMultilineEnabled)
	{
		addInfo = WStringToString(textBlock->visualText.c_str());
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

    Texture* tex = Texture::CreateTextFromData(FORMAT_A8, (uint8*)buf, width, height, false, addInfo.c_str());
    sprite = Sprite::CreateFromTexture(tex, 0, 0, textBlock->cacheFinalSize.dx, textBlock->cacheFinalSize.dy);
    SafeRelease(tex);

    SafeDeleteArray(buf);
}

void TextBlockSoftwareRender::Restore()
{
    if (!sprite)
        return;
    Texture* tex = sprite->GetTexture();
    if (!tex)
        return;
    if (!rhi::NeedRestoreTexture(tex->handle))
        return;

    int32 width = Max(textBlock->cacheDx, 1);
    int32 height = Max(textBlock->cacheDy, 1);
    int32 bsz = width * height;
    buf = new int8[bsz];
    memset(buf, 0, bsz * sizeof(int8));

    DrawText();

    tex->ReloadFromData(FORMAT_A8, (uint8*)buf, width, height);

    SafeDeleteArray(buf);
}

Font::StringMetrics TextBlockSoftwareRender::DrawTextSL(const WideString& drawText, int32 x, int32 y, int32 w)
{
	 Font::StringMetrics metrics= ftFont->DrawStringToBuffer(buf, x, y, 
										-textBlock->cacheOx, 
										-textBlock->cacheOy, 
										0, 
										0, 
										drawText, 
										true);
#if defined(LOCALIZATION_DEBUG)
    CalculateTextBBox();
#endif
    return metrics;
}
	
Font::StringMetrics TextBlockSoftwareRender::DrawTextML(const WideString& drawText, int32 x, int32 y, int32 w, int32 xOffset, uint32 yOffset, int32 lineSize)
{
    Font::StringMetrics metrics;
	if (textBlock->cacheUseJustify)
	{
		metrics= ftFont->DrawStringToBuffer(buf, x, y,
            -textBlock->cacheOx + (int32)(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX((float32)xOffset)),
            -textBlock->cacheOy + (int32)(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY((float32)yOffset)),
            (int32)ceilf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX((float32)w)),
            (int32)ceilf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY((float32)lineSize)),
										  drawText,
										  true);
	}
	else
	{
		metrics =  ftFont->DrawStringToBuffer(buf, x, y,
        	-textBlock->cacheOx + (int32)(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX((float32)xOffset)),
        	-textBlock->cacheOy + (int32)(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY((float32)yOffset)),
									 0,
									 0,
									 drawText,
									 true);
	}
#if defined(LOCALIZATION_DEBUG)
    CalculateTextBBox();
#endif
    return metrics;
}
#if defined(LOCALIZATION_DEBUG)
Vector2 TextBlockSoftwareRender::getTextOffsetTL()
{
    return textOffsetTL;
}
Vector2 TextBlockSoftwareRender::getTextOffsetBR()
{
    return textOffsetBR;
}
void TextBlockSoftwareRender::CalculateTextBBox()
{
    const int8 * bufWalker = buf;
    float32 height = static_cast<float>(bufHeight);
    float32 width = static_cast<float>(bufWidth);
    for (float32 h = 0; h < height; h++)
    {
        for (float32 w = 0; w < width; w++)
        {
            if (*bufWalker != 0  )
            {
                textOffsetTL.x = Min(w, textOffsetTL.x);
                textOffsetTL.y = Min(h, textOffsetTL.y);
                textOffsetBR.x = Max(w, textOffsetBR.x);
                textOffsetBR.y = Max(h, textOffsetBR.y);
                
            }
            bufWalker++;
        }
    }
}
#endif
};