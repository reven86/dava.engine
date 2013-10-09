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



#include "TextureConverterCell.h"
#include "ControlsFactory.h"

#include "Render/LibPVRHelper.h"
#include "../Qt/Main/QtUtils.h"

TextureConverterCell::TextureConverterCell(const Rect &rect, const String &cellIdentifier)
:   UIListCell(Rect(rect.x, rect.y, rect.dx, (float32)GetCellHeight()), cellIdentifier)
{
    ControlsFactory::CustomizeListCell(this, L"");
    
    textureName = new UIStaticText(Rect(0, 0, rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureName->SetFont(ControlsFactory::GetFont12());
	textureName->SetTextColor(ControlsFactory::GetColorDark());
    textureName->SetInputEnabled(false);
    AddControl(textureName);
    
    preview = new UIControl(Rect(0, textureName->GetRect().y + textureName->GetRect().dy, 
                                 rect.dx, ControlsFactory::TEXTURE_PREVIEW_HEIGHT));
    preview->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
    preview->SetInputEnabled(false);
    AddControl(preview);
    

    textureFormat = new UIStaticText(Rect(0, preview->GetRect().y + preview->GetRect().dy, 
                                          rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureFormat->SetFont(ControlsFactory::GetFont12());
	textureFormat->SetTextColor(ControlsFactory::GetColorDark());
    textureFormat->SetInputEnabled(false);
    AddControl(textureFormat);
    
    textureDimensions = new UIStaticText(Rect(0, textureFormat->GetRect().y + textureFormat->GetRect().dy, 
                                          rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureDimensions->SetFont(ControlsFactory::GetFont12());
	textureDimensions->SetTextColor(ControlsFactory::GetColorDark());
    textureDimensions->SetInputEnabled(false);
    AddControl(textureDimensions);

    textureSize = new UIStaticText(Rect(0, textureDimensions->GetRect().y + textureDimensions->GetRect().dy, 
                                              rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureSize->SetFont(ControlsFactory::GetFont12());
	textureSize->SetTextColor(ControlsFactory::GetColorDark());
    textureSize->SetInputEnabled(false);
    AddControl(textureSize);

    
    UIControl *line =  ControlsFactory::CreateLine(Rect(0, (float32)GetCellHeight() - 1.f, rect.dx, 1.f), Color(0.2f, 0.2f, 0.2f, 0.8f));
    AddControl(line);
    SafeRelease(line);
}

TextureConverterCell::~TextureConverterCell()
{
    SafeRelease(preview);
    SafeRelease(textureName);
    SafeRelease(textureFormat);
    SafeRelease(textureDimensions);
    SafeRelease(textureSize);
}

void TextureConverterCell::SetTexture(const FilePath &texturePath)
{
    textureFormat->SetText(L"");
    textureSize->SetText(L"");
    textureName->SetText(StringToWString(texturePath.GetFilename()));
    
    Texture *texture = Texture::CreateFromFile(texturePath);
    Sprite *s = Sprite::CreateFromTexture(texture, 0, 0, (float32)texture->width, (float32)texture->height);
    preview->SetSprite(s, 0);
    
    if(texturePath.IsEqualToExtension(".png"))
    {
        String pngFormat = Texture::GetPixelFormatString(texture->format);
        
        FilePath pvrPath = FilePath::CreateWithNewExtension(texturePath, ".pvr");
        Texture *pvrTex = Texture::CreateFromFile(pvrPath);
        if(pvrTex)
        {
            PixelFormat format = LibPVRHelper::GetPixelFormat(pvrPath);
            uint32 pvrDataSize = LibPVRHelper::GetDataSize(pvrPath);

            String pvrFormat = Texture::GetPixelFormatString(format);
            textureFormat->SetText(StringToWString(pngFormat + "/" + pvrFormat));
            
            textureSize->SetText(SizeInBytesToWideString(pvrDataSize));
            
            SafeRelease(pvrTex);
        }
        else 
        {
            textureFormat->SetText(StringToWString(pngFormat));
        }
    }
    else if(texturePath.IsEqualToExtension(".pvr"))
    {
        PixelFormat format = LibPVRHelper::GetPixelFormat(texturePath);
        uint32 pvrDataSize = LibPVRHelper::GetDataSize(texturePath);

        String pvrFormat = Texture::GetPixelFormatString(format);
        textureSize->SetText(SizeInBytesToWideString(pvrDataSize));

        FilePath pngPath = FilePath::CreateWithNewExtension(texturePath, ".png");
        Texture *pngTex = Texture::CreateFromFile(pngPath);
        if(pngTex)
        {
            String pngFormat = Texture::GetPixelFormatString(pngTex->format);
            textureFormat->SetText(StringToWString(pngFormat + "/" + pvrFormat));
            
            SafeRelease(pngTex);
        }
        else 
        {
            textureFormat->SetText(StringToWString(pvrFormat));
        }
    }

    textureDimensions->SetText(Format(L"%d x %d", texture->width, texture->height));
    
    SafeRelease(texture);
    SafeRelease(s);
}


int32 TextureConverterCell::GetCellHeight()
{
    return (ControlsFactory::BUTTON_HEIGHT * 4 + ControlsFactory::TEXTURE_PREVIEW_HEIGHT + 1);
}

