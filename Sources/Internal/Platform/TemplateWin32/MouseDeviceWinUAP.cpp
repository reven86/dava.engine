#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Platform/TemplateWin32/MouseDeviceWinUAP.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"

using namespace ::Windows::UI::Core;
using namespace ::Windows::UI::Xaml;
using namespace ::Windows::UI::Xaml::Controls;

namespace DAVA
{
void MouseDeviceUWP::SetMode(eCaptureMode newMode)
{
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
}

void MouseDeviceUWP::SetCursorInCenter()
{
}

bool MouseDeviceUWP::SkipEvents(const UIEvent* event)
{
    if (event->device == UIEvent::Device::MOUSE)
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

#endif //  __DAVAENGINE_WIN_UAP__