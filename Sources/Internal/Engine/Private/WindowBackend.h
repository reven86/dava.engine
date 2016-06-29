#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Function.h"
#include "UI/UIEvent.h"

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
class InputSystem;
class UIControlSystem;
class VirtualCoordinatesSystem;

namespace Private
{
class WindowBackend final
{
public:
    WindowBackend(EngineBackend* e, bool primary);
    ~WindowBackend();

    bool IsPrimary() const;
    bool IsVisible() const;
    bool HasFocus() const;

    // Window size in logical pixels
    float32 GetWidth() const;
    float32 GetHeight() const;
    // Window's render surface size in pixels
    float32 GetRenderSurfaceWidth() const;
    float32 GetRenderSurfaceHeight() const;

    // Window scale factors
    float32 GetScaleX() const;
    float32 GetScaleY() const;
    // Additional user scale factor
    float32 GetUserScale() const;
    // Window's render surface scale factors
    float32 GetRenderSurfaceScaleX() const;
    float32 GetRenderSurfaceScaleY() const;

    void Resize(float32 w, float32 h);
    void Close();
    Window* GetWindow() const;
    void* GetNativeHandle() const;
    NativeWindow* GetNativeWindow() const;

    void RunAsyncOnUIThread(const Function<void()>& task);

    //////////////////////////////////////////////////////////////////////////

    void EventHandler(const DispatcherEvent& e);
    void FinishEventHandlingOnCurrentFrame();
    void Update(float32 frameDelta);
    void Draw();

    //////////////////////////////////////////////////////////////////////////

    void PostFocusChanged(bool focus);
    void PostVisibilityChanged(bool visibility);
    void PostSizeChanged(float32 width, float32 height, float32 scaleX, float32 scaleY);
    void PostWindowCreated(NativeWindow* native, float32 width, float32 height, float32 scaleX, float32 scaleY);
    void PostWindowDestroyed();

    void PostKeyDown(uint32 key, bool isRepeated);
    void PostKeyUp(uint32 key);
    void PostKeyChar(uint32 key, bool isRepeated);

private:
    void HandleWindowCreated(const DispatcherEvent& e);
    void HandleWindowDestroyed(const DispatcherEvent& e);
    void HandleSizeChanged(const DispatcherEvent& e);
    void HandleFocusChanged(const DispatcherEvent& e);
    void HandleVisibilityChanged(const DispatcherEvent& e);
    void HandleMouseClick(const DispatcherEvent& e);
    void HandleMouseWheel(const DispatcherEvent& e);
    void HandleMouseMove(const DispatcherEvent& e);
    void HandleKeyPress(const DispatcherEvent& e);
    void HandleKeyChar(const DispatcherEvent& e);

    void HandlePendingSizeChanging();

    void ClearMouseButtons();

private:
    // TODO: std::unique_ptr<Window>
    Window* window = nullptr;
    NativeWindow* nativeWindow = nullptr;
    EngineBackend* engineBackend = nullptr;
    Dispatcher* dispatcher = nullptr;

    InputSystem* inputSystem = nullptr;
    UIControlSystem* uiControlSystem = nullptr;
    VirtualCoordinatesSystem* virtualCoordSystem = nullptr;

    bool isPrimary = false;
    bool isVisible = false;
    bool hasFocus = false;
    float32 width = 0.0f;
    float32 height = 0.0f;
    float32 scaleX = 1.0f;
    float32 scaleY = 1.0f;
    float32 userScale = 1.0f;

    bool pendingInitRender = false;
    bool pendingSizeChanging = false;

    Bitset<static_cast<size_t>(UIEvent::MouseButton::NUM_BUTTONS)> mouseButtonState;
};

inline bool WindowBackend::IsPrimary() const
{
    return isPrimary;
}

inline bool WindowBackend::IsVisible() const
{
    return isVisible;
}

inline bool WindowBackend::HasFocus() const
{
    return hasFocus;
}

inline float32 WindowBackend::GetWidth() const
{
    return width;
}

inline float32 WindowBackend::GetHeight() const
{
    return height;
}

inline float32 WindowBackend::GetRenderSurfaceWidth() const
{
    return width * scaleX * userScale;
}

inline float32 WindowBackend::GetRenderSurfaceHeight() const
{
    return height * scaleY * userScale;
}

inline float32 WindowBackend::GetScaleX() const
{
    return scaleX;
}

inline float32 WindowBackend::GetScaleY() const
{
    return scaleY;
}

inline float32 WindowBackend::GetUserScale() const
{
    return userScale;
}

inline float32 WindowBackend::GetRenderSurfaceScaleX() const
{
    return scaleX * userScale;
}

inline float32 WindowBackend::GetRenderSurfaceScaleY() const
{
    return scaleY * userScale;
}

inline Window* WindowBackend::GetWindow() const
{
    return window;
}

inline NativeWindow* WindowBackend::GetNativeWindow() const
{
    return nativeWindow;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
