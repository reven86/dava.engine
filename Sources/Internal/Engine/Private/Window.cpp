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

Window::~Window()
{
}

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
    signalNativeWindowCreated.Emit(this);
}

void Window::HandleEvent(const Private::DispatcherEvent& e)
{
    using Private::DispatcherEvent;
    if (e.type == DispatcherEvent::WINDOW_FOCUS_CHANGED)
    {
        hasFocus = e.stateEvent.state != 0;
        signalFocusChanged.Emit(this, hasFocus);
    }
    else if (e.type == DispatcherEvent::WINDOW_VISIBILITY_CHANGED)
    {
        isVisible = e.stateEvent.state != 0;
        signalVisibilityChanged.Emit(this, isVisible);
    }
    else if (e.type == DispatcherEvent::WINDOW_SIZE_CHANGED)
    {
        width = e.sizeEvent.width;
        height = e.sizeEvent.height;
        //scaleX = e.sizeEvent.scaleX;
        //scaleY = e.sizeEvent.scaleY;
        signalSizeChanged.Emit(this, width, height);
        sizeCome = true;
    }
    else if (e.type == DispatcherEvent::WINDOW_SCALE_CHANGED)
    {
        //width = e.sizeEvent.width;
        //height = e.sizeEvent.height;
        scaleX = e.sizeEvent.scaleX;
        scaleY = e.sizeEvent.scaleY;
        signalScaleChanged.Emit(this, scaleX, scaleY);
    }
    else if (e.type == DispatcherEvent::WINDOW_CLOSED)
    {
        signalNativeWindowDestroyed.Emit(this);
        nativeWindow = nullptr;
        if (isPrimary)
        {
            Engine::Instance()->Quit();
        }
    }
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
