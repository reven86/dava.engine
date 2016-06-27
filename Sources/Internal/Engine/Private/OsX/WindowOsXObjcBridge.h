#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EngineFwd.h"

@class NSEvent;
@class NSWindow;
@class OpenGLViewOsX;
@class OsXWindowDelegate;

namespace DAVA
{
namespace Private
{
// Bridge between C++ and Objective-C for WindowOsX class
// Responsibilities:
//  - holds neccesary Objective-C objects
//  - creates NSWindow
//  - processes notifications from OsXWindowDelegate which implements
//    interface NSWindowDelegate
//  - posts events to dispatcher
//
// WindowOsXObjcBridge is friend of WindowOsX
struct WindowOsXObjcBridge final
{
    WindowOsXObjcBridge(WindowOsX* w);
    ~WindowOsXObjcBridge();

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

    //////////////////////////////////////////////////////////////////////////

    WindowOsX* window = nullptr;

    NSWindow* nswindow = nullptr;
    OpenGLViewOsX* openGLView = nullptr;
    OsXWindowDelegate* windowDelegate = nullptr;

    bool isAppHidden = false;
    bool isMiniaturized = false;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__