#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/WindowOsXObjcBridge.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSWindow.h>
#import <AppKit/NSScreen.h>

#include "Engine/Private/Dispatcher/Dispatcher.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/OsX/WindowOsX.h"
#include "Engine/Private/OsX/OpenGLViewOsX.h"
#include "Engine/Private/OsX/OsXWindowDelegate.h"

#include "Platform/SystemTimer.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace Private
{
WindowOsXObjcBridge::WindowOsXObjcBridge(WindowOsX* w)
    : window(w)
{
}

WindowOsXObjcBridge::~WindowOsXObjcBridge() = default;

bool WindowOsXObjcBridge::DoCreateWindow(float32 x, float32 y, float32 width, float32 height)
{
    NSUInteger style = NSTitledWindowMask |
    NSMiniaturizableWindowMask |
    NSClosableWindowMask |
    NSResizableWindowMask;

    NSRect viewRect = NSMakeRect(x, y, width, height);
    windowDelegate = [[OsXWindowDelegate alloc] init:this];
    openGLView = [[OpenGLViewOsX alloc] initWithFrame:viewRect bridge:this];

    nswindow = [[NSWindow alloc] initWithContentRect:viewRect
                                           styleMask:style
                                             backing:NSBackingStoreBuffered
                                               defer:NO];
    [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [nswindow setContentView:openGLView];
    [nswindow setDelegate:windowDelegate];

    {
        viewRect = [openGLView frame];
        float32 scale = [nswindow backingScaleFactor];

        window->window->PostWindowCreated(window, viewRect.size.width, viewRect.size.height, scale, scale);
        window->window->PostVisibilityChanged(true);
    }

    [nswindow makeKeyAndOrderFront:nil];
    return true;
}

void WindowOsXObjcBridge::DoResizeWindow(float32 width, float32 height)
{
    [nswindow setContentSize:NSMakeSize(width, height)];
}

void WindowOsXObjcBridge::DoCloseWindow()
{
    [nswindow close];
}

void WindowOsXObjcBridge::TriggerPlatformEvents()
{
    dispatch_async(dispatch_get_main_queue(), [this]() {
        window->ProcessPlatformEvents();
    });
}

void WindowOsXObjcBridge::ApplicationDidHideUnhide(bool hidden)
{
    isAppHidden = hidden;
}

void WindowOsXObjcBridge::WindowDidMiniaturize()
{
    isMiniaturized = true;
    window->window->PostVisibilityChanged(false);
}

void WindowOsXObjcBridge::WindowDidDeminiaturize()
{
    isMiniaturized = false;
}

void WindowOsXObjcBridge::WindowDidBecomeKey()
{
    if (isMiniaturized || isAppHidden)
    {
        window->window->PostVisibilityChanged(true);
    }
    window->window->PostFocusChanged(true);
}

void WindowOsXObjcBridge::WindowDidResignKey()
{
    window->window->PostFocusChanged(false);
    if (isAppHidden)
    {
        window->window->PostVisibilityChanged(false);
    }
}

void WindowOsXObjcBridge::WindowDidResize()
{
    float32 scale = [nswindow backingScaleFactor];
    CGSize size = [openGLView frame].size;

    window->window->PostSizeChanged(size.width, size.height, scale, scale);
}

void WindowOsXObjcBridge::WindowDidChangeScreen()
{
}

bool WindowOsXObjcBridge::WindowShouldClose()
{
    return true;
}

void WindowOsXObjcBridge::WindowWillClose()
{
    window->window->PostWindowDestroyed();

    [nswindow setContentView:nil];
    [nswindow setDelegate:nil];

    [openGLView release];
    [windowDelegate release];
}

void WindowOsXObjcBridge::MouseClick(NSEvent* theEvent)
{
    DispatcherEvent e;
    e.window = window->window;
    e.mclickEvent.clicks = 1;
    e.mclickEvent.button = [theEvent buttonNumber] + 1;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();

    switch ([theEvent type])
    {
    case NSLeftMouseDown:
    case NSRightMouseDown:
    case NSOtherMouseDown:
        e.type = DispatcherEvent::MOUSE_BUTTON_DOWN;
        break;
    case NSLeftMouseUp:
    case NSRightMouseUp:
    case NSOtherMouseUp:
        e.type = DispatcherEvent::MOUSE_BUTTON_UP;
        break;
    default:
        break;
    }

    NSSize sz = [openGLView frame].size;
    NSPoint pt = [theEvent locationInWindow];
    e.mclickEvent.x = pt.x;
    e.mclickEvent.y = sz.height - pt.y;
    window->dispatcher->PostEvent(e);
}

void WindowOsXObjcBridge::MouseMove(NSEvent* theEvent)
{
    DispatcherEvent e;
    e.window = window->window;
    e.type = DispatcherEvent::MOUSE_MOVE;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();

    NSSize sz = [openGLView frame].size;
    NSPoint pt = theEvent.locationInWindow;
    e.mmoveEvent.x = pt.x;
    e.mmoveEvent.y = sz.height - pt.y;
    window->dispatcher->PostEvent(e);
}

void WindowOsXObjcBridge::MouseWheel(NSEvent* theEvent)
{
    // detect the wheel event device
    // http://stackoverflow.com/questions/13807616/mac-cocoa-how-to-differentiate-if-a-nsscrollwheel-event-is-from-a-mouse-or-trac
    if (NSEventPhaseNone != [theEvent momentumPhase] || NSEventPhaseNone != [theEvent phase])
    {
        // TODO: add support for mouse/touch in DispatcherEvent
        //event.device = DAVA::UIEvent::Device::TOUCH_PAD;
    }
    else
    {
        //event.device = DAVA::UIEvent::Device::MOUSE;
    }

    const float32 scrollK = 10.0f;
    float32 deltaX = [theEvent scrollingDeltaX];
    float32 deltaY = [theEvent scrollingDeltaY];

    DispatcherEvent e;
    e.window = window->window;
    e.type = DispatcherEvent::MOUSE_WHEEL;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();

    if ([theEvent hasPreciseScrollingDeltas] == YES)
    {
        // touchpad or other precise device sends integer values (-3, -1, 0, 1, 40, etc)
        e.mwheelEvent.x = deltaX / scrollK;
        e.mwheelEvent.y = deltaY / scrollK;
    }
    else
    {
        // mouse sends float values from 0.1 for one wheel tick
        e.mwheelEvent.x = deltaX * scrollK;
        e.mwheelEvent.y = deltaY * scrollK;
    }

    window->dispatcher->PostEvent(e);
}

void WindowOsXObjcBridge::KeyEvent(NSEvent* theEvent)
{
    DispatcherEvent e;
    e.window = window->window;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.type = [theEvent type] == NSKeyDown ? DispatcherEvent::KEY_DOWN : DispatcherEvent::KEY_UP;
    e.keyEvent.key = [theEvent keyCode];
    e.keyEvent.isRepeated = [theEvent isARepeat];
    window->dispatcher->PostEvent(e);

    if ([theEvent type] == NSKeyDown)
    {
        e.type = DispatcherEvent::KEY_CHAR;

        NSString* chars = [theEvent characters];
        NSUInteger n = [chars length];
        for (NSUInteger i = 0; i < n; ++i)
        {
            e.keyEvent.key = [chars characterAtIndex:i];
            window->dispatcher->PostEvent(e);
        }
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__