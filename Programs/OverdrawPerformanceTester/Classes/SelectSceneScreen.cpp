#include "SelectSceneScreen.h"

#include "GameCore.h"
#include "TesterConfig.h"

using DAVA::Rect;

const Color SelectSceneScreen::Red(1.0f, 0.0f, 0.0f, 1.0f);
const Color SelectSceneScreen::Green(0.0f, 1.0f, 0.0f, 1.0f);

const float32 SelectSceneScreen::resolutionButtonsXOffset = 10.0f;
const float32 SelectSceneScreen::resolutionButtonsYOffset = 100.0f;
const float32 SelectSceneScreen::buttonHeight = 40.0f;
const float32 SelectSceneScreen::buttonWidth = 150.0f;
const float32 SelectSceneScreen::heigthDistanceBetweenButtons = 10.0f;
const float32 SelectSceneScreen::texturePixelFormatXOffset = 250.0f;
const float32 SelectSceneScreen::texturePixelFormatYOffset = 100.0f;
const float32 SelectSceneScreen::overdrawXOffset = 490.0f;
const float32 SelectSceneScreen::overdrawYOffset = 100.0f;
const float32 SelectSceneScreen::chartHeightYOffset = overdrawYOffset + buttonHeight * 4;
const float32 SelectSceneScreen::minFrametimeThreshold = 0.033f;
const float32 SelectSceneScreen::frametimeIncreaseStep = 0.016f;

const Array<SelectSceneScreen::ButtonInfo, 4> SelectSceneScreen::resolutionButtonsInfo =
{ {
    { L"2048", 1, Rect(resolutionButtonsXOffset, resolutionButtonsYOffset + buttonHeight * 0 + heigthDistanceBetweenButtons * 0, buttonWidth, buttonHeight), 2048 },
    { L"1024", 2, Rect(resolutionButtonsXOffset, resolutionButtonsYOffset + buttonHeight * 1 + heigthDistanceBetweenButtons * 1, buttonWidth, buttonHeight), 1024 },
    { L"512", 3, Rect(resolutionButtonsXOffset, resolutionButtonsYOffset + buttonHeight * 2 + heigthDistanceBetweenButtons * 2, buttonWidth, buttonHeight), 512 },
    { L"256", 4, Rect(resolutionButtonsXOffset, resolutionButtonsYOffset + buttonHeight * 3 + heigthDistanceBetweenButtons * 3, buttonWidth, buttonHeight), 256 }
} };

const Array<SelectSceneScreen::ButtonInfo, 5> SelectSceneScreen::texturePixelFormatButtonsInfo =
{ {
    { L"RGBA 8888", 1, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 0 + heigthDistanceBetweenButtons * 0, buttonWidth, buttonHeight), FORMAT_RGBA8888 },
    { L"RGBA 4444", 2, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 1 + heigthDistanceBetweenButtons * 1, buttonWidth, buttonHeight), FORMAT_RGBA4444 },
    { L"PVR 2", 3, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 2 + heigthDistanceBetweenButtons * 2, buttonWidth, buttonHeight), FORMAT_PVR2 },
    { L"PVR 4", 4, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 3 + heigthDistanceBetweenButtons * 3, buttonWidth, buttonHeight), FORMAT_PVR4 },
    { L"A8", 5, Rect(texturePixelFormatXOffset, texturePixelFormatYOffset + buttonHeight * 4 + heigthDistanceBetweenButtons * 4, buttonWidth, buttonHeight), FORMAT_A8 }
} };

const Array<SelectSceneScreen::ButtonInfo, 2> SelectSceneScreen::overdrawButtonsInfo =
{ {
    { L"-", 1, Rect(overdrawXOffset, overdrawYOffset + buttonHeight, buttonWidth * 1.5f, buttonHeight), -1 },
    { L"+", 2, Rect(overdrawXOffset + buttonWidth * 1.5f, overdrawYOffset + buttonHeight, buttonWidth * 1.5f, buttonHeight), 1 }
} };

const Array<SelectSceneScreen::ButtonInfo, 2> SelectSceneScreen::chartHeightButtonsInfo =
{ {
    { L"-", 1, Rect(overdrawXOffset, chartHeightYOffset + buttonHeight, buttonWidth * 1.5f, buttonHeight), -1 },
    { L"+", 2, Rect(overdrawXOffset + buttonWidth * 1.5f, chartHeightYOffset + buttonHeight, buttonWidth * 1.5f, buttonHeight), 1 }
    } };

SelectSceneScreen::SelectSceneScreen()
    : BaseScreen()
{
}

