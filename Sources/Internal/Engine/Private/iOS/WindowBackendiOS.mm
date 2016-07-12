#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/WindowBackendiOS.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include <AppKit/NSScreen.h>

#include "Engine/Public/iOS/WindowNativeServiceiOS.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/iOS/PlatformCoreiOS.h"
#include "Engine/Private/iOS/WindowNativeBridgeiOS.h"

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
    , bridge(new WindowNativeBridgeiOS(this))
    , nativeService(new WindowNativeService(bridge))
{
}

WindowBackend::~WindowBackend()
{
    delete bridge;
}

void* WindowBackend::GetHandle() const
{
    return bridge->openGLView;
}

bool WindowBackend::Create(float32 width, float32 height)
{
    float32 x = 0.0f;
    float32 y = 0.0f;
    return bridge->DoCreateWindow(x, y, width, height);
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
    case UIDispatcherEvent::RESIZE_WINDOW:
        bridge->DoResizeWindow(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case UIDispatcherEvent::CLOSE_WINDOW:
        bridge->DoCloseWindow();
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

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
