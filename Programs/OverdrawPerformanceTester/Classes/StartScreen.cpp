#include "StartScreen.h"

#include "GameCore.h"
#include "OverdrawTestConfig.h"

using DAVA::Rect;

const Color StartScreen::Red(1.0f, 0.0f, 0.0f, 1.0f);
const Color StartScreen::Green(0.0f, 1.0f, 0.0f, 1.0f);

const float32 StartScreen::resolutionButtonsXOffset = 10.0f;
const float32 StartScreen::resolutionButtonsYOffset = 100.0f;
const float32 StartScreen::buttonHeight = 40.0f;
const float32 StartScreen::buttonWidth = 150.0f;
const float32 StartScreen::heigthDistanceBetweenButtons = 10.0f;
const float32 StartScreen::texturePixelFormatXOffset = 250.0f;
const float32 StartScreen::texturePixelFormatYOffset = 100.0f;
const float32 StartScreen::overdrawXOffset = 490.0f;
const float32 StartScreen::overdrawYOffset = 100.0f;
const float32 StartScreen::chartHeightYOffset = overdrawYOffset + buttonHeight * 4;
const float32 StartScreen::minFrametimeThreshold = 0.033f;
const float32 StartScreen::frametimeIncreaseStep = 0.016f;

const Array<StartScreen::ButtonInfo, 4> StartScreen::resolutionButtonsInfo =
{ {
{ L"2048", 1, Rect(resolutionButtonsXOffset, resolutionButtonsYOffset + buttonHeight * 0 + heigthDistanceBetweenButtons * 0, buttonWidth, buttonHeight), 2048 },
{ L"1024", 2, Rect(resolutionButtonsXOffset, resolutionButtonsYOffset + buttonHeight * 1 + heigthDistanceBetweenButtons * 1, buttonWidth, buttonHeight), 1024 },
{ L"512", 3, Rect(resolutionButtonsXOffset, resolutionButtonsYOffset + buttonHeight * 2 + heigthDistanceBetweenButtons * 2, buttonWidth, buttonHeight), 512 },
{ L"256", 4, Rect(resolutionButtonsXOffset, resolutionButtonsYOffset + buttonHeight * 3 + heigthDistanceBetweenButtons * 3, buttonWidth, buttonHeight), 256 }
} };

const Array<StartScreen::ButtonInfo, 9> StartScreen::texturePixelFormatButtonsInfo =
{ {
{ L"RGBA 8888", 1, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 0 + heigthDistanceBetweenButtons * 0, buttonWidth, buttonHeight), FORMAT_RGBA8888 },
{ L"RGBA 4444", 2, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 1 + heigthDistanceBetweenButtons * 1, buttonWidth, buttonHeight), FORMAT_RGBA4444 },
{ L"PVR 2", 3, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 2 + heigthDistanceBetweenButtons * 2, buttonWidth, buttonHeight), FORMAT_PVR2 },
{ L"PVR 4", 4, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 3 + heigthDistanceBetweenButtons * 3, buttonWidth, buttonHeight), FORMAT_PVR4 },
{ L"A8", 5, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 4 + heigthDistanceBetweenButtons * 4, buttonWidth, buttonHeight), FORMAT_A8 },
{ L"PVR2_2", 6, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 5 + heigthDistanceBetweenButtons * 5, buttonWidth, buttonHeight), FORMAT_PVR2_2 },
{ L"PVR4_2", 7, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 6 + heigthDistanceBetweenButtons * 6, buttonWidth, buttonHeight), FORMAT_PVR4_2 },
{ L"ETC1", 8, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 7 + heigthDistanceBetweenButtons * 7, buttonWidth, buttonHeight), FORMAT_ETC1 },
{ L"ETC2_RGBA", 9, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 8 + heigthDistanceBetweenButtons * 8, buttonWidth, buttonHeight), FORMAT_ETC2_RGBA }
} };

const Array<StartScreen::ButtonInfo, 2> StartScreen::overdrawButtonsInfo =
{ {
{ L"-", 1, Rect(overdrawXOffset, overdrawYOffset + buttonHeight, buttonWidth * 1.5f, buttonHeight), -1 },
{ L"+", 2, Rect(overdrawXOffset + buttonWidth * 1.5f, overdrawYOffset + buttonHeight, buttonWidth * 1.5f, buttonHeight), 1 }
} };

const Array<StartScreen::ButtonInfo, 2> StartScreen::chartHeightButtonsInfo =
{ {
{ L"-", 1, Rect(overdrawXOffset, chartHeightYOffset + buttonHeight, buttonWidth * 1.5f, buttonHeight), -1 },
{ L"+", 2, Rect(overdrawXOffset + buttonWidth * 1.5f, chartHeightYOffset + buttonHeight, buttonWidth * 1.5f, buttonHeight), 1 }
} };

StartScreen::StartScreen()
    : BaseScreen()
{
}

