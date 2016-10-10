#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/UWP/PlatformCoreUWP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Window.h"
#include "Engine/UWP/NativeServiceUWP.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Engine/Private/UWP/Window/WindowBackendUWP.h"

#include "Platform/SystemTimer.h"
#include "Concurrency/Thread.h"
#include "Logger/Logger.h"
#include "Utils/Utils.h"
#include "Platform/DeviceInfo.h"

extern int DAVAMain(DAVA::Vector<DAVA::String> cmdline);

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* engineBackend)
    : engineBackend(*engineBackend)
    , dispatcher(*engineBackend->GetDispatcher())
    , nativeService(new NativeService(this))
{
}

PlatformCore::~PlatformCore() = default;

void PlatformCore::Init()
{
}

void PlatformCore::Run()
{
    engineBackend.OnGameLoopStarted();

    while (!quitGameThread)
    {
        uint64 frameBeginTime = SystemTimer::Instance()->AbsoluteMS();

        int32 fps = engineBackend.OnFrame();

        uint64 frameEndTime = SystemTimer::Instance()->AbsoluteMS();
        uint32 frameDuration = static_cast<uint32>(frameEndTime - frameBeginTime);

        int32 sleep = 1;
        if (fps > 0)
        {
            sleep = 1000 / fps - frameDuration;
            if (sleep < 1)
                sleep = 1;
        }
        Sleep(sleep);
    }

    engineBackend.OnGameLoopStopped();
    engineBackend.OnEngineCleanup();
}

void PlatformCore::PrepareToQuit()
{
    engineBackend.PostAppTerminate(true);
}

void PlatformCore::Quit()
{
    quitGameThread = true;
}

void PlatformCore::OnLaunched()
{
    Logger::FrameworkDebug("========== CoreWinUWP::OnLaunched: thread=%d", GetCurrentThreadId());

    if (!gameThreadRunning)
    {
        Thread* gameThread = Thread::Create(MakeFunction(this, &PlatformCore::GameThread));
        gameThread->Start();
        gameThread->BindToProcessor(0);
        // TODO: make Thread detachable
        //gameThread->Detach();
        //gameThread->Release();

        gameThreadRunning = true;
    }
}

void PlatformCore::OnActivated()
{
    Logger::FrameworkDebug("========== CoreWinUWP::OnActivated: thread=%d", GetCurrentThreadId());
}

void PlatformCore::OnWindowCreated(::Windows::UI::Xaml::Window ^ xamlWindow)
{
    Logger::FrameworkDebug("========== CoreWinUWP::OnWindowCreated: thread=%d", GetCurrentThreadId());

    // TODO: think about binding XAML window to prior created Window instance
    Window* primaryWindow = engineBackend.GetPrimaryWindow();
    if (primaryWindow == nullptr)
    {
        primaryWindow = engineBackend.InitializePrimaryWindow();
    }
    WindowBackend* windowBackend = primaryWindow->GetBackend();
    windowBackend->BindXamlWindow(xamlWindow);
}

void PlatformCore::OnSuspending()
{
    Logger::FrameworkDebug("========== CoreWinUWP::OnSuspending: thread=%d", GetCurrentThreadId());

    MainDispatcherEvent e(MainDispatcherEvent::APP_SUSPENDED);
    dispatcher.SendEvent(e); // Blocking call !!!
}

void PlatformCore::OnResuming()
{
    Logger::FrameworkDebug("========== CoreWinUWP::OnResuming: thread=%d", GetCurrentThreadId());

    MainDispatcherEvent e(MainDispatcherEvent::APP_RESUMED);
    dispatcher.PostEvent(e);
}

void PlatformCore::OnUnhandledException(::Windows::UI::Xaml::UnhandledExceptionEventArgs ^ arg)
{
    Logger::Error("Unhandled exception: hresult=0x%08X, message=%s", arg->Exception, WStringToString(arg->Message->Data()).c_str());
}

void PlatformCore::GameThread()
{
    Logger::FrameworkDebug("========== PlatformCore::GameThread enter: thread=%d", GetCurrentThreadId());

    Vector<String> cmdline = engineBackend.GetCommandLine();
    DAVAMain(std::move(cmdline));

    // DAVA::Logger is already dead
    OutputDebugStringA("========== PlatformCore::GameThread leave\n");

    using namespace ::Windows::UI::Xaml;
    Application::Current->Exit();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
