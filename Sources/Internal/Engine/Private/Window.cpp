#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/Engine.h"
#include "Engine/Public/Window.h"
#include "Engine/Private/PlatformWindow.h"
#include "Engine/Private/Dispatcher/DispatcherEvent.h"

namespace DAVA
{
Window::Window(bool primary)
    : isPrimary(primary)
{
}

Window::~Window() = default;

void Window::Resize(float32 width, float32 height)
{
    if (nativeWindow != nullptr)
    {
        nativeWindow->Resize(width, height);
    }
}

void* Window::NativeHandle() const
{
    if (nativeWindow != nullptr)
    {
        return nativeWindow->Handle();
    }
    return nullptr;
}

void Window::RunAsyncOnUIThread(const Function<void()>& task)
{
    if (nativeWindow != nullptr)
    {
        nativeWindow->RunAsyncOnUIThread(task);
    }
}

void Window::BindNativeWindow(Private::PlatformWindow* nativeWindow_)
{
    nativeWindow = nativeWindow_;
    signalWindowCreated.Emit(this);
}

void Window::PreHandleWindowCreated(const Private::DispatcherEvent& e)
{
    width = e.sizeEvent.width;
    height = e.sizeEvent.height;
    scaleX = e.sizeEvent.scaleX;
    scaleY = e.sizeEvent.scaleY;
}

void Window::HandleWindowCreated(const Private::DispatcherEvent& /*e*/)
{
    signalWindowCreated.Emit(this);
}

void Window::HandleWindowDestroyed(const Private::DispatcherEvent& e)
{
    signalWindowDestroyed.Emit(this);
    nativeWindow = nullptr;
}

void Window::PreHandleSizeScaleChanged(const Private::DispatcherEvent& e)
{
    width = e.sizeEvent.width;
    height = e.sizeEvent.height;
    scaleX = e.sizeEvent.scaleX;
    scaleY = e.sizeEvent.scaleY;
}

void Window::HandleSizeScaleChanged(const Private::DispatcherEvent& /*e*/)
{
    signalSizeScaleChanged.Emit(this, width, height, scaleX, scaleY);
}

void Window::HandleEvent(const Private::DispatcherEvent& e)
{
    using Private::DispatcherEvent;
    if (e.type == DispatcherEvent::WINDOW_FOCUS_CHANGED)
    {
        Logger::Error("****** WINDOW_FOCUS_CHANGED: state=%u", e.stateEvent.state);
        hasFocus = e.stateEvent.state != 0;
        signalFocusChanged.Emit(this, hasFocus);
    }
    else if (e.type == DispatcherEvent::WINDOW_VISIBILITY_CHANGED)
    {
        Logger::Error("****** WINDOW_VISIBILITY_CHANGED: state=%u", e.stateEvent.state);
        isVisible = e.stateEvent.state != 0;
        signalVisibilityChanged.Emit(this, isVisible);
    }
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
