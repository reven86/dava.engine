#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/Window/WindowBackendiOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Public/iOS/WindowNativeServiceiOS.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/iOS/PlatformCoreiOS.h"
#include "Engine/Private/iOS/Window/WindowNativeBridgeiOS.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
WindowBackend::WindowBackend(EngineBackend* e, Window* w)
    : engineBackend(e)
    , dispatcher(engineBackend->GetDispatcher())
    , window(w)
    , platformDispatcher(MakeFunction(this, &WindowBackend::EventHandler))
    , bridge(new WindowNativeBridge(this))
    , nativeService(new WindowNativeService(bridge))
{
}

WindowBackend::~WindowBackend()
{
    PlatformCore* core = engineBackend->GetPlatformCore();
    core->didBecomeResignActive.Disconnect(sigidAppBecomeOrResignActive);
    core->didEnterForegroundBackground.Disconnect(sigidAppDidEnterForegroundOrBackground);

    delete bridge;
}

void* WindowBackend::GetHandle() const
{
    return bridge->GetHandle();
}

bool WindowBackend::Create(float32 /*width*/, float32 /*height*/)
{
    // iOS windows are always created with size same as screen size
    if (bridge->DoCreateWindow())
    {
        PlatformCore* core = engineBackend->GetPlatformCore();
        sigidAppBecomeOrResignActive = core->didBecomeResignActive.Connect(bridge, &WindowNativeBridge::ApplicationDidBecomeOrResignActive);
        sigidAppDidEnterForegroundOrBackground = core->didEnterForegroundBackground.Connect(bridge, &WindowNativeBridge::ApplicationDidEnterForegroundOrBackground);
        return true;
    }
    return false;
}

void WindowBackend::Resize(float32 /*width*/, float32 /*height*/)
{
    // iOS windows are always stretched to screen size
}

void WindowBackend::Close()
{
    // iOS windows cannot be closed
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
    platformDispatcher.ProcessEvents();
}

void WindowBackend::EventHandler(const UIDispatcherEvent& e)
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
