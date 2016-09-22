#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EnginePrivateFwd.h"

@class NSEvent;
@class NSWindow;

@class RenderView;
@class WindowDelegate;

namespace DAVA
{
namespace Private
{
// Bridge between C++ and Objective-C for OsX's WindowBackend class
// Responsibilities:
//  - holds neccesary Objective-C objects
//  - creates NSWindow
//  - processes notifications from WindowDelegate which implements
//    interface NSWindowDelegate
//  - posts events to dispatcher
//
// WindowNativeBridge is friend of OsX's WindowBackend
struct WindowNativeBridge final
{
    WindowNativeBridge(WindowBackend* wbackend);
    ~WindowNativeBridge();

    bool DoCreateWindow(float32 x, float32 y, float32 width, float32 height);
    void DoResizeWindow(float32 width, float32 height);
    void DoCloseWindow();

    void TriggerPlatformEvents();

    void ApplicationDidHideUnhide(bool hidden);

    void WindowDidMiniaturize();
    void WindowDidDeminiaturize();
    void WindowDidBecomeKey();
    void WindowDidResignKey();
    void WindowDidResize();
    //void WindowWillStartLiveResize();
    //void WindowDidEndLiveResize();
    void WindowDidChangeScreen();
    bool WindowShouldClose();
    void WindowWillClose();

    void MouseClick(NSEvent* theEvent);
    void MouseMove(NSEvent* theEvent);
    void MouseWheel(NSEvent* theEvent);
    void KeyEvent(NSEvent* theEvent);
    void MouseEntered(NSEvent* theEvent);
    void MouseExited(NSEvent* theEvent);
    void SetMouseMode(eMouseMode mode);
    eMouseMode GetMouseMode() const;

    //////////////////////////////////////////////////////////////////////////

    WindowBackend* windowBackend = nullptr;

    NSWindow* nswindow = nullptr;
    RenderView* renderView = nullptr;
    WindowDelegate* windowDelegate = nullptr;

    bool isAppHidden = false;
    bool isMiniaturized = false;

private:
    void CreateOrUpdateTrackArea();
    void* GetOrCreateBlankCursor();
    void SetMouseVisibility(bool visible);
    void SetMouseCaptured(bool capture);
    bool DeferredMouseMode(const MainDispatcherEvent& e);

    //for using MouseEntered MouseExited events, set trackArea
    NSTrackingArea* trackingArea = nullptr;
    // set blank cursor, not use [NSCursor hide/unhide], system sometimes show it
    void* blankCursor = nullptr;
    bool mouseCaptured = false;
    bool mouseVisibled = true;
    bool deferredMouseMode = false;
    eMouseMode nativeMouseMode = eMouseMode::OFF;
    bool hasFocus = false;
    bool focusChanged = false;
    // If mouse pointer was outside window rectangle when enabling pinning mode then
    // mouse clicks are forwarded to other windows and our application loses focus.
    // So move mouse pointer to window center before enabling pinning mode.
    // Secondly, after using CGWarpMouseCursorPosition function to center mouse pointer
    // mouse move events arrive with big delta which causes mouse hopping.
    // The best solution I have investigated is to skip first N mouse move events after enabling
    // pinning mode: global variable skipMouseMoveEvents is set to some reasonable value
    // and is checked in OpenGLView's process method to skip mouse move events
    uint32 skipMouseMoveEvents = 0;
    const uint32 SKIP_N_MOUSE_MOVE_EVENTS = 4;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
