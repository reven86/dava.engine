#include "Base/Platform.h"

#if defined(__DAVAENGINE_IPHONE__)
#include "Platform/TemplateiOS/MouseCaptureIOS.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Platform/TemplateMacOS/MouseCaptureMacOS.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/MouseCaptureAndroid.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Platform/TemplateWin32/MouseCaptureWin32.h"
#include "Platform/TemplateWin32/CorePlatformWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Platform/TemplateWin32/MouseCaptureWinUAP.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#endif

#if defined(__DAVAENGINE_WIN32__)

void MouseCapturePrivate::SetCursorPosition()
{
    HWND hWnd = static_cast<HWND>(DAVA::Core::Instance()->GetNativeView());
    RECT wndRect;
    GetWindowRect(hWnd, &wndRect);
    int centerX = static_cast<int>((wndRect.left + wndRect.right) >> 1);
    int centerY = static_cast<int>((wndRect.bottom + wndRect.top) >> 1);
    SetCursorPos(centerX, centerY);
}

bool MouseCapturePrivate::SetSystemCursorVisibility(bool show)
{
    DAVA::Logger::Info("!!!!!!! SetSystemCursorVisibility %d", show);

    DAVA::int32 showCount = 0;
    showCount = ShowCursor(show); // No cursor info available, just call
    DAVA::Logger::Info("!!!!!!! showCount %d", showCount);

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

void MouseCapturePrivate::SetNativePining(DAVA::InputSystem::eMouseCaptureMode mode)
{
    DAVA::Logger::Info("!!!!!! NativePining %d", int(mode));

    static DAVA::Point2i lastCursorPosition;

    switch (mode)
    {
    case DAVA::InputSystem::eMouseCaptureMode::OFF:
    case DAVA::InputSystem::eMouseCaptureMode::PINING:
    {
        SetSystemCursorVisibility(mode != DAVA::InputSystem::eMouseCaptureMode::PINING);
        if (mode == DAVA::InputSystem::eMouseCaptureMode::PINING)
        {
            POINT p;
            GetCursorPos(&p);
            lastCursorPosition.x = p.x;
            lastCursorPosition.y = p.y;

            HWND hWnd = static_cast<HWND>(DAVA::Core::Instance()->GetNativeView());
            RECT wndRect;
            GetWindowRect(hWnd, &wndRect);
            int centerX = static_cast<int>((wndRect.left + wndRect.right) >> 1);
            int centerY = static_cast<int>((wndRect.bottom + wndRect.top) >> 1);
            SetCursorPos(centerX, centerY);
        }
        else
        {
            SetCursorPos(lastCursorPosition.x, lastCursorPosition.y);
        }
        break;
    }
    case DAVA::InputSystem::eMouseCaptureMode::FRAME:
        DAVA::Logger::Error("Unsupported cursor capture mode");
        break;
    default:
        DVASSERT_MSG(false, "Incorrect cursor capture mode");
        DAVA::Logger::Error("Incorrect cursor capture mode");
        break;
    }
}


#endif
