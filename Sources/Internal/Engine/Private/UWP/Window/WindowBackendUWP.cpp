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
WindowBackend::WindowBackend(EngineBackend* e, Window* w)
    : engine(e)
    , dispatcher(engine->GetDispatcher())
    , window(w)
    , platformDispatcher(MakeFunction(this, &WindowBackend::PlatformEventHandler))
    , bridge(ref new WindowNativeBridge(this))
    , nativeService(new WindowNativeService(bridge))
{
}

WindowBackend::~WindowBackend() = default;

void* WindowBackend::GetHandle() const
{
    return bridge->GetHandle();
}

void WindowBackend::Resize(float32 width, float32 height)
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::RESIZE_WINDOW;
    e.resizeEvent.width = width;
    e.resizeEvent.height = height;
    platformDispatcher.PostEvent(e);
}

void WindowBackend::Close()
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::CLOSE_WINDOW;
    platformDispatcher.PostEvent(e);
}

bool WindowBackend::IsWindowReadyForRender() const
{
    return GetHandle() != nullptr;
}

void WindowBackend::RunAsyncOnUIThread(const Function<void()>& task)
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::FUNCTOR;
    e.functor = task;
    platformDispatcher.PostEvent(e);
}

void WindowBackend::TriggerPlatformEvents()
{
    bridge->TriggerPlatformEvents();
}

void WindowBackend::ProcessPlatformEvents()
{
    // Method executes in context of XAML::Window's UI thread
    platformDispatcher.ProcessEvents();
}

void WindowBackend::BindXamlWindow(::Windows::UI::Xaml::Window ^ xamlWindow)
{
    // Method executes in context of XAML::Window's UI thread
    bridge->BindToXamlWindow(xamlWindow);
}

void WindowBackend::PlatformEventHandler(const UIDispatcherEvent& e)
{
    // Method executes in context of XAML::Window's UI thread
    switch (e.type)
    {
    case UIDispatcherEvent::RESIZE_WINDOW:
        bridge->DoResizeWindow(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case UIDispatcherEvent::CLOSE_WINDOW:
        bridge->DoCloseWindow();
        break;
    case UIDispatcherEvent::FUNCTOR:
        e.functor();
        break;
    case UIDispatcherEvent::CHANGE_MOUSE_MODE:
        bridge->DoChangeMouseMode(e.mouseMode);
    break;
    default:
        break;
    }
}

void WindowBackend::SetMouseMode(eMouseMode mode)
{
    if (mouseMode == mode)
    {
        return;
    }
    mouseMode = mode;
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::CHANGE_MOUSE_MODE;
    e.mouseMode = mode;
    platformDispatcher.PostEvent(e);
}

eMouseMode WindowBackend::GetMouseMode() const
{
    return mouseMode;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
