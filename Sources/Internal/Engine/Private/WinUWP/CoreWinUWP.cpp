#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/WinUWP/CoreWinUWP.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/WinUWP/WindowWinUWP.h"

#include "Platform/SystemTimer.h"
#include "Concurrency/Thread.h"
#include "Concurrency/LockGuard.h"
#include "Logger/Logger.h"

extern int GameMain(DAVA::Vector<DAVA::String> cmdline);

namespace DAVA
{
namespace Private
{
CoreWinUWP::CoreWinUWP(EngineBackend* e)
    : engineBackend(e)
{
}

CoreWinUWP::~CoreWinUWP() = default;

Vector<String> CoreWinUWP::GetCommandLine(int /*argc*/, char* /*argv*/ [])
{
    return Vector<String>();
}

void CoreWinUWP::Init()
{
}

void CoreWinUWP::Run()
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

void CoreWinUWP::Quit()
{
    quitGameThread = true;
}

void CoreWinUWP::OnLaunched()
{
    Logger::Debug("****** CoreWinUWP::OnLaunched: thread=%d", GetCurrentThreadId());

    if (!gameThreadRunning)
    {
        Thread* gameThread = Thread::Create(MakeFunction(this, &CoreWinUWP::GameThread));
        gameThread->Start();
        gameThread->BindToProcessor(0);
        // TODO: make Thread detachable
        //gameThread->Detach();
        //gameThread->Release();

        gameThreadRunning = true;
    }
}

void CoreWinUWP::OnActivated()
{
    Logger::Debug("****** CoreWinUWP::OnActivated: thread=%d", GetCurrentThreadId());
}

void CoreWinUWP::OnWindowCreated(::Windows::UI::Xaml::Window ^ xamlWindow)
{
    Logger::Debug("****** CoreWinUWP::OnWindowCreated: thread=%d", GetCurrentThreadId());

    WindowWinUWP* nativeWindow = new WindowWinUWP(engineBackend, engineBackend->GetPrimaryWindow());
    nativeWindow->BindXamlWindow(xamlWindow);
}

void CoreWinUWP::OnSuspending()
{
    Logger::Debug("******** CoreWinUWP::OnSuspending: thread=%d", GetCurrentThreadId());
}

void CoreWinUWP::OnResuming()
{
    Logger::Debug("******** CoreWinUWP::OnResuming: thread=%d", GetCurrentThreadId());
}

void CoreWinUWP::GameThread()
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
