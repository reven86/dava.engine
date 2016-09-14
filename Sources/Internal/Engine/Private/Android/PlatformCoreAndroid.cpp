#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Android/PlatformCoreAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)

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
    , dispatcher(engineBackend->GetDispatcher())
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
    engineBackend->OnBeforeTerminate();
}

void PlatformCore::Quit()
{
    quitGameThread = true;
}

WindowBackend* PlatformCore::ActivityOnCreate()
{
    Logger::FrameworkDebug("=========== PlatformCore::ActivityOnCreate");

    WindowBackend* primaryWindowBackend = new WindowBackend(engineBackend, engineBackend->GetPrimaryWindow());
    return primaryWindowBackend;
}

void PlatformCore::ActivityOnResume()
{
    Logger::FrameworkDebug("=========== PlatformCore::ActivityOnResume");

    MainDispatcherEvent e(MainDispatcherEvent::APP_RESUMED);
    dispatcher->PostEvent(e);
}

void PlatformCore::ActivityOnPause()
{
    Logger::FrameworkDebug("=========== PlatformCore::ActivityOnPause");

    MainDispatcherEvent e(MainDispatcherEvent::APP_SUSPENDED);
    dispatcher->SendEvent(e); // Blocking call !!!
}

void PlatformCore::ActivityOnDestroy()
{
    Logger::FrameworkDebug("=========== PlatformCore::ActivityOnDestroy");

    // Do nonblocking call as Java part will wait until native thread is finished
    MainDispatcherEvent e(MainDispatcherEvent::APP_IMMEDIATE_TERMINATE);
    dispatcher->PostEvent(e);
}

void PlatformCore::GameThread()
{
    Logger::FrameworkDebug("=========== PlatformCore::GameThread: enter");

    Vector<String> cmdline;
    GameMain(std::move(cmdline));

    Logger::FrameworkDebug("=========== PlatformCore::GameThread: leave");
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
