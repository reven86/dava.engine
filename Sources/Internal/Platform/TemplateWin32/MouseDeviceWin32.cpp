#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Platform/TemplateWin32/MouseDeviceWin32.h"
#include "Platform/TemplateWin32/CorePlatformWin32.h"

#include "Engine/Engine.h"

namespace DAVA
{
void MouseDeviceWin32::SetCursorInCenter()
{
#if defined(__DAVAENGINE_COREV2__)
    HWND hWnd = static_cast<HWND>(Engine::Instance()->PrimaryWindow()->NativeHandle());
#else
    HWND hWnd = static_cast<HWND>(DAVA::Core::Instance()->GetNativeView());
#endif
    RECT wndRect;
    GetWindowRect(hWnd, &wndRect);
    int centerX = static_cast<int>((wndRect.left + wndRect.right) >> 1);
    int centerY = static_cast<int>((wndRect.bottom + wndRect.top) >> 1);
    SetCursorPos(centerX, centerY);
}

bool MouseDeviceWin32::SkipEvents(const UIEvent* event)
{
    return false;
}

bool MouseDeviceWin32::SetSystemCursorVisibility(bool show)
{
    DAVA::int32 showCount = 0;
    showCount = ShowCursor(show); // No cursor info available, just call

    if (show && showCount >= 0)
    {
        // If system cursor is visible then showCount should be >= 0
        lastSystemCursorShowState = true;
    }
    else if (!show && showCount < 0)
    {
        // If system cursor is not visible then showCount should be -1
        lastSystemCursorShowState = false;
    }
    else
    {
        // Setup failure
        return false;
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
            GetCursorPos(&p);
            lastCursorPosition.x = p.x;
            lastCursorPosition.y = p.y;

            SetCursorInCenter();
        }
        else
        {
            SetCursorPos(lastCursorPosition.x, lastCursorPosition.y);
        }
        break;
    }
    case eCaptureMode::FRAME:
        Logger::Error("Unsupported cursor capture mode");
        break;
    default:
        DVASSERT_MSG(false, "Incorrect cursor capture mode");
        Logger::Error("Incorrect cursor capture mode");
        break;
    }
}

} //  namespace DAVA

#endif //  __DAVAENGINE_WIN32__
