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
    : WindowBackendBase(*window,
                        *engineBackend->GetDispatcher(),
                        MakeFunction(this, &WindowBackend::UIEventHandler))
    , engineBackend(engineBackend)
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
    PostResize(width, height);
}

void WindowBackend::Close()
{
    bridge->CloseWindow();
}

void WindowBackend::Detach()
{
    // On mac detach is similar to close
    Close();
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

void WindowBackend::UIEventHandler(const UIDispatcherEvent& e)
{
    switch (e.type)
    {
    case UIDispatcherEvent::RESIZE_WINDOW:
        bridge->ResizeWindow(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case UIDispatcherEvent::FUNCTOR:
        e.functor();
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
