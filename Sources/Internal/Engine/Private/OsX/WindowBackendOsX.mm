#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/WindowBackendOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include <AppKit/NSScreen.h>

#include "Engine/Public/OsX/WindowNativeServiceOsX.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/OsX/PlatformCoreOsX.h"
#include "Engine/Private/OsX/WindowBackendObjcBridge.h"

#include "Concurrency/LockGuard.h"
#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
WindowBackend::WindowBackend(EngineBackend* e, Window* w)
    : engine(e)
    , dispatcher(engine->GetDispatcher())
    , window(w)
    , platformDispatcher(MakeFunction(this, &WindowBackend::EventHandler))
    , bridge(new WindowBackendObjcBridge(this))
    , nativeService(new WindowNativeService(bridge))
{
    hideUnhideSignalId = engine->GetPlatformCore()->didHideUnhide.Connect(bridge, &WindowBackendObjcBridge::ApplicationDidHideUnhide);
}

WindowBackend::~WindowBackend()
{
    engine->GetPlatformCore()->didHideUnhide.Disconnect(hideUnhideSignalId);
    delete bridge;
}

void* WindowBackend::GetHandle() const
{
    return bridge->openGLView;
}

bool WindowBackend::Create(float32 width, float32 height)
{
    NSSize screenSize = [[NSScreen mainScreen] frame].size;
    float32 x = (screenSize.width - width) / 2.0f;
    float32 y = (screenSize.height - height) / 2.0f;
    return bridge->DoCreateWindow(x, y, width, height);
}

void WindowBackend::Resize(float32 width, float32 height)
{
    PlatformEvent e;
    e.type = PlatformEvent::RESIZE_WINDOW;
    e.resizeEvent.width = width;
    e.resizeEvent.height = height;
    platformDispatcher.PostEvent(e);
}

void WindowBackend::Close()
{
    PlatformEvent e;
    e.type = PlatformEvent::CLOSE_WINDOW;
    platformDispatcher.PostEvent(e);
}

void WindowBackend::RunAsyncOnUIThread(const Function<void()>& task)
{
    PlatformEvent e;
    e.type = PlatformEvent::FUNCTOR;
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

void WindowBackend::EventHandler(const PlatformEvent& e)
{
    switch (e.type)
    {
    case PlatformEvent::RESIZE_WINDOW:
        bridge->DoResizeWindow(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case PlatformEvent::CLOSE_WINDOW:
        bridge->DoCloseWindow();
        break;
    case PlatformEvent::FUNCTOR:
        e.functor();
        break;
    default:
        break;
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
