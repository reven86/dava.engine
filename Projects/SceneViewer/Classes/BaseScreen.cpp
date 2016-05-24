#include "BaseScreen.h"

int32 BaseScreen::screensCount = 0;

BaseScreen::BaseScreen()
    : UIScreen()
    , font(nullptr)
    , screenID(screensCount++)
    , loaded(false)
{
    UIScreenManager::Instance()->RegisterScreen(screenID, this);
}

bool BaseScreen::SystemInput(UIEvent* currentInput)
{
    if ((currentInput->key == Key::BACK) && (UIEvent::Phase::KEY_DOWN == currentInput->phase))
    {
        SetPreviousScreen();
    }
    else
    {
        return UIScreen::SystemInput(currentInput);
    }
    return true;
}

void BaseScreen::SystemScreenSizeChanged(const Rect& newFullScreenSize)
{
    UnloadResources();
    LoadResources();
}

void BaseScreen::LoadResources()
{
    if (!loaded)
    {
        GetBackground()->SetColor(Color(0.f, 0.f, 0.f, 1.f));
        DVASSERT(font == NULL);
        font = FTFont::Create("~res:/Fonts/korinna.ttf");
        font->SetSize(20.f);

        loaded = true;
    }
}

void BaseScreen::UnloadResources()
{
    SafeRelease(font);
    RemoveAllControls();
    loaded = false;
}

UIButton* BaseScreen::CreateButton(const Rect& rect, const WideString& text)
{
    DVASSERT(font);

    UIButton* button = new UIButton(rect);
    button->SetStateText(UIControl::STATE_NORMAL, text);
    button->SetStateTextAlign(UIControl::STATE_NORMAL, ALIGN_HCENTER | ALIGN_VCENTER);
    button->SetStateFont(UIControl::STATE_NORMAL, font);
    button->SetStateFontColor(UIControl::STATE_NORMAL, Color::White);
    button->SetStateFontColor(UIControl::STATE_PRESSED_INSIDE, Color(0.7f, 0.7f, 0.7f, 1.f));

    return button;
}

void BaseScreen::SetPreviousScreen() const
{
    if (screenID)
    {
        UIScreenManager::Instance()->SetScreen(screenID - 1);
    }
}

void BaseScreen::SetNextScreen() const
{
    if (screenID < screensCount - 1)
    {
        UIScreenManager::Instance()->SetScreen(screenID + 1);
    }
}
