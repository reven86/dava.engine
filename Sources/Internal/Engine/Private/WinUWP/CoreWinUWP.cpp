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
#include "Logger/Logger.h"

extern int GameMain(DAVA::Vector<DAVA::String> cmdline);

namespace DAVA
{
namespace Private
{
// clang-format off

CoreWinUWP::CoreWinUWP(EngineBackend* e)
    : engineBackend(e)
{

}

CoreWinUWP::~CoreWinUWP()
{
    SafeRelease(gameThread);
}

Vector<String> CoreWinUWP::GetCommandLine(int /*argc*/, char* /*argv*/[])
{
    return Vector<String>();
}

void CoreWinUWP::Init()
{
}

void CoreWinUWP::Run()
{
    engineBackend->OnGameLoopStarted();

    while(!quitGameThread)
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
}

void CoreWinUWP::Quit()
{
}

WindowWinUWP* CoreWinUWP::CreateNativeWindow(WindowBackend* w, float32 width, float32 height)
{
    return nullptr;
}

void CoreWinUWP::DestroyNativeWindow(WindowBackend* w)
{
    w->GetNativeWindow()->DestroyNWindow();
}

void CoreWinUWP::OnApplicationLaunched()
{
    Logger::Debug("****** CoreWinUWP::OnApplicationLaunched: thread=%d", GetCurrentThreadId());

    if (gameThread == nullptr)
    {
        gameThread = Thread::Create(MakeFunction(this, &CoreWinUWP::GameThread));
        gameThread->Start();
        gameThread->BindToProcessor(0);
    }
}

void CoreWinUWP::OnNativeWindowCreated(::Windows::UI::Xaml::Window^ xamlWindow)
{
    Logger::Debug("****** CoreWinUWP::OnNativeWindowCreated: thread=%d", GetCurrentThreadId());

    bool isPrimary = engineBackend->primaryWindow == nullptr;

    WindowBackend* w = new WindowBackend(engineBackend, isPrimary);
    engineBackend->windows.insert(w);

    WindowWinUWP* nativeW = new WindowWinUWP(engineBackend, w);
    nativeW->BindXamlWindow(xamlWindow);

    if (isPrimary)
    {
        engineBackend->primaryWindow = w;
    }
}

void CoreWinUWP::OnSuspending()
{

}

void CoreWinUWP::OnResuming()
{

}

void CoreWinUWP::GameThread()
{
    Logger::Debug("****** CoreWinUWP::GameThread enter: thread=%d", GetCurrentThreadId());

    Vector<String> cmdline;
    GameMain(cmdline);

    Logger::Debug("****** CoreWinUWP::GameThread leave: thread=%d", GetCurrentThreadId());
}

// clang-format on

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
