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
    
TextBlockSoftwareTexInvalidater::TextBlockSoftwareTexInvalidater(TextBlock *textBlock) :
  textBlock(textBlock)
{
}
    
TextBlockSoftwareTexInvalidater::~TextBlockSoftwareTexInvalidater()
{
    // Create a copy of textureSet here, as textureSet will be cleaned by texture itself inside the SetInvalidater call.
    Set<Texture*> setCopy = textureSet;
    Set<Texture*>::iterator it = setCopy.begin();
    for(; it != setCopy.end(); ++it)
    {
        (*it)->SetInvalidater(NULL);
    }
    DVASSERT(textureSet.size() == 0);
}
    
void TextBlockSoftwareTexInvalidater::InvalidateTexture(DAVA::Texture *texture)
{
    textBlock->ForcePrepare(texture);
}

void TextBlockSoftwareTexInvalidater::RemoveTexture(Texture *tex)
{
    Set<Texture*>::iterator it = textureSet.find(tex);
    if(it != textureSet.end())
    {
        textureSet.erase(it);
    }
    else
    {
        Logger::Error("[TextBlockSoftwareTexInvalidater::RemoveTexToSet] trying to remove texture not in set");
    }
}
    
void TextBlockSoftwareTexInvalidater::AddTexture(Texture *tex)
{
    Set<Texture*>::iterator it = textureSet.find(tex);
    if(it == textureSet.end())
    {
        textureSet.insert(tex);
    }
    else
    {
        Logger::Error("[TextBlockSoftwareTexInvalidater::AddTexToSet] trying to add texture already in set");
    }
}
    
TextBlockSoftwareRender::TextBlockSoftwareRender(TextBlock* textBlock) :
	TextBlockRender(textBlock)
{
	buf = NULL;
	ftFont = (FTFont*)textBlock->font;
}
	
void TextBlockSoftwareRender::Prepare(Texture *texture /*=NULL*/)
{
    // Prevent releasing sprite when texture is invalidated
    if(!texture)
    {
        TextBlockRender::Prepare(NULL);
    }

    int32 width = Max(textBlock->cacheDx, 1);
    int32 height = Max(textBlock->cacheDy, 1);
    
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
	
    if(!texture)
    {
        Texture *tex = Texture::CreateTextFromData(FORMAT_A8, (uint8*)buf, width, height, false, addInfo.c_str());
        if(textBlock->textureInvalidater)
        {
            tex->SetInvalidater(textBlock->textureInvalidater);
        }
        sprite = Sprite::CreateFromTexture(tex, 0, 0, textBlock->cacheFinalSize.dx, textBlock->cacheFinalSize.dy);
        SafeRelease(tex);
    }
    else
    {
        texture->ReloadFromData(FORMAT_A8, (uint8*)buf, width, height);
    }
    
	SafeDeleteArray(buf);
}
	
Font::StringMetrics TextBlockSoftwareRender::DrawTextSL(const WideString& drawText, int32 x, int32 y, int32 w)
{
	return ftFont->DrawStringToBuffer(buf, x, y, 
										-textBlock->cacheOx, 
										-textBlock->cacheOy, 
										0, 
										0, 
										drawText, 
										true);
}
	
Font::StringMetrics TextBlockSoftwareRender::DrawTextML(const WideString& drawText, int32 x, int32 y, int32 w, int32 xOffset, uint32 yOffset, int32 lineSize)
{
	if (textBlock->cacheUseJustify)
	{
		return ftFont->DrawStringToBuffer(buf, x, y,
										  -textBlock->cacheOx + (int32)(Core::GetVirtualToPhysicalFactor() * xOffset),
										  -textBlock->cacheOy + (int32)(Core::GetVirtualToPhysicalFactor() * yOffset),
										  (int32)ceilf(Core::GetVirtualToPhysicalFactor() * w),
										  (int32)ceilf(Core::GetVirtualToPhysicalFactor() * lineSize),
										  drawText,
										  true);
	}

	return ftFont->DrawStringToBuffer(buf, x, y,
								     -textBlock->cacheOx + (int32)(Core::GetVirtualToPhysicalFactor() * xOffset),
								     -textBlock->cacheOy + (int32)(Core::GetVirtualToPhysicalFactor() * yOffset),
									 0,
									 0,
									 drawText,
									 true);
}

};