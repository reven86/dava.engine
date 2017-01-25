#include "Base/Platform.h"
#include "Logger/Logger.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Platform/TemplateWin32/MouseDeviceWin32.h"
#include "Platform/TemplateWin32/CorePlatformWin32.h"

#if !defined(__DAVAENGINE_COREV2__)

namespace DAVA
{
void MouseDeviceWin32::SetCursorInCenter()
{
#if defined(__DAVAENGINE_COREV2__)
    HWND hWnd = static_cast<HWND>(Engine::Instance()->PrimaryWindow()->GetNativeHandle());
#else
    HWND hWnd = static_cast<HWND>(DAVA::Core::Instance()->GetNativeView());
#endif
    RECT wndRect;
    ::GetWindowRect(hWnd, &wndRect);
    int centerX = static_cast<int>((wndRect.left + wndRect.right) >> 1);
    int centerY = static_cast<int>((wndRect.bottom + wndRect.top) >> 1);
    ::SetCursorPos(centerX, centerY);
    ::SetCursor(NULL);
}

bool MouseDeviceWin32::SkipEvents(const UIEvent* event)
{
    return false;
}

bool MouseDeviceWin32::SetSystemCursorVisibility(bool show)
{
#if defined(__DAVAENGINE_COREV2__)
    HWND wnd = static_cast<HWND>(Engine::Instance()->PrimaryWindow()->GetNativeHandle());
#else
    HWND wnd = static_cast<HWND>(Core::Instance()->GetNativeView());
#endif
    if (show)
    {
        HCURSOR defaultCursor = LoadCursor(NULL, IDC_ARROW);
        SetClassLongPtr(wnd, GCLP_HCURSOR, static_cast<LONG>(reinterpret_cast<LONG_PTR>(defaultCursor)));
        ::SetCursor(defaultCursor);
    }
    else
    {
        SetClassLongPtr(wnd, GCLP_HCURSOR, NULL);
        ::SetCursor(NULL);
    }
    return true;
}

void MouseDeviceWin32::SetMode(eCaptureMode newMode)
{
    switch (newMode)
    {
    case eCaptureMode::OFF:
    case eCaptureMode::PINING:
    {
        SetSystemCursorVisibility(newMode != eCaptureMode::PINING);
        if (newMode == eCaptureMode::PINING)
        {
            POINT p;
            ::GetCursorPos(&p);
            lastCursorPosition.x = p.x;
            lastCursorPosition.y = p.y;

            SetCursorInCenter();
        }
        else
        {
            ::SetCursorPos(lastCursorPosition.x, lastCursorPosition.y);
        }
        break;
    }
    case eCaptureMode::FRAME:
        Logger::Error("Unsupported cursor capture mode");
        break;
    default:
        DVASSERT(false, "Incorrect cursor capture mode");
        Logger::Error("Incorrect cursor capture mode");
        break;
    }
}

} //  namespace DAVA

#endif //!defined(__DAVAENGINE_COREV2__)

#endif //  __DAVAENGINE_WIN32__
