#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Infrastructure/BaseScreen.h"

#include "Engine/EngineFwd.h"
#include "Engine/Public/Dispatcher.h"

namespace DAVA
{
class UIButton;
class Font;
struct Rect;
class BaseObject;
class Thread;
};

class GameCore;
class CoreV2Test : public BaseScreen
{
public:
    CoreV2Test(GameCore* g);
    ~CoreV2Test();

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    void OnQuit(DAVA::BaseObject* obj, void* data, void* callerData);
    void OnResize(DAVA::BaseObject* obj, void* data, void* callerData);
    void OnRun(DAVA::BaseObject* obj, void* data, void* callerData);
    void OnDispatcherTest(DAVA::BaseObject* obj, void* data, void* callerData);

    void OnWindowCreated(DAVA::Window& w);
    void OnWindowDestroyed(DAVA::Window& w);

    DAVA::UIButton* CreateUIButton(DAVA::Font* font, const DAVA::Rect& rect, const DAVA::String& text,
                                   void (CoreV2Test::*onClick)(DAVA::BaseObject*, void*, void*));

private:
    DAVA::Engine* engine = nullptr;

    DAVA::UIButton* buttonQuit = nullptr;

    DAVA::UIButton* buttonResize640x480 = nullptr;
    DAVA::UIButton* buttonResize1024x768 = nullptr;

    DAVA::UIButton* buttonRunOnMain = nullptr;
    DAVA::UIButton* buttonRunOnUI = nullptr;

    DAVA::UIButton* buttonDispTrigger1 = nullptr;
    DAVA::UIButton* buttonDispTrigger2 = nullptr;
    DAVA::UIButton* buttonDispTrigger3 = nullptr;
    DAVA::UIButton* buttonDispTrigger1000 = nullptr;
    DAVA::UIButton* buttonDispTrigger2000 = nullptr;
    DAVA::UIButton* buttonDispTrigger3000 = nullptr;

    size_t tokenOnWindowCreated = 0;
    size_t tokenOnWindowDestroyed = 0;

    //////////////////////////////////////////////////////////////////////////
    using TestDispatcher = DAVA::Dispatcher<int>;

    DAVA::Vector<DAVA::RefPtr<DAVA::Thread>> dispatcherThreads;
    DAVA::Vector<std::unique_ptr<TestDispatcher>> dispatchers;
    bool stopDispatchers = false;

    void DispatcherThread(TestDispatcher* dispatcher, int index);
    void DispatcherEventHandler(int type);
};

#endif // __DAVAENGINE_COREV2__
