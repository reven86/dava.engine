#include "Platform/DPIHelper.h"
#include "ShellScalingAPI.h"

//temporary decision
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#include "Platform/TemplateWin32/CorePlatformWin32.h"

namespace DAVA
{

#if defined(__DAVAENGINE_WIN32__)

uint32 DPIHelper::GetScreenDPI()
{
    uint32 hDPI = 0;

    using MonitorDpiFn = HRESULT(WINAPI*)(_In_ HMONITOR, _In_ MONITOR_DPI_TYPE, _Out_ UINT*, _Out_ UINT*);

    // we are trying to get pointer on GetDpiForMonitor function with GetProcAddress
    // because this function is available only on win8.1 and win10 but we should be able
    // to run the same build on win7, win8, win10. So on win7 GetProcAddress will return null
    // and GetDpiForMonitor wont be called
    HMODULE module = GetModuleHandle(TEXT("shcore.dll"));
    MonitorDpiFn fn = reinterpret_cast<MonitorDpiFn>(GetProcAddress(module, "GetDpiForMonitor"));

#if !defined(__DAVAENGINE_COREV2__)
    void* nativeWindow = Core::Instance()->GetNativeWindow();
#else
    void* nativeWindow = Engine::Instance()->PrimaryWindow()->GetNativeHandle();
#endif
    if (nullptr != fn && nullptr != nativeWindow)
    {
        HWND hwnd = static_cast<HWND>(nativeWindow);

        UINT x = 0, y = 0;
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        (*fn)(monitor, MDT_EFFECTIVE_DPI, &x, &y);

        hDPI = x;
    }
    else
    {
        // default behavior for windows (ver < 8.1)
        // get dpi from caps
        HDC screen = GetDC(NULL);
        hDPI = GetDeviceCaps(screen, LOGPIXELSX);
        ReleaseDC(NULL, screen);
    }

    return hDPI;
}

float64 DPIHelper::GetDpiScaleFactor(int32 screenId)
{
    return 1.0;
}

Size2i DPIHelper::GetScreenSize()
{
    Size2i screenSize;
    HDC screen = GetDC(NULL);
    screenSize.dx = GetDeviceCaps(screen, HORZRES);
    screenSize.dy = GetDeviceCaps(screen, VERTRES);
    ReleaseDC(NULL, screen);
    return screenSize;
}

#elif defined(__DAVAENGINE_WIN_UAP__)

uint32 DPIHelper::GetScreenDPI()
{
#if defined(__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__) && !defined(__DAVAENGINE_COREV2__)
    CorePlatformWinUAP* core = static_cast<CorePlatformWinUAP*>(Core::Instance());
    uint32 d(0);
    auto func = [&d]()
    {
        using ::Windows::Graphics::Display::DisplayInformation;
        d = uint32(DisplayInformation::GetForCurrentView()->RawDpiX);
        Logger::FrameworkDebug("[DPIHelper] GetScreenDPI = %d", d);
    };
    core->RunOnUIThreadBlocked(func);
    return d;
#else
    return 0;
#endif //  (__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__)
}

float64 DPIHelper::GetDpiScaleFactor(int32 /*screenId*/)
{
#if defined(__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__) && !defined(__DAVAENGINE_COREV2__)
    float64 scaleFactor = 0.0;
    CorePlatformWinUAP* core = static_cast<CorePlatformWinUAP*>(Core::Instance());
    core->RunOnUIThreadBlocked([&scaleFactor]()
                               {
                                   using Windows::Graphics::Display::DisplayInformation;
                                   DisplayInformation ^ displayInfo = DisplayInformation::GetForCurrentView();
                                   scaleFactor = displayInfo->RawPixelsPerViewPixel;
                               });
    return scaleFactor;
#else
    return 0;
#endif //  (__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__)
}

Size2i DPIHelper::GetScreenSize()
{
#if defined(__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__) && !defined(__DAVAENGINE_COREV2__)
    CorePlatformWinUAP* core = static_cast<CorePlatformWinUAP*>(Core::Instance());
    uint32 w(0), h(0);
    auto func = [&w, &h]()
    {
        auto winBounds = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Bounds;
        w = static_cast<uint32>(winBounds.Width);
        h = static_cast<uint32>(winBounds.Height);
    };
    core->RunOnUIThreadBlocked(func);
    return Size2i(w, h);
#else
    return Size2i(0, 0);
#endif //  (__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__)
}

#endif

} // namespace DAVA
