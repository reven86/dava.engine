#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Android/PlatformCoreAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Public/Android/NativeServiceAndroid.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Engine/Private/Android/AndroidBridge.h"
#include "Engine/Private/Android/Window/WindowBackendAndroid.h"

#include "Platform/SystemTimer.h"
#include "Concurrency/Thread.h"
#include "Logger/Logger.h"

extern DAVA::Private::AndroidBridge* androidBridge;
extern int GameMain(DAVA::Vector<DAVA::String> cmdline);

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* ebackend)
    : engineBackend(ebackend)
    , dispatcher(engineBackend->GetDispatcher())
    , bridge(androidBridge)
    , nativeService(new NativeService(this))
{
    androidBridge->core = this;
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

void PlatformCore::GameThread()
{
    Logger::Info("******** PlatformCore::GameThread: enter");

    Vector<String> cmdline;
    GameMain(std::move(cmdline));

    Logger::Info("******** PlatformCore::GameThread: leave");
}

WindowBackend* PlatformCore::OnCreate()
{
    Logger::Info("******** PlatformCore::OnCreate: thread=%llX", Thread::GetCurrentIdAsInteger());

    WindowBackend* primaryWindowBackend = new WindowBackend(engineBackend, engineBackend->GetPrimaryWindow());
    return primaryWindowBackend;
}

void PlatformCore::OnStart()
{
    Logger::Info("******** PlatformCore::OnStart: thread=%llX", Thread::GetCurrentIdAsInteger());
}

void PlatformCore::OnResume()
{
    Logger::Info("******** PlatformCore::OnResume: thread=%llX", Thread::GetCurrentIdAsInteger());

    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::APP_RESUMED;
    dispatcher->PostEvent(e);
}

void PlatformCore::OnPause()
{
    Logger::Info("******** PlatformCore::OnPause: thread=%llX", Thread::GetCurrentIdAsInteger());

    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::APP_SUSPENDED;
    dispatcher->SendEvent(e); // Blocking call !!!
}

void PlatformCore::OnStop()
{
    Logger::Info("******** PlatformCore::OnStop: thread=%llX", Thread::GetCurrentIdAsInteger());
}

void PlatformCore::OnDestroy()
{
    Logger::Info("******** PlatformCore::OnDestroy: thread=%llX", Thread::GetCurrentIdAsInteger());

    MainDispatcherEvent e;
    e.window = nullptr;
    e.type = MainDispatcherEvent::APP_IMMEDIATE_TERMINATE;
    dispatcher->PostEvent(e);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
