#include "Engine/Private/Win32/PlatformCoreWin32.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include <shellapi.h>
#include <timeapi.h>

#include "Engine/Window.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Win32/DllImportWin32.h"
#include "Engine/Private/Win32/Window/WindowBackendWin32.h"

#include "Logger/Logger.h"
#include "Time/SystemTimer.h"
#include "Utils/Utils.h"

namespace DAVA
{
namespace Private
{
HINSTANCE PlatformCore::hinstance = nullptr;

PlatformCore::PlatformCore(EngineBackend* engineBackend)
    : engineBackend(*engineBackend)
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
// Ops, steam knows nothing about new pointer input, burn it with fire
#if !defined(__DAVAENGINE_STEAM__)
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
#endif

    // TODO: temporal hardcode, separate task for setting rotation

    // Auto-rotation preferences are supported starting from Windows 8.
    // Also system sets auto-rotation preferences only for calling process and these preferences are applied only
    // when application's window has focus.
    if (DllImport::fnGetAutoRotationState != nullptr)
    {
        AR_STATE arstate{};
        BOOL result = DllImport::fnGetAutoRotationState(&arstate);
        if (result && (arstate & (AR_NOSENSOR | AR_NOT_SUPPORTED)) == 0)
        {
            DllImport::fnSetDisplayAutoRotationPreferences(ORIENTATION_PREFERENCE_LANDSCAPE | ORIENTATION_PREFERENCE_LANDSCAPE_FLIPPED);
        }
    }

    engineBackend.InitializePrimaryWindow();
}

void PlatformCore::Run()
{
    MSG msg;
    bool quitLoop = false;

    engineBackend.OnGameLoopStarted();

    WindowBackend* primaryWindowBackend = EngineBackend::GetWindowBackend(engineBackend.GetPrimaryWindow());
    primaryWindowBackend->Create(1024.0f, 768.0f);

    for (;;)
    {
        int64 frameBeginTime = SystemTimer::GetMs();

        while (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            quitLoop = WM_QUIT == msg.message;
            if (quitLoop)
                break;

            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        int32 fps = engineBackend.OnFrame();
        int64 frameEndTime = SystemTimer::GetMs();
        int32 frameDuration = static_cast<int32>(frameEndTime - frameBeginTime);

        int32 sleep = 1;
        if (fps > 0)
        {
            sleep = 1000 / fps - frameDuration;
            if (sleep < 1)
                sleep = 1;
        }
        ::Sleep(sleep);

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

void PlatformCore::EnableHighResolutionTimer(bool enable)
{
    static UINT minTimerPeriod = 0;
    static bool highResolutionEnabled = false;

    if (minTimerPeriod == 0)
    {
        // On first call obtain timer capabilities
        TIMECAPS timeCaps;
        if (::timeGetDevCaps(&timeCaps, sizeof(TIMECAPS)) == TIMERR_NOERROR)
        {
            minTimerPeriod = timeCaps.wPeriodMin;
        }
    }

    // Application must match each call to timeBeginPeriod with a call to timeEndPeriod
    // https://msdn.microsoft.com/en-us/library/dd757633(v=vs.85).aspx
    if (minTimerPeriod != 0 && highResolutionEnabled != enable)
    {
        if (enable)
        {
            ::timeBeginPeriod(minTimerPeriod);
        }
        else
        {
            timeEndPeriod(minTimerPeriod);
        }
        highResolutionEnabled = enable;
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
