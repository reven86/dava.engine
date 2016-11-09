#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/EngineTypes.h"
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
    WindowNativeBridge(WindowBackend* windowBackend);
    ~WindowNativeBridge();

    bool CreateWindow(float32 x, float32 y, float32 width, float32 height);
    void ResizeWindow(float32 width, float32 height);
    void CloseWindow();
    void SetTitle(const char8* title);
    float32 GetDpi();

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
    void MouseEntered(NSEvent* theEvent);
    void MouseExited(NSEvent* theEvent);
    void MouseWheel(NSEvent* theEvent);
    void KeyEvent(NSEvent* theEvent);
    void FlagsChanged(NSEvent* theEvent);
    void MagnifyWithEvent(NSEvent* theEvent);
    void RotateWithEvent(NSEvent* theEvent);
    void SwipeWithEvent(NSEvent* theEvent);

    static eModifierKeys GetModifierKeys(NSEvent* theEvent);
    static eMouseButtons GetMouseButton(NSEvent* theEvent);

    //////////////////////////////////////////////////////////////////////////

    WindowBackend* windowBackend = nullptr;
    Window* window = nullptr;
    MainDispatcher* mainDispatcher = nullptr;

    NSWindow* nswindow = nullptr;
    RenderView* renderView = nullptr;
    WindowDelegate* windowDelegate = nullptr;

    bool isAppHidden = false;
    bool isMiniaturized = false;
    uint32 lastModifierFlags = 0; // Saved NSEvent.modifierFlags to detect Shift, Alt presses
    bool isVisible = false;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
