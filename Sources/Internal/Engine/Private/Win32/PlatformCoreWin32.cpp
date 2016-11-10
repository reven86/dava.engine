#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Win32/PlatformCoreWin32.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include <shellapi.h>

#include "Engine/Window.h"
#include "Engine/Win32/NativeServiceWin32.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Win32/DllImportWin32.h"
#include "Engine/Private/Win32/Window/WindowBackendWin32.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"

namespace DAVA
{
namespace Private
{
HINSTANCE PlatformCore::hinstance = nullptr;

PlatformCore::PlatformCore(EngineBackend* engineBackend)
    : engineBackend(*engineBackend)
    , nativeService(new NativeService(this))
{
    DllImport::Initialize();

    // Enable per monitor dpi awareness if by some reason it has not been set in manifest file
    if (DllImport::fnGetProcessDpiAwareness != nullptr)
    {
        PROCESS_DPI_AWARENESS dpiAwareLevel;
        HRESULT hr = DllImport::fnGetProcessDpiAwareness(nullptr, &dpiAwareLevel);
        if (hr == S_OK && dpiAwareLevel != PROCESS_PER_MONITOR_DPI_AWARE)
        {
            DllImport::fnSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
        }
    }
    hinstance = reinterpret_cast<HINSTANCE>(::GetModuleHandleW(nullptr));
}

PlatformCore::~PlatformCore() = default;

void PlatformCore::Init()
{
    // Check whether new pointer input is enabled and enable it if so
    if (DllImport::fnEnableMouseInPointer != nullptr)
    {
        // EnableMouseInPointer should be called only once in process lifetime. All desktop applications
        // by deafult start with mouse-in-pointer disabled.
        if (!DllImport::fnEnableMouseInPointer(TRUE))
        {
            Logger::Warning("Failed to enable new pointer input");
        }
    }

    engineBackend.InitializePrimaryWindow();
}

void PlatformCore::Run()
{
    MSG msg;
    bool quitLoop = false;

    engineBackend.OnGameLoopStarted();

    WindowBackend* primaryWindowBackend = engineBackend.GetPrimaryWindow()->GetBackend();
    primaryWindowBackend->Create(640.0f, 480.0f);

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

        if (quitLoop)
            break;
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
    ::PostQuitMessage(engineBackend.GetExitCode());
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
