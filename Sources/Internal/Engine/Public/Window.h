#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Signal.h"

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
class Window final
{
    friend class Private::EngineBackend;
    friend Private::PlatformWindow;

public:
    void Resize(float32 width, float32 height);
    void* NativeHandle() const;

    bool IsPrimary() const;
    bool IsVisible() const;
    bool HasFocus() const;

    float32 Width() const;
    float32 Height() const;
    float32 ScaleX() const;
    float32 ScaleY() const;

    void RunAsyncOnUIThread(const Function<void()>& task);

public:
    // Signals
    Signal<Window*> signalWindowCreated;
    Signal<Window*> signalWindowDestroyed;
    Signal<Window*, bool> signalVisibilityChanged;
    Signal<Window*, bool> signalFocusChanged;
    Signal<Window*, float32, float32, float32, float32> signalSizeScaleChanged;

private:
    Window(bool primary);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void BindNativeWindow(Private::PlatformWindow* nativeWindow_);

    void PreHandleWindowCreated(const Private::DispatcherEvent& e);
    void HandleWindowCreated(const Private::DispatcherEvent& e);

    void HandleWindowDestroyed(const Private::DispatcherEvent& e);

    void PreHandleSizeScaleChanged(const Private::DispatcherEvent& e);
    void HandleSizeScaleChanged(const Private::DispatcherEvent& e);

    void HandleFocusChanged(const Private::DispatcherEvent& e);
    void HandleVisibilityChanged(const Private::DispatcherEvent& e);

private:
    Private::PlatformWindow* nativeWindow = nullptr;

    bool isPrimary = false;
    bool isVisible = false;
    bool hasFocus = false;
    float32 width = 0.0f;
    float32 height = 0.0f;
    float32 scaleX = 1.0f;
    float32 scaleY = 1.0f;
};

inline bool Window::IsPrimary() const
{
    return isPrimary;
}

inline bool Window::IsVisible() const
{
    return isVisible;
}

inline bool Window::HasFocus() const
{
    return hasFocus;
}

inline float32 Window::Width() const
{
    return width;
}

inline float32 Window::Height() const
{
    return height;
}

inline float32 Window::ScaleX() const
{
    return scaleX;
}

inline float32 Window::ScaleY() const
{
    return scaleY;
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
