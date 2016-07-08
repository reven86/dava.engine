#include "Platform/DPIHelper.h"

//temporary decision
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"

namespace DAVA
{

#if defined(__DAVAENGINE_WIN32__)

uint32 DPIHelper::GetScreenDPI()
{
    HDC screen = GetDC(NULL);

    // in common dpi is the same in horizontal and vertical demensions
    // in any case under win this value is 96dpi due to OS limitation
    uint32 hDPI = GetDeviceCaps(screen, LOGPIXELSX);
    ReleaseDC(NULL, screen);

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
#if defined(__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__)
    CorePlatformWinUAP* core = static_cast<CorePlatformWinUAP*>(Core::Instance());
    uint32 d(0);
    auto func = [&d]()
    {
        using namespace Windows::Graphics::Display;
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
#if defined(__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__)
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
#if defined(__DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__)
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
