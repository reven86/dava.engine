#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/EngineBackend.h"

#if defined(__DAVAENGINE_WIN32__)

#include <shellapi.h>

#include "Engine/Private/Win32/CoreWin32.h"
#include "Engine/Private/Win32/WindowWin32.h"

#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"

namespace DAVA
{
namespace Private
{
HINSTANCE CoreWin32::hinstance = nullptr;

CoreWin32::CoreWin32()
{
    hinstance = reinterpret_cast<HINSTANCE>(::GetModuleHandleW(nullptr));
}

CoreWin32::~CoreWin32() = default;

Vector<String> CoreWin32::GetCommandLine(int argc, char* argv[])
{
    Vector<String> cmdargs;

    int nargs = 0;
    LPWSTR cmdline = GetCommandLineW();
    LPWSTR* arglist = CommandLineToArgvW(cmdline, &nargs);
    if (arglist != nullptr)
    {
        cmdargs.reserve(nargs);
        for (int i = 0; i < nargs; ++i)
        {
            cmdargs.push_back(WStringToString(arglist[i]));
        }
        LocalFree(arglist);
    }
    return cmdargs;
}

void CoreWin32::Init(bool consoleMode_)
{
    consoleMode = consoleMode_;
}

void CoreWin32::Run()
{
    if (consoleMode)
        RunConsole();
    else
        RunGUI();
}

void CoreWin32::Quit()
{
    if (consoleMode)
        quitConsole = true;
    else
        ::PostQuitMessage(0);
}

void CoreWin32::RunGUI()
{
    MSG msg;
    bool quitLoop = false;

    EngineBackend::instance->OnGameLoopStarted();
    for (;;)
    {
        uint64 frameBeginTime = SystemTimer::Instance()->AbsoluteMS();

        while (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            quitLoop = WM_QUIT == msg.message;
            if (quitLoop)
                break;

            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        int32 fps = EngineBackend::instance->OnFrame();
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

        if (quitLoop)
            break;
    }
    EngineBackend::instance->OnGameLoopStopped();
}

void CoreWin32::RunConsole()
{
    EngineBackend::instance->OnGameLoopStarted();
    while (!quitConsole)
    {
        EngineBackend::instance->OnFrameConsole();
        Sleep(1);
    }
    EngineBackend::instance->OnGameLoopStopped();
}

WindowWin32* CoreWin32::CreateNativeWindow(Window* w)
{
    return WindowWin32::Create(w);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
