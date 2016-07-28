#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Android/Window/WindowBackendAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Public/Android/WindowNativeServiceAndroid.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/Android/PlatformCoreAndroid.h"

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
    , nativeService(new WindowNativeService(this))
{
}

WindowBackend::~WindowBackend() = default;

bool WindowBackend::Create(float32 width, float32 height)
{
    return false;
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
}

void WindowBackend::DoResizeWindow(float32 width, float32 height)
{
}

void WindowBackend::DoCloseWindow()
{
}

void WindowBackend::EventHandler(const UIDispatcherEvent& e)
{
    switch (e.type)
    {
    case UIDispatcherEvent::RESIZE_WINDOW:
        DoResizeWindow(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case UIDispatcherEvent::CLOSE_WINDOW:
        DoCloseWindow();
        break;
    case UIDispatcherEvent::FUNCTOR:
        e.functor();
        break;
    default:
        break;
    }
}

void WindowBackend::OnResume()
{
    Logger::Info("******** WindowBackend::OnResume: thread=%llX", Thread::GetCurrentIdAsInteger());

    window->PostVisibilityChanged(true);
    window->PostFocusChanged(true);
}

void WindowBackend::OnPause()
{
    Logger::Info("******** WindowBackend::OnPause: thread=%llX", Thread::GetCurrentIdAsInteger());

    window->PostFocusChanged(false);
    window->PostVisibilityChanged(false);
}

void WindowBackend::SurfaceChanged(JNIEnv* env, jobject surface, int width, int height)
{
    Logger::Info("******** WindowBackend::SurfaceChanged: thread=%llX, w=%d, h=%d", Thread::GetCurrentIdAsInteger(), width, height);

    if (androidWindow != nullptr)
    {
        ANativeWindow_release(androidWindow);
    }
    androidWindow = ANativeWindow_fromSurface(env, surface);

    if (firstTimeSurfaceChanged)
    {
        window->PostWindowCreated(this, static_cast<float32>(width), static_cast<float32>(height), 1.0f, 1.0f);
        firstTimeSurfaceChanged = false;
    }
    else
    {
        window->PostSizeChanged(static_cast<float32>(width), static_cast<float32>(height), 1.0f, 1.0f);
    }
}

void WindowBackend::SurfaceDestroyed()
{
    Logger::Info("******** WindowBackend::SurfaceDestroyed: thread=%llX", Thread::GetCurrentIdAsInteger());

    if (androidWindow != nullptr)
    {
        ANativeWindow_release(androidWindow);
        androidWindow = nullptr;
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
