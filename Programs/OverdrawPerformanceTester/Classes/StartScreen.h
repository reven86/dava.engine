#pragma once

#include "BaseScreen.h"

using namespace DAVA;

class StartScreen : public BaseScreen
{
public:
    StartScreen();

    virtual void LoadResources();
    virtual void UnloadResources();

    virtual ~StartScreen()
    {
    }

private:
    struct ButtonInfo
    {
        WideString caption;
        int32 tag;
        Rect rect;
        int16 data;
    };

    void CreateLabel(const Rect&& rect, const WideString&& caption);
    void ReleaseButtons(UnorderedMap<UIButton*, ButtonInfo>& buttons);
    void OnStart(BaseObject* caller, void* param, void* callerData);
    void OnResolutionButtonClick(BaseObject* sender, void* data, void* callerData);
    void OnTextureFormatButtonClick(BaseObject* sender, void* data, void* callerData);
    void OnChangeOverdrawButtonClick(BaseObject* sender, void* data, void* callerData);
    void OnChangeChartHeightButtonClick(BaseObject* sender, void* data, void* callerData);

    template <size_t size>
    void InitializeButtons(const Array<ButtonInfo, size>& buttonsInfo, UnorderedMap<UIButton*, ButtonInfo>& buttonsMap, Message&& msg, bool isFirstButtonGreen = true)
    {
        for (size_t i = 0; i < buttonsInfo.size(); i++)
        {
            UIButton* btn = CreateButton(buttonsInfo[i].rect, buttonsInfo[i].caption);
            btn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, msg);
            if (isFirstButtonGreen && i == 0)
                btn->SetDebugDrawColor(Green);
            btn->SetDebugDraw(true);
            btn->SetTag(buttonsInfo[i].tag);
            AddControl(btn);
            buttonsMap[btn] = buttonsInfo[i];
        }
    }

    UIStaticText* overdrawCountLabel = nullptr;
    UIStaticText* chartHeightLabel = nullptr;

    UnorderedMap<UIButton*, ButtonInfo> resolutionButtons;
    UnorderedMap<UIButton*, ButtonInfo> texturePixelFormatButtons;
    UnorderedMap<UIButton*, ButtonInfo> overdrawButtons;
    UnorderedMap<UIButton*, ButtonInfo> chartHeightButtons;

    static const Color Red;
    static const Color Green;

    static const float32 resolutionButtonsXOffset;
    static const float32 resolutionButtonsYOffset;
    static const float32 buttonHeight;
    static const float32 buttonWidth;
    static const float32 heigthDistanceBetweenButtons;
    static const float32 texturePixelFormatXOffset;
    static const float32 texturePixelFormatYOffset;
    static const float32 overdrawXOffset;
    static const float32 overdrawYOffset;
    static const float32 chartHeightYOffset;
    static const float32 minFrametimeThreshold;
    static const float32 frametimeIncreaseStep;

    static const Array<ButtonInfo, 4> resolutionButtonsInfo;
    static const Array<ButtonInfo, 9> texturePixelFormatButtonsInfo;
    static const Array<ButtonInfo, 2> overdrawButtonsInfo;
    static const Array<ButtonInfo, 2> chartHeightButtonsInfo;
};
