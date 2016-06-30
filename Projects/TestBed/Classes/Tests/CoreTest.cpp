#include "Tests/CoreTest.h"
#include "Infrastructure/GameCore.h"

#include <Engine/Engine.h>

using namespace DAVA;

CoreTest::CoreTest(GameCore* g)
    : BaseScreen(g, "CoreTest")
#if defined(__DAVAENGINE_COREV2__)
    , engine(g->GetEngine())
#endif
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
#if defined(__DAVAENGINE_COREV2__)
    engine->Quit();
#else
    Core::Instance()->Quit();
#endif
}