void StartScreen::LoadResources()
{
    BaseScreen::LoadResources();

    Rect screenRect = GetRect();
    Size2i screenSize = DAVA::UIControlSystem::Instance()->vcs->GetVirtualScreenSize();
    screenRect.dx = static_cast<float32>(screenSize.dx);
    screenRect.dy = static_cast<float32>(screenSize.dy);
    SetRect(screenRect);

    ScopedPtr<UIButton> startButton(CreateButton(Rect(5, 5, screenRect.dx, buttonHeight), L"Start"));
    startButton->SetDebugDraw(true);
    startButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &StartScreen::OnStart));
    AddControl(startButton);

    CreateLabel({ overdrawXOffset, overdrawYOffset - buttonHeight, buttonWidth * 3.0f, buttonHeight }, L"Overdraw screens count");
    overdrawCountLabel = new UIStaticText(Rect(overdrawXOffset, overdrawYOffset, buttonWidth * 3.0f, buttonHeight));
    overdrawCountLabel->SetFont(font);
    overdrawCountLabel->SetTextColor(Color::White);
    overdrawCountLabel->SetText(Format(L"%d", OverdrawTestConfig::overdrawScreensCount));
    overdrawCountLabel->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    overdrawCountLabel->SetDebugDraw(true);
    AddControl(overdrawCountLabel);
    InitializeButtons(overdrawButtonsInfo, overdrawButtons, Message(this, &StartScreen::OnChangeOverdrawButtonClick), false);

    CreateLabel({ overdrawXOffset, chartHeightYOffset - buttonHeight, buttonWidth * 3.0f, buttonHeight }, L"Max frametime");
    chartHeightLabel = new UIStaticText(Rect(overdrawXOffset, chartHeightYOffset, buttonWidth * 3.0f, buttonHeight));
    chartHeightLabel->SetFont(font);
    chartHeightLabel->SetTextColor(Color::White);
    chartHeightLabel->SetText(Format(L"%.3f", OverdrawTestConfig::chartHeight));
    chartHeightLabel->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    chartHeightLabel->SetDebugDraw(true);
    AddControl(chartHeightLabel);
    InitializeButtons(chartHeightButtonsInfo, chartHeightButtons, Message(this, &StartScreen::OnChangeChartHeightButtonClick), false);

    CreateLabel({ resolutionButtonsXOffset, resolutionButtonsYOffset - buttonHeight, buttonWidth, buttonHeight }, L"Tex resolution");
    InitializeButtons(resolutionButtonsInfo, resolutionButtons, Message(this, &StartScreen::OnResolutionButtonClick));

    CreateLabel({ texturePixelFormatXOffset, texturePixelFormatYOffset - buttonHeight, buttonWidth, buttonHeight }, L"Tex format");
    InitializeButtons(texturePixelFormatButtonsInfo, texturePixelFormatButtons, Message(this, &StartScreen::OnTextureFormatButtonClick));

    SetDebugDraw(false, false);
}

void StartScreen::UnloadResources()
{
    BaseScreen::UnloadResources();

    ReleaseButtons(resolutionButtons);
    ReleaseButtons(texturePixelFormatButtons);
    ReleaseButtons(overdrawButtons);
    ReleaseButtons(chartHeightButtons);

    SafeRelease(overdrawCountLabel);
    SafeRelease(chartHeightLabel);
}

void StartScreen::CreateLabel(const Rect&& rect, const WideString&& caption)
{
    ScopedPtr<UIStaticText> label(new UIStaticText(rect));
    label->SetFont(font);
    label->SetTextColor(Color::White);
    label->SetText(caption);
    label->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    label->SetDebugDraw(false);
    AddControl(label);
}

void StartScreen::ReleaseButtons(DAVA::UnorderedMap<UIButton*, ButtonInfo>& buttons)
{
    for (auto& btn : buttons)
    {
        UIButton* buttonToDelete = btn.first;
        SafeRelease(buttonToDelete);
    }
    buttons.clear();
}

void StartScreen::OnStart(BaseObject* caller, void* param, void* callerData)
{
    SetNextScreen();
}

void StartScreen::OnResolutionButtonClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* pickedButton = DynamicTypeCheck<UIButton*>(sender);
    for (auto& btn : resolutionButtons)
    {
        if (btn.first->GetTag() == pickedButton->GetTag())
        {
            btn.first->SetDebugDrawColor(Green);
            OverdrawTestConfig::textureResolution = btn.second.data;
        }
        else
            btn.first->SetDebugDrawColor(Red);
    }
}

void StartScreen::OnTextureFormatButtonClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* pickedButton = DynamicTypeCheck<UIButton*>(sender);

    for (auto& btn : texturePixelFormatButtons)
    {
        if (btn.first->GetTag() == pickedButton->GetTag())
        {
            btn.first->SetDebugDrawColor(Green);
            OverdrawTestConfig::pixelFormat = static_cast<DAVA::PixelFormat>(btn.second.data);
        }
        else
            btn.first->SetDebugDrawColor(Red);
    }
}

void StartScreen::OnChangeOverdrawButtonClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* pickedButton = DynamicTypeCheck<UIButton*>(sender);

    for (auto& btn : overdrawButtons)
    {
        if (btn.first->GetTag() == pickedButton->GetTag())
        {
            OverdrawTestConfig::overdrawScreensCount = Max(static_cast<uint8>(1), static_cast<uint8>(OverdrawTestConfig::overdrawScreensCount + btn.second.data));
            overdrawCountLabel->SetText(Format(L"%d", OverdrawTestConfig::overdrawScreensCount));
        }
    }
}

void StartScreen::OnChangeChartHeightButtonClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* pickedButton = DynamicTypeCheck<UIButton*>(sender);

    for (auto& btn : overdrawButtons)
    {
        if (btn.first->GetTag() == pickedButton->GetTag())
        {
            OverdrawTestConfig::chartHeight = Max(minFrametimeThreshold, OverdrawTestConfig::chartHeight + btn.second.data * frametimeIncreaseStep);
            chartHeightLabel->SetText(Format(L"%.3f", OverdrawTestConfig::chartHeight));
        }
    }
}
