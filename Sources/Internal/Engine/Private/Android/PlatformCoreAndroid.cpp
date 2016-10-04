#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Android/PlatformCoreAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Window.h"
#include "Engine/Android/NativeServiceAndroid.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Engine/Private/Android/AndroidBridge.h"
#include "Engine/Private/Android/Window/WindowBackendAndroid.h"

#include "Platform/SystemTimer.h"
#include "Logger/Logger.h"

extern int GameMain(DAVA::Vector<DAVA::String> cmdline);

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* engineBackend)
    : engineBackend(engineBackend)
    , mainDispatcher(engineBackend->GetDispatcher())
    , nativeService(new NativeService(this))
{
    AndroidBridge::AttachPlatformCore(this);
}

PlatformCore::~PlatformCore() = default;

void PlatformCore::Init()
{
}

void PlatformCore::Run()
{
    engineBackend->OnGameLoopStarted();

    while (!quitGameThread)
    {
        uint64 frameBeginTime = SystemTimer::Instance()->AbsoluteMS();

        int32 fps = engineBackend->OnFrame();

        uint64 frameEndTime = SystemTimer::Instance()->AbsoluteMS();
        uint32 frameDuration = static_cast<uint32>(frameEndTime - frameBeginTime);

        int32 sleep = 1;
        if (fps > 0)
        {
            sleep = 1000 / fps - frameDuration;
            if (sleep < 1)
                sleep = 1;
        }
        Thread::Sleep(sleep);
    }

    engineBackend->OnGameLoopStopped();
    engineBackend->OnEngineCleanup();
}

void PlatformCore::PrepareToQuit()
{
    AndroidBridge::PostQuitToActivity();
}

void PlatformCore::Quit()
{
    quitGameThread = true;
}

WindowBackend* PlatformCore::ActivityOnCreate()
{
    Logger::FrameworkDebug("=========== PlatformCore::ActivityOnCreate");

    Window* primaryWindow = engineBackend->InitializePrimaryWindow();
    WindowBackend* primaryWindowBackend = primaryWindow->GetBackend();
    return primaryWindowBackend;
}

void PlatformCore::ActivityOnResume()
{
    Logger::FrameworkDebug("=========== PlatformCore::ActivityOnResume");

    mainDispatcher->PostEvent(MainDispatcherEvent(MainDispatcherEvent::APP_RESUMED));
}

void PlatformCore::ActivityOnPause()
{
    Logger::FrameworkDebug("=========== PlatformCore::ActivityOnPause");

    // Blocking call !!!
    mainDispatcher->SendEvent(MainDispatcherEvent(MainDispatcherEvent::APP_SUSPENDED));
}

void PlatformCore::ActivityOnDestroy()
{
    Logger::FrameworkDebug("=========== PlatformCore::ActivityOnDestroy");

    // Dispatch application termination request initiated by system, i.e. android activity is finishing
    // Do nonblocking call as Java part will wait until native thread is finished
    engineBackend->PostAppTerminate(true);
}

void PlatformCore::GameThread()
{
    Logger::FrameworkDebug("=========== PlatformCore::GameThread: enter");

    Vector<String> cmdline;
    GameMain(std::move(cmdline));

    // Logger already is dead
    ANDROID_LOG_DEBUG("=========== PlatformCore::GameThread: leave");
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
