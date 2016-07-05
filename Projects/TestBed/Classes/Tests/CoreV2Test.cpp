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
    for (int i = 0; i < 4; ++i)
    {
        dispatchers.push_back(std::unique_ptr<TestDispatcher>(new TestDispatcher(MakeFunction(this, &CoreV2Test::DispatcherEventHandler))));
        dispatcherThreads.emplace_back(Thread::Create([this, i]() {
            DispatcherThread(dispatchers[i].get(), i);
        }));
        dispatcherThreads.back()->Start();
    }

    engine->windowCreated.Connect(MakeFunction(this, &CoreV2Test::OnWindowCreated));
    engine->windowDestroyed.Connect(MakeFunction(this, &CoreV2Test::OnWindowDestroyed));
}

CoreV2Test::~CoreV2Test()
{
    stopDispatchers = true;
}

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

    y = 10.0f;
    buttonDispTrigger1 = CreateUIButton(font, Rect(250, y, 200, h), "Trigger 1", &CoreV2Test::OnDispatcherTest);
    buttonDispTrigger2 = CreateUIButton(font, Rect(250, y += h + gap, 200, h), "Trigger 2", &CoreV2Test::OnDispatcherTest);
    buttonDispTrigger3 = CreateUIButton(font, Rect(250, y += h + gap, 200, h), "Trigger 3", &CoreV2Test::OnDispatcherTest);
    buttonDispTrigger1000 = CreateUIButton(font, Rect(250, y += h + gap, 200, h), "Trigger 1000", &CoreV2Test::OnDispatcherTest);
    buttonDispTrigger2000 = CreateUIButton(font, Rect(250, y += h + gap, 200, h), "Trigger 2000", &CoreV2Test::OnDispatcherTest);
    buttonDispTrigger3000 = CreateUIButton(font, Rect(250, y += h + gap, 200, h), "Trigger 3000", &CoreV2Test::OnDispatcherTest);
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

    SafeRelease(buttonDispTrigger1);
    SafeRelease(buttonDispTrigger2);
    SafeRelease(buttonDispTrigger3);
    SafeRelease(buttonDispTrigger1000);
    SafeRelease(buttonDispTrigger2000);
    SafeRelease(buttonDispTrigger3000);

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
    using DAVA::Private::DispatcherEvent;

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

void CoreV2Test::OnDispatcherTest(DAVA::BaseObject* obj, void* data, void* callerData)
{
    TestDispatcher* disp = dispatchers[0].get();
    if (obj == buttonDispTrigger1)
    {
        disp->PostEvent(1);
    }
    else if (obj == buttonDispTrigger2)
    {
        disp->PostEvent(2);
    }
    else if (obj == buttonDispTrigger3)
    {
        disp->PostEvent(3);
    }
    else if (obj == buttonDispTrigger1000)
    {
        disp->PostEvent(1000);
    }
    else if (obj == buttonDispTrigger2000)
    {
        disp->PostEvent(2000);
    }
    else if (obj == buttonDispTrigger3000)
    {
        disp->PostEvent(3000);
    }
}

void CoreV2Test::DispatcherThread(TestDispatcher* dispatcher, int index)
{
    dispatcher->LinkToCurrentThread();
    Logger::Debug("###### CoreV2Test::DispatcherThread enter: thread=%llu, index=%d", dispatcher->GetLinkedThread(), index);
    while (!stopDispatchers)
    {
        dispatcher->ProcessEvents();
        Sleep(100);
    }
    Logger::Debug("###### CoreV2Test::DispatcherThread leave: thread=%llu, index=%d", dispatcher->GetLinkedThread(), index);
}

void CoreV2Test::DispatcherEventHandler(int type)
{
    Logger::Debug("###### CoreV2Test::EventHandler: thread=%llu, type=%d", Thread::GetCurrentIdAsInteger(), type);
    if (type == 1 || type == 2 || type == 3)
    {
        // 1: post to even dispatchers
        // 2: post to odd dispatchers
        // 3: broadcast
        for (size_t i = 0, n = dispatchers.size(); i < n; ++i)
        {
            if ((type == 1 && (i & 1) == 0) || (type == 2 && (i & 1) == 1) || type == 3)
            {
                dispatchers[i]->PostEvent(4);
            }
        }
    }
    else if (type >= 3000)
    {
        // chained send event with deadlock
        size_t index = type - 3000;
        if (index < dispatchers.size())
        {
            dispatchers[index]->SendEvent(type + 1);
        }
        else
        {
            dispatchers[0]->SendEvent(10000);
        }
    }
    else if (type >= 2000)
    {
        // chained send event
        size_t index = type - 2000;
        if (index < dispatchers.size())
        {
            dispatchers[index]->SendEvent(type + 1);
        }
    }
    else if (type >= 1000)
    {
        // chained post event
        size_t index = type - 1000;
        if (index < dispatchers.size())
        {
            dispatchers[index]->PostEvent(type + 1);
        }
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
