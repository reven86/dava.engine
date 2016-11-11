#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/UWP/Window/WindowBackendUWP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/UWP/WindowNativeServiceUWP.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/UWP/PlatformCoreUWP.h"
#include "Engine/Private/UWP/Window/WindowNativeBridgeUWP.h"
#include "Platform/DeviceInfo.h"

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

void WindowBackend::SetFullscreen(eFullscreen newMode)
{
    // Fullscreen mode cannot be changed on phones
    if (IsWindowsPhone())
    {
        return;
    }

    uiDispatcher.PostEvent(UIDispatcherEvent::CreateSetFullscreenEvent(newMode));
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
    case UIDispatcherEvent::SET_FULLSCREEN:
        bridge->SetFullscreen(e.setFullscreenEvent.mode);
        break;
    case UIDispatcherEvent::FUNCTOR:
        e.functor();
        break;
    case UIDispatcherEvent::SET_CURSOR_CAPTURE:
        bridge->SetCursorCapture(e.setCursorCaptureEvent.mode);
        break;
    case UIDispatcherEvent::SET_CURSOR_VISIBILITY:
        bridge->SetCursorVisibility(e.setCursorVisibilityEvent.visible);
        break;
    default:
        break;
    }
}

bool WindowBackend::IsWindowsPhone() const
{
    return DeviceInfo::GetPlatform() == DeviceInfo::ePlatform::PLATFORM_PHONE_WIN_UAP;
}

void WindowBackend::SetCursorCapture(eCursorCapture mode)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateSetCursorCaptureEvent(mode));
}

void WindowBackend::SetCursorVisibility(bool visible)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateSetCursorVisibilityEvent(visible));
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
