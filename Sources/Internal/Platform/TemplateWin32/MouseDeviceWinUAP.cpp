#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#if !defined(__DAVAENGINE_COREV2__)

#include "UI/UIEvent.h"

#include "Platform/TemplateWin32/MouseDeviceWinUAP.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"

namespace DAVA
{
void MouseDeviceUWP::SetMode(eCaptureMode newMode)
{
    using ::Windows::UI::Core::CoreCursor;
    using ::Windows::UI::Core::CoreCursorType;
    using ::Windows::UI::Xaml::Window;
    using ::Windows::UI::Xaml::Controls::SwapChainPanel;

#if !defined(__DAVAENGINE_COREV2__)
    CorePlatformWinUAP* core = static_cast<CorePlatformWinUAP*>(Core::Instance());
    SwapChainPanel ^ swapchain = reinterpret_cast<SwapChainPanel ^>(DAVA::Core::Instance()->GetNativeView());
    DVASSERT(swapchain);

    if (eCaptureMode::PINING == newMode)
    {
        skipMouseMoveEvents = SKIP_N_MOUSE_MOVE_EVENTS;
        core->RunOnUIThread([]()
                            {
                                Window::Current->CoreWindow->PointerCursor = nullptr;
                            });
    }
    else
    {
        core->RunOnUIThread([]()
                            {
                                Window::Current->CoreWindow->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
                            });
    }
#endif // !__DAVAENGINE_COREV2__
}

void MouseDeviceUWP::SetCursorInCenter()
{
}

bool MouseDeviceUWP::SkipEvents(const UIEvent* event)
{
    if (event->device == eInputDevices::MOUSE && (event->phase == UIEvent::Phase::DRAG || event->phase == UIEvent::Phase::MOVE))
    {
        if (skipMouseMoveEvents)
        {
            skipMouseMoveEvents--;
            return true;
        }
    }
    return false;
}

} // namespace DAVA

#endif // !defined(__DAVAENGINE_COREV2__)

#endif //  __DAVAENGINE_WIN_UAP__