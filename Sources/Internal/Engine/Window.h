#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include <bitset>

#include "Base/BaseTypes.h"
#include "Functional/Signal.h"
#include "Math/Vector.h"

#include "Engine/EngineTypes.h"
#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/EngineBackend.h"

namespace rhi
{
struct InitParam;
}

namespace DAVA
{
class InputSystem;
class UIControlSystem;
class VirtualCoordinatesSystem;

/** Window class. */
class Window final
{
    friend class Private::EngineBackend;
    friend class Private::PlatformCore;

private:
    Window(Private::EngineBackend* engineBackend, bool primary);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

public:
    bool IsPrimary() const;
    bool IsVisible() const;
    bool HasFocus() const;

    /** 
        Returns dots-per-inch for a monitor, where the window is placed. 
        
        \remark Use `Window::dpiChanged` signal to know, when window was placed on another monitor with different dpi.
    */
    float32 GetDPI() const;

    /** 
        Returns size of the window's client area. 
        Window size in screen coordinates may differ from the size in pixels,
        if the window was created on a system with high-dpi support (e.g. OSX or Windows 10).

        \remark Use `GetSurfaceSize()` to get rendering surface size in pixels.
        \remark Use `Window::sizeChanged` signal to know, when window size was changed.
    */
    Size2f GetSize() const;

    /** 
        Sets the size of the window's client area. 
        Window size in screen coordinates may differ from the size in pixels,
        if the windows was created on a system with high-dpi support (e.g. OSX or Windows 10).
        On some platforms (iOS, Android or Win10 Phone) there is no real window system and
        as a consequence window size can't be changed, so this function will have no effect.

        \remark Use `Window::sizeChanged` signal to know, when window size was changed.
    */
    void SetSize(Size2f size);

    /**
         Returns size of the window's rendering surface in pixels.
         Surface size is in raw pixels.

         \remark Use `Window::sizeChanged` signal to know, when window surface size was changed.
         \remark Use `SetSurfaceScale` to tune surface size.
    */
    Size2f GetSurfaceSize() const;

    /** 
        Returns window rendering surface scale. 
        By default it is 1.0f until user changes it with `SetSurfaceScale()` method.
    */
    float32 GetSurfaceScale() const;

    /** 
        Sets window rendering surface scale.
        Return `true` if scaling was successfully set. Scale value has to be from `0.0f` to `1.0f`.
        
        \remark This should be used by user to tune rendering surface size for performance reason.
    */
    bool SetSurfaceScale(float32 scale);

    void Close();
    void SetTitle(const String& title);

    Engine* GetEngine() const;
    void* GetNativeHandle() const;
    WindowNativeService* GetNativeService() const;

    void RunAsyncOnUIThread(const Function<void()>& task);

    /** Set cursor capture mode for current Window, see more about modes in eCursorCapture enum class.
        Supported on Win32, OsX, WinUWP.
        Remarks:
        The Window keeps the last mode, and releases or captures a cursor by itself if needed (when it loses or receives focus).
        If the last mode is pinning, it will set after any keyboard event (mouse wheel too) or mouse press event inside client area.
    */
    void SetCursorCapture(eCursorCapture mode);

    /** Get cursor capture mode.*/
    eCursorCapture GetCursorCapture() const;

    /** Set cursor visibility for current Window.
        Supported on Win32, OsX, WinUWP.
        Remarks:
        The Window keeps the last state, and shows or hides a cursor by itself if needed (when it loses or receives focus).
    */
    void SetCursorVisibility(bool visible);

    /** Get cursor visibility.*/
    bool GetCursorVisibility() const;

public:
    // Signals
    Signal<Window*, bool> visibilityChanged;
    Signal<Window*, bool> focusChanged;
    Signal<Window*, float32> dpiChanged;
    Signal<Window*, Size2f, Size2f> sizeChanged; //<! First Size2f is window size, second Size2f is window surface size
    //Signal<Window*> beginUpdate;
    //Signal<Window*> beginDraw;
    Signal<Window*, float32> update;
    //Signal<Window*> endDraw;
    //Signal<Window*> endUpdate;

private:
    /// Get pointer to WindowBackend which may be used by PlatformCore
    Private::WindowBackend* GetBackend() const;

    /// Initialize platform specific render params, e.g. acquire/release context functions for Qt platform
    void InitCustomRenderParams(rhi::InitParam& params);
    void Update(float32 frameDelta);
    void Draw();

    /// Process main dispatcher events targeting this window
    void EventHandler(const Private::MainDispatcherEvent& e);
    /// Do some window specific tasks after all dispatcher events have been processed on current frame,
    /// e.g. initiate processing tasks on window UI thread
    void FinishEventHandlingOnCurrentFrame();

    void HandleWindowCreated(const Private::MainDispatcherEvent& e);
    void HandleWindowDestroyed(const Private::MainDispatcherEvent& e);
    void HandleCursorCaptuleLost(const Private::MainDispatcherEvent& e);
    void HandleSizeChanged(const Private::MainDispatcherEvent& e);
    void HandleDpiChanged(const Private::MainDispatcherEvent& e);
    void HandleFocusChanged(const Private::MainDispatcherEvent& e);
    void HandleVisibilityChanged(const Private::MainDispatcherEvent& e);
    void HandleMouseClick(const Private::MainDispatcherEvent& e);
    void HandleMouseWheel(const Private::MainDispatcherEvent& e);
    void HandleMouseMove(const Private::MainDispatcherEvent& e);
    void HandleTouchClick(const Private::MainDispatcherEvent& e);
    void HandleTouchMove(const Private::MainDispatcherEvent& e);
    void HandleTrackpadGesture(const Private::MainDispatcherEvent& e);
    void HandleKeyPress(const Private::MainDispatcherEvent& e);
    void HandleKeyChar(const Private::MainDispatcherEvent& e);
    bool HandleInputActivation(const Private::MainDispatcherEvent& e);

    void MergeSizeChangedEvents(const Private::MainDispatcherEvent& e);
    void UpdateVirtualCoordinatesSystem();

private:
    Private::EngineBackend* engineBackend = nullptr;
    Private::MainDispatcher* mainDispatcher = nullptr;
    std::unique_ptr<Private::WindowBackend> windowBackend;

    InputSystem* inputSystem = nullptr;
    UIControlSystem* uiControlSystem = nullptr;

    bool isPrimary = false;
    bool isVisible = false;
    bool hasFocus = false;
    bool sizeEventsMerged = false; // Flag indicating that all size events are merged on current frame

    // Shortcut for eMouseButtons::COUNT
    static const size_t MOUSE_BUTTON_COUNT = static_cast<size_t>(eMouseButtons::COUNT);
    std::bitset<MOUSE_BUTTON_COUNT> mouseButtonState;
    eCursorCapture cursorCapture = eCursorCapture::OFF;
    bool cursorVisible = false;
    bool waitInputActivation = false;
    bool skipFirstMouseUpEventBeforeCursorCapture = false;
    float32 dpi = 0.0f; //!< Window DPI
    float32 width = 0.0f; //!< Window client area width.
    float32 height = 0.0f; //!< Window client area height.
    float32 surfaceWidth = 0.0f; //!< Window rendering surface width.
    float32 surfaceHeight = 0.0f; //!< Window rendering surface height.
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

inline float32 Window::GetDPI() const
{
    return dpi;
}

inline Size2f Window::GetSize() const
{
    return { width, height };
}

inline Size2f Window::GetSurfaceSize() const
{
    return { surfaceWidth, surfaceHeight };
}

inline Private::WindowBackend* Window::GetBackend() const
{
    return windowBackend.get();
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
