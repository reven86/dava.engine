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


#include "Render/2D/TextBlockGraphicsRender.h"
#include "Core/Core.h"
#include "Utils/Utils.h"
#include "Render/RenderManager.h"
#include "Render/2D/RenderSystem2D/VirtualCoordinatesTransformSystem.h"

namespace DAVA 
{
	
TextBlockGraphicsRender::TextBlockGraphicsRender(TextBlock* textBlock) :
	TextBlockRender(textBlock)
{
	grFont = (GraphicsFont*)textBlock->font;
	isPredrawed = true;
}
	
void TextBlockGraphicsRender::Prepare()
{
	TextBlockRender::Prepare();
	
	isPredrawed = false;
	sprite = Sprite::CreateAsRenderTarget((float32)textBlock->cacheDx, (float32)textBlock->cacheDy, FORMAT_RGBA8888);
	if (sprite && sprite->GetTexture())
	{
		if (!textBlock->isMultilineEnabled)
			sprite->GetTexture()->SetDebugInfo(WStringToString(textBlock->text));
		else if (textBlock->isMultilineEnabled)
		{
			if (textBlock->multilineStrings.size() > 0)
				sprite->GetTexture()->SetDebugInfo(WStringToString(textBlock->multilineStrings[0]));
		}
	}
}
	
void TextBlockGraphicsRender::PreDraw()
{
	if (isPredrawed)
		return;
	
	isPredrawed = true;
	RenderManager::Instance()->SetRenderTarget(sprite);
	DrawText();
	RenderManager::Instance()->RestoreRenderTarget();
}
	
Size2i TextBlockGraphicsRender::DrawTextSL(const WideString& drawText, int32 x, int32 y, int32 w)
{
	if (textBlock->cacheUseJustify)
	{
		return grFont->DrawString(0, 0, drawText, (int32)ceilf(VirtualCoordinates::GetVirtualToPhysicalFactor() * w));
	}

	return grFont->DrawString(0, 0, drawText);
}

Size2i TextBlockGraphicsRender::DrawTextML(const WideString& drawText,
										   int32 x, int32 y, int32 w,
										   int32 xOffset, uint32 yOffset,
										   int32 lineSize)
{
	if (textBlock->cacheUseJustify)
	{
		return grFont->DrawString((float32)xOffset,
										   (float32)yOffset,
										   drawText,
										   (int32)ceilf(VirtualCoordinates::GetVirtualToPhysicalFactor() * w), lineSize);
	}
	
	return grFont->DrawString((float32)xOffset, (float32)yOffset, drawText, 0, 0);
}
	
};