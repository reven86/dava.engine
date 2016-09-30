#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/Window/WindowBackendOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include <AppKit/NSScreen.h>

#include "Engine/OsX/WindowNativeServiceOsX.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/OsX/PlatformCoreOsX.h"
#include "Engine/Private/OsX/Window/WindowNativeBridgeOsX.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
WindowBackend::WindowBackend(EngineBackend* engineBackend, Window* window)
    : engineBackend(engineBackend)
    , window(window)
    , mainDispatcher(engineBackend->GetDispatcher())
    , uiDispatcher(MakeFunction(this, &WindowBackend::UIEventHandler))
    , bridge(new WindowNativeBridge(this))
    , nativeService(new WindowNativeService(bridge.get()))
{
}

WindowBackend::~WindowBackend() = default;

void* WindowBackend::GetHandle() const
{
    return bridge->renderView;
}

bool WindowBackend::Create(float32 width, float32 height)
{
    hideUnhideSignalId = engineBackend->GetPlatformCore()->didHideUnhide.Connect(bridge.get(), &WindowNativeBridge::ApplicationDidHideUnhide);

    NSSize screenSize = [[NSScreen mainScreen] frame].size;
    float32 x = (screenSize.width - width) / 2.0f;
    float32 y = (screenSize.height - height) / 2.0f;
    return bridge->CreateWindow(x, y, width, height);
}

void WindowBackend::Resize(float32 width, float32 height)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateResizeEvent(width, height));
}

void WindowBackend::Close(bool /*appIsTerminating*/)
{
    closeRequestByApp = true;
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

bool WindowBackend::IsWindowReadyForRender() const
{
    return GetHandle() != nullptr;
}

void WindowBackend::TriggerPlatformEvents()
{
    bridge->TriggerPlatformEvents();
}

void WindowBackend::ProcessPlatformEvents()
{
    uiDispatcher.ProcessEvents();
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

void WindowBackend::UIEventHandler(const UIDispatcherEvent& e)
{
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

void WindowBackend::WindowWillClose()
{
    engineBackend->GetPlatformCore()->didHideUnhide.Disconnect(hideUnhideSignalId);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
