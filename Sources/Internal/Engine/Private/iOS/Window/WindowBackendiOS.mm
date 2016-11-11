#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/Window/WindowBackendiOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/iOS/WindowNativeServiceiOS.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/iOS/PlatformCoreiOS.h"
#include "Engine/Private/iOS/Window/WindowNativeBridgeiOS.h"
#include "Engine/Private/iOS/Window/RenderViewiOS.mm"

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

WindowBackend::~WindowBackend()
{
    PlatformCore* core = engineBackend->GetPlatformCore();
    core->didBecomeResignActive.Disconnect(sigidAppBecomeOrResignActive);
    core->didEnterForegroundBackground.Disconnect(sigidAppDidEnterForegroundOrBackground);
}

void* WindowBackend::GetHandle() const
{
    return bridge->GetHandle();
}

bool WindowBackend::Create()
{
    // iOS windows are always created with size same as screen size
    if (bridge->CreateWindow())
    {
        PlatformCore* core = engineBackend->GetPlatformCore();
        sigidAppBecomeOrResignActive = core->didBecomeResignActive.Connect(bridge.get(),
                                                                           &WindowNativeBridge::ApplicationDidBecomeOrResignActive);
        sigidAppDidEnterForegroundOrBackground = core->didEnterForegroundBackground.Connect(bridge.get(),
                                                                                            &WindowNativeBridge::ApplicationDidEnterForegroundOrBackground);
        return true;
    }
    return false;
}

void WindowBackend::Resize(float32 /*width*/, float32 /*height*/)
{
    // iOS windows are always stretched to screen size
}

void WindowBackend::Close(bool appIsTerminating)
{
    // iOS windows cannot be closed
    // TODO: later add ability to close secondary windows

    if (appIsTerminating)
    {
        // If application is terminating then send event as if window has been destroyed.
        // Engine ensures that Close with appIsTerminating with true value is always called on termination.
        mainDispatcher->SendEvent(MainDispatcherEvent::CreateWindowDestroyedEvent(window));
    }
}

void WindowBackend::SetTitle(const String& title)
{
    // iOS window does not have title
}

void WindowBackend::SetFullscreen(eFullscreen /*newMode*/)
{
    // Fullscreen mode cannot be changed on iOS
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

float32 WindowBackend::WindowBackend::GetSurfaceScale() const
{
    return [bridge->renderView surfaceScale];
}

bool WindowBackend::SetSurfaceScale(float32 scale)
{
    DVASSERT(scale > 0.0f && scale <= 1.0f);

    [bridge->renderView setSurfaceScale:scale];

    CGSize size = [bridge->renderView frame].size;
    CGSize surfaceSize = [bridge->renderView surfaceSize];
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, size.width, size.height, surfaceSize.width, surfaceSize.height));

    return true;
}

void WindowBackend::SetCursorCapture(eCursorCapture mode)
{
    // not supported
}

void WindowBackend::SetCursorVisibility(bool visible)
{
    // not supported
}

void WindowBackend::UIEventHandler(const UIDispatcherEvent& e)
{
    switch (e.type)
    {
    // iOS windows cannot be closed and are always stretched to screen size
    // case UIDispatcherEvent::CLOSE_WINDOW:
    // case UIDispatcherEvent::RESIZE_WINDOW:
    case UIDispatcherEvent::FUNCTOR:
        e.functor();
        break;
    default:
        break;
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
