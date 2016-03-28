#include "Base/Platform.h"
#include "Core/Core.h"

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
#endif

MouseCapturePrivate* MouseCapture::GetPrivateImpl()
{
    static MouseCapturePrivate privateImpl;
    return &privateImpl;
}

void MouseCapture::SetMouseCaptureMode(DAVA::InputSystem::eMouseCaptureMode newMode)
{
    if (mode != newMode)
    {
        DAVA::Logger::Info("!!!!!! SetMouseCaptureMode %d", int(newMode));
        mode = newMode;
        if (DAVA::InputSystem::eMouseCaptureMode::OFF == mode)
        {
            SetNativePining(mode);
            deferredCapture = false;
        }

        if (DAVA::InputSystem::eMouseCaptureMode::PINING == mode)
        {
            if (focused && !focusChenged)
            {
                SetNativePining(mode);
            }
            else
            {
                deferredCapture = true;
            }
        }
    }
}

DAVA::InputSystem::eMouseCaptureMode MouseCapture::GetMouseCaptureMode()
{
    return nativeMode;
}

void MouseCapture::SetApplicationFocus(bool isFocused)
{
    if (focused != isFocused)
    {
        focusChenged = true;
        if (firstEntered)
        {
            firstEntered = false;
            focusChenged = false;
        }
        DAVA::Logger::Info("!!!!!! focus %d", int(isFocused));

        focused = isFocused;
        if (DAVA::InputSystem::eMouseCaptureMode::PINING == mode)
        {
            if (focused)
            {
                DAVA::Logger::Info("!!!!!! deferredCapture = true ");
                deferredCapture = true;
            }
            else
            {
                SetNativePining(DAVA::InputSystem::eMouseCaptureMode::OFF);
            }
        }
    }
}

bool MouseCapture::SkipEvents(DAVA::UIEvent* event)
{
    focusChenged = false;
    if (DAVA::InputSystem::eMouseCaptureMode::PINING == mode && focused && !deferredCapture)
    {
        GetPrivateImpl()->SetCursorPosition();
    }
    if (!deferredCapture && focused)
    {
        return false;
    }
    if (event->phase == DAVA::UIEvent::Phase::ENDED)
    {
        bool inRect = true;
        DAVA::Vector2 windowSize = DAVA::Core::Instance()->GetWindowSize();
        inRect &= (event->point.x >= 0.f && event->point.x <= windowSize.x);
        inRect &= (event->point.y >= 0.f && event->point.y <= windowSize.y);
        if (inRect)
        {
            DAVA::Logger::Info("!!!!!! SkipEvents deferredCapture = false");
            deferredCapture = false;
            SetNativePining(DAVA::InputSystem::eMouseCaptureMode::PINING);
        }
    }
    return true;
}

void MouseCapture::SetNativePining(DAVA::InputSystem::eMouseCaptureMode newNativeMode)
{
    if (newNativeMode != nativeMode)
    {
        nativeMode = newNativeMode;
        GetPrivateImpl()->SetNativePining(nativeMode);
    }
}

DAVA::InputSystem::eMouseCaptureMode MouseCapture::mode = DAVA::InputSystem::eMouseCaptureMode::OFF;
DAVA::InputSystem::eMouseCaptureMode MouseCapture::nativeMode = DAVA::InputSystem::eMouseCaptureMode::OFF;
bool MouseCapture::focused = false;
bool MouseCapture::focusChenged = false;
bool MouseCapture::firstEntered = true;
bool MouseCapture::deferredCapture = false;
