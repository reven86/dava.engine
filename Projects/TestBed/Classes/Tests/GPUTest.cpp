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


#include "GPUTest.h"

GPUTest::GPUTest()
    : BaseScreen("GPUTest")
{
}

void GPUTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<Font> font(FTFont::Create("~res:/Fonts/korinna.ttf"));
    font->SetSize(16.0F);

    ScopedPtr<UIControl> imageBackground(new UIControl(Rect(10.f, 10.f, 256.f, 128.f)));

    ScopedPtr<Texture> texture(Texture::CreateFromFile("~res:/TestData/GPUTest/texture.tex"));
    ScopedPtr<Sprite> sprite(Sprite::CreateFromTexture(texture, 0, 0, 128.f, 64.f));

    imageBackground->SetSprite(sprite, 0);

    AddControl(imageBackground);

    ScopedPtr<UIStaticText> textControl(new UIStaticText(Rect(10.0f, 138.0f, 256.f, 30.f)));
    textControl->SetTextColor(Color::White);
    textControl->SetFont(font);
    textControl->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);

    String text = Format("Detected GPU: %s", GlobalEnumMap<eGPUFamily>::Instance()->ToString(static_cast<eGPUFamily>(DeviceInfo::GetGPUFamily())));
    textControl->SetText(StringToWString(text));
    AddControl(textControl);
}
