#include "Tests/CoreTest.h"

using namespace DAVA;

CoreTest::CoreTest()
    : BaseScreen("CoreTest")
{
}

void CoreTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<UIButton> quitBtn(new UIButton(Rect(10, 10, 200, 50)));
    quitBtn->SetStateText(0xFF, L"Core::Quit");
    quitBtn->SetStateColor(0xFF, Color::White);
    quitBtn->SetDebugDraw(true);
    quitBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CoreTest::Quit));
    AddControl(quitBtn);
}

void CoreTest::UnloadResources()
{
    BaseScreen::UnloadResources();
}

void CoreTest::Quit(BaseObject* obj, void* data, void* callerData)
{
#if !defined(__DAVAENGINE_COREV2__)
    Core::Instance()->Quit();
#endif
}
