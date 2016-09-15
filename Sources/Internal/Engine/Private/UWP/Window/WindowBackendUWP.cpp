#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/UWP/Window/WindowBackendUWP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/UWP/WindowNativeServiceUWP.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/UWP/PlatformCoreUWP.h"
#include "Engine/Private/UWP/Window/WindowNativeBridgeUWP.h"

namespace DAVA
{
namespace Private
{
WindowBackend::WindowBackend(EngineBackend* engineBackend, Window* window)
    : WindowBackendBase(*window,
                        *engineBackend->GetDispatcher(),
                        MakeFunction(this, &WindowBackend::UIEventHandler))
    , bridge(ref new WindowNativeBridge(this))
    , nativeService(new WindowNativeService(bridge))
{
}

WindowBackend::~WindowBackend() = default;

void WindowBackend::Resize(float32 width, float32 height)
{
    PostResize(width, height);
}

void WindowBackend::Close()
{
    PostClose(false);
}

void WindowBackend::Detach()
{
    PostClose(true);
}

void* WindowBackend::GetHandle() const
{
    return bridge->GetHandle();
}

bool WindowBackend::IsWindowReadyForRender() const
{
    return GetHandle() != nullptr;
}

void WindowBackend::TriggerPlatformEvents()
{
    if (uiDispatcher.HasEvents())
    {
        bridge->TriggerPlatformEvents();
    }
}

void WindowBackend::ProcessPlatformEvents()
{
    // Method executes in context of XAML::Window's UI thread
    uiDispatcher.ProcessEvents();
}

void WindowBackend::BindXamlWindow(::Windows::UI::Xaml::Window ^ xamlWindow)
{
    // Method executes in context of XAML::Window's UI thread
    uiDispatcher.LinkToCurrentThread();
    bridge->BindToXamlWindow(xamlWindow);
}

void WindowBackend::UIEventHandler(const UIDispatcherEvent& e)
{
    // Method executes in context of XAML::Window's UI thread
    switch (e.type)
    {
    case UIDispatcherEvent::RESIZE_WINDOW:
        bridge->DoResizeWindow(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case UIDispatcherEvent::CLOSE_WINDOW:
        bridge->DoCloseWindow(e.closeEvent.detach ? true : false);
        break;
    case UIDispatcherEvent::FUNCTOR:
        e.functor();
        break;
    default:
        break;
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
