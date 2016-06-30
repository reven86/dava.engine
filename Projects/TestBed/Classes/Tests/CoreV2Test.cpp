#if defined(__DAVAENGINE_COREV2__)

#include "Tests/CoreV2Test.h"
#include "Infrastructure/GameCore.h"

#include "Engine/Engine.h"
#include "Logger/Logger.h"

using namespace DAVA;

CoreV2Test::CoreV2Test(GameCore* g)
    : BaseScreen(g, "CoreV2Test")
    , engine(g->GetEngine())
{
    engine->windowCreated.Connect(MakeFunction(this, &CoreV2Test::OnWindowCreated));
    engine->windowDestroyed.Connect(MakeFunction(this, &CoreV2Test::OnWindowDestroyed));
}

CoreV2Test::~CoreV2Test() = default;

void CoreV2Test::LoadResources()
{
    BaseScreen::LoadResources();

    Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");
    font->SetSize(12);

    float32 h = 20.0f;
    float32 gap = 10.0f;
    float32 y = 10.0f;
    buttonQuit = CreateUIButton(font, Rect(10, y, 200, h), "Quit", &CoreV2Test::OnQuit);

    buttonResize640x480 = CreateUIButton(font, Rect(10, y += h + gap, 200, h), "Resize 640x480", &CoreV2Test::OnResize);
    buttonResize1024x768 = CreateUIButton(font, Rect(10, y += h + gap, 200, h), "Resize 1024x768", &CoreV2Test::OnResize);

    buttonRunOnMain = CreateUIButton(font, Rect(10, y += h + gap, 200, h), "Run on main", &CoreV2Test::OnRun);
    buttonRunOnUI = CreateUIButton(font, Rect(10, y += h + gap, 200, h), "Run on UI", &CoreV2Test::OnRun);
}

void CoreV2Test::UnloadResources()
{
    engine->windowCreated.Disconnect(tokenOnWindowCreated);
    engine->windowDestroyed.Disconnect(tokenOnWindowDestroyed);

    SafeRelease(buttonQuit);
    SafeRelease(buttonResize640x480);
    SafeRelease(buttonResize1024x768);
    SafeRelease(buttonRunOnMain);
    SafeRelease(buttonRunOnUI);

    BaseScreen::UnloadResources();
}

void CoreV2Test::OnQuit(DAVA::BaseObject* obj, void* data, void* callerData)
{
    engine->Quit(4);
}

void CoreV2Test::OnResize(DAVA::BaseObject* obj, void* data, void* callerData)
{
    float32 w = 0.0f;
    float32 h = 0.0f;
    if (obj == buttonResize640x480)
    {
        w = 640.0f;
        h = 480.0f;
    }
    else if (obj == buttonResize1024x768)
    {
        w = 1024.0f;
        h = 768.0f;
    }
    engine->PrimaryWindow()->Resize(w, h);
}

void CoreV2Test::OnRun(DAVA::BaseObject* obj, void* data, void* callerData)
{
    if (obj == buttonRunOnMain)
    {
        engine->RunAsyncOnMainThread([]() {
            Logger::Error("******** KABOOM on main thread********");
        });
    }
    else if (obj == buttonRunOnUI)
    {
        engine->PrimaryWindow()->RunAsyncOnUIThread([]() {
            Logger::Error("******** KABOOM on UI thread********");
        });
    }
}

void CoreV2Test::OnWindowCreated(DAVA::Window* w)
{
    Logger::Debug("****** CoreV2Test::OnWindowCreated");
}

void CoreV2Test::OnWindowDestroyed(DAVA::Window* w)
{
    Logger::Debug("****** CoreV2Test::OnWindowDestroyed");
}

DAVA::UIButton* CoreV2Test::CreateUIButton(DAVA::Font* font, const DAVA::Rect& rect, const DAVA::String& text,
                                           void (CoreV2Test::*onClick)(DAVA::BaseObject*, void*, void*))
{
    using namespace DAVA;

    UIButton* button = new UIButton(rect);
    button->SetStateFont(0xFF, font);
    button->SetStateText(0xFF, StringToWString(text));
    button->SetStateFontColor(0xFF, Color::White);
    button->SetDebugDraw(true);
    button->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, onClick));
    AddControl(button);
    return button;
}

#endif // __DAVAENGINE_COREV2__
