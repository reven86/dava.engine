#include "GPUTest.h"

GPUTest::GPUTest(GameCore& gameCore)
    : BaseScreen(gameCore, "GPUTest")
{
}

void GPUTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<Font> font(FTFont::Create("~res:/Fonts/korinna.ttf"));
    font->SetSize(16.0F);

    { //create control to display texture, loaded with default GPU
        ScopedPtr<UIControl> textureBackground(new UIControl(Rect(10.f, 10.f, 256.f, 128.f)));
        ScopedPtr<Texture> texture(Texture::CreateFromFile("~res:/TestData/GPUTest/Texture/texture.tex"));
        ScopedPtr<Sprite> sprite(Sprite::CreateFromTexture(texture, 0, 0, 128.f, 64.f));
        textureBackground->SetSprite(sprite, 0);
        AddControl(textureBackground);
    }

    { //create control to display sprite, loaded with default GPU
        ScopedPtr<UIControl> spriteBackground(new UIControl(Rect(276.f, 10.f, 256.f, 128.f)));
        ScopedPtr<Sprite> sprite(Sprite::Create("~res:/TestData/GPUTest/Sprite/texture"));
        spriteBackground->SetSprite(sprite, 0);
        AddControl(spriteBackground);
    }

    { //create control to display text with name of GPU, detected by system
        ScopedPtr<UIStaticText> textControl(new UIStaticText(Rect(10.0f, 148.0f, 512.f, 30.f)));
        textControl->SetTextColor(Color::White);
        textControl->SetFont(font);
        textControl->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);

        String text = Format("Detected GPU: %s", GlobalEnumMap<eGPUFamily>::Instance()->ToString(static_cast<eGPUFamily>(DeviceInfo::GetGPUFamily())));
        textControl->SetText(StringToWString(text));
        AddControl(textControl);
    }

    SetDebugDraw(true, true);
}
