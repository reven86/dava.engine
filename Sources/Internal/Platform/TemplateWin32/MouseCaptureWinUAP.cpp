#include "Base/Platform.h"

#if defined(__DAVAENGINE_IPHONE__)
#include "Platform/TemplateiOS/MouseCaptureIOS.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Platform/TemplateMacOS/MouseCaptureMacOS.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/MouseCaptureAndroid.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Platform/TemplateWin32/MouseCaptureWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Platform/TemplateWin32/MouseCaptureWinUAP.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#endif

#include "Platform/SystemTimer.h"

void MouseCapturePrivate::SetMouseCaptureMode(DAVA::InputSystem::eMouseCaptureMode newMode)
{
    DAVA::Logger::Info("!!!!! SetMouseCaptureMode %d", (int)newMode);
    if (mode != newMode)
    {
        mode = newMode;
        if (DAVA::InputSystem::eMouseCaptureMode::OFF == mode)
        {
            return NativePining(mode);
        }

        if (DAVA::InputSystem::eMouseCaptureMode::PINING == mode)
        {
            if (focused)
            {
                NativePining(mode);
            }
            else
            {
                deferredCapture = true;
            }
        }
    }
}

DAVA::InputSystem::eMouseCaptureMode MouseCapturePrivate::GetMouseCaptureMode()
{
    return mode;
}

void MouseCapturePrivate::SetApplicationFocus(bool isFocused)
{
    DAVA::Logger::Info("!!!!! SetApplicationFocus focusChanged %d, isFocused %d, focused %d, mode %d", (int)focusChanged, (int)isFocused, (int)focused, (int)mode);
    if (focused != isFocused)
    {
        focused = isFocused;
        if (mode == DAVA::InputSystem::eMouseCaptureMode::PINING)
        {
            if (focused)
            {
                deferredCapture = true;
            }
            else
            {
                NativePining(DAVA::InputSystem::eMouseCaptureMode::OFF);
                focusChanged = false;
            }
        }
    }
}

bool MouseCapturePrivate::SkipEvents(DAVA::UIEvent* event)
{
    if (!deferredCapture && focused)
    {
        return false;
    }
    if (event->phase == DAVA::UIEvent::Phase::ENDED)
    {
        DAVA::Logger::Info("!!!!! SkipEvents deferredCapture  false");
        deferredCapture = false;
        focusChanged = false;
        NativePining(DAVA::InputSystem::eMouseCaptureMode::PINING);
    }
    return true;
}

void MouseCapturePrivate::NativePining(DAVA::InputSystem::eMouseCaptureMode newMode)
{
    DAVA::Logger::Info("!!!!! @@@@ NativePining %d", (int)newMode);
    (static_cast<DAVA::CorePlatformWinUAP*>(DAVA::Core::Instance()))->SetMouseCaptureMode(newMode);
}
