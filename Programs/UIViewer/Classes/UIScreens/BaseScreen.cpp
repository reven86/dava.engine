#include "UIScreens/BaseScreen.h"

#include <Input/KeyboardDevice.h>
#include <UI/UIButton.h>
#include <UI/UIScreenManager.h>

DAVA::int32 BaseScreen::screensCount = 0;

BaseScreen::BaseScreen()
    : UIScreen()
    , screenID(screensCount++)
{
    DAVA::UIScreenManager::Instance()->RegisterScreen(screenID, this);
    GetOrCreateComponent<DAVA::UIControlBackground>();
}

bool BaseScreen::SystemInput(DAVA::UIEvent* currentInput)
{
    if ((currentInput->key == DAVA::Key::BACK) && (DAVA::UIEvent::Phase::KEY_DOWN == currentInput->phase))
    {
        SetPreviousScreen();
        return true;
    }

    return UIScreen::SystemInput(currentInput);
}

void BaseScreen::LoadResources()
{
    GetBackground()->SetColor(DAVA::Color(0.f, 0.f, 0.f, 1.f));
    DVASSERT(!font);
    DVASSERT(!fontSmall);
    font = DAVA::FTFont::Create("~res:/Fonts/korinna.ttf");
    font->SetSize(20.f);
    fontSmall = DAVA::FTFont::Create("~res:/Fonts/korinna.ttf");
    fontSmall->SetSize(15.f);
}

void BaseScreen::UnloadResources()
{
    font.reset();
    fontSmall.reset();
    RemoveAllControls();
}

DAVA::UIButton* BaseScreen::CreateButton(const DAVA::Rect& rect, const DAVA::WideString& text)
{
    using namespace DAVA;
    DVASSERT(font);

    UIButton* button = new UIButton(rect);
    button->SetStateText(UIControl::STATE_NORMAL, text);
    button->SetStateTextAlign(UIControl::STATE_NORMAL, ALIGN_HCENTER | ALIGN_VCENTER);

    button->SetStateFontColor(UIControl::STATE_NORMAL, Color::White);
    button->SetStateFont(UIControl::STATE_NORMAL, font);
    button->SetStateTextColorInheritType(UIControl::STATE_NORMAL, UIControlBackground::COLOR_IGNORE_PARENT);

    button->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    button->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.4f, 0.5f, 0.4f, 0.9f));
    button->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    button->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.65f, 0.75f, 0.65f, 0.9f));

    return button;
}

void BaseScreen::SetPreviousScreen() const
{
    if (screenID)
    {
        DAVA::UIScreenManager::Instance()->SetScreen(screenID - 1);
    }
}

void BaseScreen::SetNextScreen() const
{
    if (screenID < screensCount - 1)
    {
        DAVA::UIScreenManager::Instance()->SetScreen(screenID + 1);
    }
}