void SelectSceneScreen::LoadResources()
{
    BaseScreen::LoadResources();

    Rect screenRect = GetRect();
    Size2i screenSize = DAVA::UIControlSystem::Instance()->vcs->GetVirtualScreenSize();
    screenRect.dx = static_cast<float32>(screenSize.dx);
    screenRect.dy = static_cast<float32>(screenSize.dy);
    SetRect(screenRect);

    ScopedPtr<UIButton> startButton(CreateButton(Rect(5, 5, screenRect.dx, buttonHeight), L"Start"));
    startButton->SetDebugDraw(true);
    startButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SelectSceneScreen::OnStart));
    AddControl(startButton);
    
    CreateLabel({ overdrawXOffset, overdrawYOffset - buttonHeight, buttonWidth * 3.0f, buttonHeight }, L"Overdraw screens count");
    overdrawCountLabel = new UIStaticText(Rect(overdrawXOffset, overdrawYOffset, buttonWidth * 3.0f, buttonHeight));
    overdrawCountLabel->SetFont(font);
    overdrawCountLabel->SetTextColor(Color::White);
    overdrawCountLabel->SetText(Format(L"%d", TesterConfig::overdrawScreensCount));
    overdrawCountLabel->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    overdrawCountLabel->SetDebugDraw(true);
    AddControl(overdrawCountLabel);
    InitializeButtons(overdrawButtonsInfo, overdrawButtons, Message(this, &SelectSceneScreen::OnChangeOverdrawButtonClick), false);

    CreateLabel({ overdrawXOffset, chartHeightYOffset - buttonHeight, buttonWidth * 3.0f, buttonHeight }, L"Max frametime");
    chartHeightLabel = new UIStaticText(Rect(overdrawXOffset, chartHeightYOffset, buttonWidth * 3.0f, buttonHeight));
    chartHeightLabel->SetFont(font);
    chartHeightLabel->SetTextColor(Color::White);
    chartHeightLabel->SetText(Format(L"%.3f", TesterConfig::chartHeight));
    chartHeightLabel->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    chartHeightLabel->SetDebugDraw(true);
    AddControl(chartHeightLabel);
    InitializeButtons(chartHeightButtonsInfo, chartHeightButtons, Message(this, &SelectSceneScreen::OnChangeChartHeightButtonClick), false);

    CreateLabel({ resolutionButtonsXOffset, resolutionButtonsYOffset - buttonHeight, buttonWidth, buttonHeight }, L"Tex resolution");
    InitializeButtons(resolutionButtonsInfo, resolutionButtons, Message(this, &SelectSceneScreen::OnResolutionButtonClick));

    CreateLabel({ texturePixelFormatXOffset, texturePixelFormatYOffset - buttonHeight, buttonWidth, buttonHeight }, L"Tex format");
    InitializeButtons(texturePixelFormatButtonsInfo, texturePixelFormatButtons, Message(this, &SelectSceneScreen::OnTextureFormatButtonClick));

    SetDebugDraw(false, false);
}

void SelectSceneScreen::UnloadResources()
{
    BaseScreen::UnloadResources();

    ReleaseButtons(resolutionButtons);
    ReleaseButtons(texturePixelFormatButtons);
    ReleaseButtons(overdrawButtons);
    ReleaseButtons(chartHeightButtons);

    SafeRelease(overdrawCountLabel);
    SafeRelease(chartHeightLabel);
}

void SelectSceneScreen::CreateLabel(const Rect&& rect, const WideString&& caption)
{
    ScopedPtr<UIStaticText> label(new UIStaticText(rect));
    label->SetFont(font);
    label->SetTextColor(Color::White);
    label->SetText(caption);
    label->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    label->SetDebugDraw(false);
    AddControl(label);
}

void SelectSceneScreen::ReleaseButtons(DAVA::UnorderedMap<UIButton*, ButtonInfo>& buttons)
{
    for (auto& btn : buttons)
    {
        UIButton* buttonToDelete = btn.first;
        SafeRelease(buttonToDelete);
    }
    buttons.clear();
}

void SelectSceneScreen::OnStart(BaseObject* caller, void* param, void* callerData)
{
    SetNextScreen();
}

void SelectSceneScreen::OnResolutionButtonClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* pickedButton = DynamicTypeCheck<UIButton*>(sender);
    for (auto& btn : resolutionButtons)
    {
        if (btn.first->GetTag() == pickedButton->GetTag())
        {
            btn.first->SetDebugDrawColor(Green);
            TesterConfig::textureResolution = btn.second.data;
        }
        else
            btn.first->SetDebugDrawColor(Red);
    }
}

void SelectSceneScreen::OnTextureFormatButtonClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* pickedButton = DynamicTypeCheck<UIButton*>(sender);
    
    for (auto& btn : texturePixelFormatButtons)
    {
        if (btn.first->GetTag() == pickedButton->GetTag())
        {
            btn.first->SetDebugDrawColor(Green);
            TesterConfig::pixelFormat = static_cast<DAVA::PixelFormat>(btn.second.data);
        }
        else
            btn.first->SetDebugDrawColor(Red);
    }
}

void SelectSceneScreen::OnChangeOverdrawButtonClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* pickedButton = DynamicTypeCheck<UIButton*>(sender);

    for (auto& btn : overdrawButtons)
    {
        if (btn.first->GetTag() == pickedButton->GetTag())
        {
            TesterConfig::overdrawScreensCount = Max(static_cast<uint8>(1), static_cast<uint8>(TesterConfig::overdrawScreensCount + btn.second.data));
            overdrawCountLabel->SetText(Format(L"%d", TesterConfig::overdrawScreensCount));
        }
    }
}

void SelectSceneScreen::OnChangeChartHeightButtonClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* pickedButton = DynamicTypeCheck<UIButton*>(sender);

    for (auto& btn : overdrawButtons)
    {
        if (btn.first->GetTag() == pickedButton->GetTag())
        {
            TesterConfig::chartHeight = Max(minFrametimeThreshold, TesterConfig::chartHeight + btn.second.data * frametimeIncreaseStep);
            chartHeightLabel->SetText(Format(L"%.3f", TesterConfig::chartHeight));
        }
    }
}
