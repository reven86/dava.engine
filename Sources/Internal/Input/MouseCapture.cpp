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
#endif

MouseCapturePrivate* MouseCapture::GetPrivateImpl()
{
    static MouseCapturePrivate privateImpl;
    return &privateImpl;
}

void MouseCapture::SetMouseCaptureMode(DAVA::InputSystem::eMouseCaptureMode mode)
{
    GetPrivateImpl()->SetMouseCaptureMode(mode);
}

DAVA::InputSystem::eMouseCaptureMode MouseCapture::GetMouseCaptureMode()
{
    return GetPrivateImpl()->GetMouseCaptureMode();
}

void MouseCapture::SetApplicationFocus(bool isFocused)
{
    GetPrivateImpl()->SetApplicationFocus(isFocused);
}

bool MouseCapture::SkipEvents(DAVA::UIEvent* event)
{
    return GetPrivateImpl()->SkipEvents(event);
}
