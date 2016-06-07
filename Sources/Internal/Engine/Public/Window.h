#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Signal.h"
#include "Math/Vector.h"

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
class Window final
{
public:
    bool IsPrimary() const;
    bool IsVisible() const;
    bool HasFocus() const;

    float32 GetWidth() const;
    float32 GetHeight() const;
    float32 GetScaleX() const;
    float32 GetScaleY() const;

    Vector2 GetSize() const;
    Vector2 GetScale() const;

    void Resize(float32 w, float32 h);
    void Resize(Vector2 size);

    void* GetNativeHandle() const;

    void RunAsyncOnUIThread(const Function<void()>& task);

public:
    // Signals
    Signal<Window*, bool> visibilityChanged;
    Signal<Window*, bool> focusChanged;
    Signal<Window*, float32, float32, float32, float32> sizeScaleChanged;
    Signal<Window*, float32> update;

private:
    Window(Private::WindowBackend* backend);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

private:
    Private::WindowBackend* windowBackend = nullptr;

    // Friends
    friend class Private::EngineBackend;
    friend class Private::WindowBackend;
};

inline Vector2 Window::GetSize() const
{
    return Vector2(GetWidth(), GetHeight());
}

inline Vector2 Window::GetScale() const
{
    return Vector2(GetScaleX(), GetScaleY());
}

inline void Window::Resize(Vector2 size)
{
    Resize(size.dx, size.dy);
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
