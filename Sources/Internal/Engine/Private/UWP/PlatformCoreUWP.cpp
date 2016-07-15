#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/UWP/PlatformCoreUWP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Public/UWP/NativeServiceUWP.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/UWP/Window/WindowBackendUWP.h"

#include "Platform/SystemTimer.h"
#include "Concurrency/Thread.h"
#include "Logger/Logger.h"

extern int GameMain(DAVA::Vector<DAVA::String> cmdline);

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* e)
    : engineBackend(e)
    , nativeService(new NativeService(this))
{
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
        Sleep(sleep);
    }

    engineBackend->OnGameLoopStopped();
    engineBackend->OnBeforeTerminate();

    using namespace ::Windows::UI::Xaml;
    Application::Current->Exit();
}

void PlatformCore::Quit()
{
    quitGameThread = true;
}

void PlatformCore::OnLaunched()
{
    Logger::Debug("****** CoreWinUWP::OnLaunched: thread=%d", GetCurrentThreadId());

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
    Logger::Debug("****** CoreWinUWP::OnActivated: thread=%d", GetCurrentThreadId());
}

void PlatformCore::OnWindowCreated(::Windows::UI::Xaml::Window ^ xamlWindow)
{
    Logger::Debug("****** CoreWinUWP::OnWindowCreated: thread=%d", GetCurrentThreadId());

    WindowBackend* nativeWindow = new WindowBackend(engineBackend, engineBackend->GetPrimaryWindow());
    nativeWindow->BindXamlWindow(xamlWindow);
}

void PlatformCore::OnSuspending()
{
    Logger::Debug("******** CoreWinUWP::OnSuspending: thread=%d", GetCurrentThreadId());
}

void PlatformCore::OnResuming()
{
    Logger::Debug("******** CoreWinUWP::OnResuming: thread=%d", GetCurrentThreadId());
}

void PlatformCore::GameThread()
{
    Logger::Debug("****** CoreWinUWP::GameThread enter: thread=%d", GetCurrentThreadId());

    Vector<String> cmdline;
    GameMain(cmdline);

    Logger::Debug("****** CoreWinUWP::GameThread leave: thread=%d", GetCurrentThreadId());
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
