#include "SelectSceneScreen.h"

#include "UI/Focus/UIFocusComponent.h"

#include "GameCore.h"
#include "TesterConfig.h"

#define SETTINGS_PATH "~doc:/SceneViewerSettings.bin"

using DAVA::Rect;

const Color SelectSceneScreen::Green(0.0f, 1.0f, 0.0f, 1.0f);
const Color SelectSceneScreen::Red(1.0f, 0.0f, 0.0f, 1.0f);
const float32 SelectSceneScreen::resolutionButtonsXOffset = 10.0f;
const float32 SelectSceneScreen::resolutionButtonsYOffset = 100.0f;
const float32 SelectSceneScreen::buttonHeight = 40.0f;
const float32 SelectSceneScreen::buttonWidth = 150.0f;
const float32 SelectSceneScreen::heigthDistanceBetweenButtons = 10.0f;
const float32 SelectSceneScreen::texturePixelFormatXOffset = 250.0f;
const float32 SelectSceneScreen::texturePixelFormatYOffset = 100.0f;
const float32 SelectSceneScreen::overdrawXOffset = 490.0f;
const float32 SelectSceneScreen::overdrawYOffset = 100.0f;

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

SelectSceneScreen::SelectSceneScreen()
    : BaseScreen()
    , fileNameText(NULL)
    , fileSystemDialog(NULL)
{
//     LoadSettings();
}

void SelectSceneScreen::LoadResources()
{
    BaseScreen::LoadResources();

    Rect screenRect = GetRect();
    Size2i screenSize = DAVA::UIControlSystem::Instance()->vcs->GetVirtualScreenSize();
    screenRect.dx = static_cast<float32>(screenSize.dx);
    screenRect.dy = static_cast<float32>(screenSize.dy);
    SetRect(screenRect);

    ScopedPtr<UIButton> startButton(CreateButton(Rect(0, 5, screenRect.dx, buttonHeight), L"Start"));
    startButton->SetDebugDraw(true);
    startButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SelectSceneScreen::OnStart));
    
    ScopedPtr<UIStaticText> overdrawLabel(new UIStaticText(Rect(overdrawXOffset, overdrawYOffset - buttonHeight, buttonWidth * 3.0f, buttonHeight)));
    overdrawLabel->SetFont(font);
    overdrawLabel->SetTextColor(Color::White);
    overdrawLabel->SetText(L"Overdraw screens count");
    overdrawLabel->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    overdrawLabel->SetDebugDraw(false);
    AddControl(overdrawLabel);

    overdrawCountLabel = new UIStaticText(Rect(overdrawXOffset, overdrawYOffset, buttonWidth * 3.0f, buttonHeight));
    overdrawCountLabel->SetFont(font);
    overdrawCountLabel->SetTextColor(Color::White);
    overdrawCountLabel->SetText(Format(L"%d", TesterConfig::overdrawScreensCount));
    overdrawCountLabel->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    overdrawCountLabel->SetDebugDraw(true);
    AddControl(overdrawCountLabel);

    for (size_t i = 0; i < overdrawButtonsInfo.size(); i++)
    {
        UIButton* btn = CreateButton(overdrawButtonsInfo[i].rect, overdrawButtonsInfo[i].caption);
        btn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SelectSceneScreen::OnChangeOverdrawButtonClick));
        btn->SetDebugDraw(true);
        btn->SetTag(overdrawButtonsInfo[i].tag);
        AddControl(btn);
        overdrawButtons[btn] = overdrawButtonsInfo[i];
    }
    
    ScopedPtr<UIStaticText> resLabel(new UIStaticText(Rect(resolutionButtonsXOffset, resolutionButtonsYOffset - buttonHeight, buttonWidth, buttonHeight)));
    resLabel->SetFont(font);
    resLabel->SetTextColor(Color::White);
    resLabel->SetText(L"Tex resolution");
    resLabel->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    resLabel->SetDebugDraw(false);
    AddControl(resLabel);

    for (size_t i = 0; i < resolutionButtonsInfo.size(); i++)
    {
        UIButton* btn = CreateButton(resolutionButtonsInfo[i].rect, resolutionButtonsInfo[i].caption);
        btn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SelectSceneScreen::OnResolutionButtonClick));
        if (i == 0)
            btn->SetDebugDrawColor(Green);
        btn->SetDebugDraw(true);
        btn->SetTag(resolutionButtonsInfo[i].tag);
        AddControl(btn);
        resolutionButtons[btn] = resolutionButtonsInfo[i];
    }

    ScopedPtr<UIStaticText> formatLabel(new UIStaticText(Rect(texturePixelFormatXOffset, texturePixelFormatYOffset - buttonHeight, buttonWidth, buttonHeight)));
    formatLabel->SetFont(font);
    formatLabel->SetTextColor(Color::White);
    formatLabel->SetText(L"Tex format");
    formatLabel->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    formatLabel->SetDebugDraw(false);
    AddControl(formatLabel);

    for (size_t i = 0; i < texturePixelFormatButtonsInfo.size(); i++)
    {
        UIButton* btn = CreateButton(texturePixelFormatButtonsInfo[i].rect, texturePixelFormatButtonsInfo[i].caption);
        btn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SelectSceneScreen::OnTextureFormatButtonClick));
        if (i == 0)
            btn->SetDebugDrawColor(Green);
        btn->SetDebugDraw(true);
        btn->SetTag(texturePixelFormatButtonsInfo[i].tag);
        AddControl(btn);
        texturePixelFormatButtons[btn] = texturePixelFormatButtonsInfo[i];
    }

    AddControl(startButton);

    SetDebugDraw(false, false);
}

void SelectSceneScreen::UnloadResources()
{
    SafeRelease(fileNameText);
    BaseScreen::UnloadResources();

    ReleaseButtons(resolutionButtons);
    ReleaseButtons(texturePixelFormatButtons);
    ReleaseButtons(overdrawButtons);

    SafeRelease(overdrawCountLabel);

    delete inputDelegate;
    SafeRelease(overdrawInfoMessage);
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
