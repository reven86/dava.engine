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
    : engineBackend(engineBackend)
    , window(window)
    , mainDispatcher(engineBackend->GetDispatcher())
    , uiDispatcher(MakeFunction(this, &WindowBackend::UIEventHandler))
    , bridge(ref new WindowNativeBridge(this))
    , nativeService(new WindowNativeService(bridge))
{
}

WindowBackend::~WindowBackend() = default;

void WindowBackend::Resize(float32 width, float32 height)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateResizeEvent(width, height));
}

void WindowBackend::Close(bool /*appIsTerminating*/)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateCloseEvent());
}

void WindowBackend::SetTitle(const String& title)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateSetTitleEvent(title));
}

void WindowBackend::RunAsyncOnUIThread(const Function<void()>& task)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateFunctorEvent(task));
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
        bridge->ResizeWindow(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case UIDispatcherEvent::CLOSE_WINDOW:
        bridge->CloseWindow();
        break;
    case UIDispatcherEvent::SET_TITLE:
        bridge->SetTitle(e.setTitleEvent.title);
        delete[] e.setTitleEvent.title;
        break;
    case UIDispatcherEvent::FUNCTOR:
        e.functor();
        break;
    case UIDispatcherEvent::CHANGE_CAPTURE_MODE:
        bridge->ChangeCaptureMode(e.mouseMode);
        break;
    case UIDispatcherEvent::CHANGE_MOUSE_VISIBILITY:
        bridge->ChangeMouseVisibility(e.mouseVisible);
        break;
    default:
        break;
    }
}

bool WindowBackend::SetCaptureMode(eCaptureMode mode)
{
    if (eCaptureMode::FRAME == mode)
    {
        //for now, not supported
        return false;
    }
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::CHANGE_CAPTURE_MODE;
    e.mouseMode = mode;
    uiDispatcher.PostEvent(e);
    return true;
}

bool WindowBackend::SetMouseVisibility(bool visible)
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::CHANGE_MOUSE_VISIBILITY;
    e.mouseVisible = visible;
    uiDispatcher.PostEvent(e);
    return true;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
