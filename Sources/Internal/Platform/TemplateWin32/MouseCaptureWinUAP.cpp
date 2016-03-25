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

#if defined(__DAVAENGINE_WIN_UAP__)

using namespace ::Windows::UI::Core;
using namespace ::Windows::UI::Xaml;
using namespace ::Windows::UI::Xaml::Controls;

void MouseCapturePrivate::SetNativePining(DAVA::InputSystem::eMouseCaptureMode newMode)
{
    SwapChainPanel ^ swapchain = reinterpret_cast<SwapChainPanel ^>(DAVA::Core::Instance()->GetNativeView());
    DVASSERT(swapchain);
    DAVA::Logger::Info("!!!!!! SetNativePining %d", (int)newMode);

    if (DAVA::InputSystem::eMouseCaptureMode::PINING == newMode)
    {
        swapchain->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([]()
                                                                                                  {
                                                                                                      Window::Current->CoreWindow->PointerCursor = nullptr;
                                                                                                  }));
    }
    else
    {
        swapchain->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([]()
                                                                                                  {
                                                                                                      Window::Current->CoreWindow->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
                                                                                                  }));
    }
}

void MouseCapturePrivate::SetCursorPosition()
{
}

#endif
