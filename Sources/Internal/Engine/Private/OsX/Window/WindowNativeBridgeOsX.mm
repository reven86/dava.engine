#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/Window/WindowNativeBridgeOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSWindow.h>
#import <AppKit/NSScreen.h>

#include "Engine/Window.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/OsX/Window/WindowBackendOsX.h"
#include "Engine/Private/OsX/Window/RenderViewOsX.h"
#include "Engine/Private/OsX/Window/WindowDelegateOsX.h"

#include "Platform/SystemTimer.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace Private
{
WindowNativeBridge::WindowNativeBridge(WindowBackend* windowBackend)
    : windowBackend(*windowBackend)
{
}

WindowNativeBridge::~WindowNativeBridge() = default;

bool WindowNativeBridge::CreateWindow(float32 x, float32 y, float32 width, float32 height)
{
    // clang-format off
    NSUInteger style = NSTitledWindowMask |
                       NSMiniaturizableWindowMask |
                       NSClosableWindowMask |
                       NSResizableWindowMask;
    // clang-format on

    NSRect viewRect = NSMakeRect(x, y, width, height);
    windowDelegate = [[WindowDelegate alloc] initWithBridge:this];
    renderView = [[RenderView alloc] initWithFrame:viewRect andBridge:this];

    nswindow = [[NSWindow alloc] initWithContentRect:viewRect
                                           styleMask:style
                                             backing:NSBackingStoreBuffered
                                               defer:NO];
    [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [nswindow setContentView:renderView];
    [nswindow setDelegate:windowDelegate];

    {
        float32 scale = [nswindow backingScaleFactor];

        windowBackend.PostWindowCreated(viewRect.size.width, viewRect.size.height, scale, scale);
        windowBackend.PostVisibilityChanged(true);
    }

    [nswindow makeKeyAndOrderFront:nil];
    return true;
}

void WindowNativeBridge::ResizeWindow(float32 width, float32 height)
{
    [nswindow setContentSize:NSMakeSize(width, height)];
}

void WindowNativeBridge::CloseWindow()
{
    [nswindow close];
}

void WindowNativeBridge::TriggerPlatformEvents()
{
    dispatch_async(dispatch_get_main_queue(), [this]() {
        windowBackend.ProcessPlatformEvents();
    });
}

void WindowNativeBridge::ApplicationDidHideUnhide(bool hidden)
{
    isAppHidden = hidden;
}

void WindowNativeBridge::WindowDidMiniaturize()
{
    isMiniaturized = true;
    windowBackend.PostVisibilityChanged(false);
}

void WindowNativeBridge::WindowDidDeminiaturize()
{
    isMiniaturized = false;
}

void WindowNativeBridge::WindowDidBecomeKey()
{
    if (isMiniaturized || isAppHidden)
    {
        windowBackend.PostVisibilityChanged(true);
    }
    windowBackend.PostFocusChanged(true);
}

void WindowNativeBridge::WindowDidResignKey()
{
    windowBackend.PostFocusChanged(false);
    if (isAppHidden)
    {
        windowBackend.PostVisibilityChanged(false);
    }
}

void WindowNativeBridge::WindowDidResize()
{
    float32 scale = [nswindow backingScaleFactor];
    CGSize size = [renderView frame].size;

    windowBackend.PostSizeChanged(size.width, size.height, scale, scale);
}

void WindowNativeBridge::WindowDidChangeScreen()
{
}

bool WindowNativeBridge::WindowShouldClose()
{
    if (!windowBackend.closeRequestByApp)
    {
        windowBackend.PostUserCloseRequest();
        return false;
    }
    return true;
}

void WindowNativeBridge::WindowWillClose()
{
    windowBackend.WindowWillClose();
    windowBackend.DispatchWindowDestroyed(false);

    [nswindow setContentView:nil];
    [nswindow setDelegate:nil];

    [renderView release];
    [windowDelegate release];
}

void WindowNativeBridge::MouseClick(NSEvent* theEvent)
{
    MainDispatcherEvent e(windowBackend.window);
    e.mclickEvent.clicks = 1;
    e.mclickEvent.button = [theEvent buttonNumber] + 1;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();

    switch ([theEvent type])
    {
    case NSLeftMouseDown:
    case NSRightMouseDown:
    case NSOtherMouseDown:
        e.type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
        break;
    case NSLeftMouseUp:
    case NSRightMouseUp:
    case NSOtherMouseUp:
        e.type = MainDispatcherEvent::MOUSE_BUTTON_UP;
        break;
    default:
        break;
    }

    NSSize sz = [renderView frame].size;
    NSPoint pt = [theEvent locationInWindow];
    e.mclickEvent.x = pt.x;
    e.mclickEvent.y = sz.height - pt.y;
    windowBackend.mainDispatcher.PostEvent(e);
}

void WindowNativeBridge::MouseMove(NSEvent* theEvent)
{
    NSSize sz = [renderView frame].size;
    NSPoint pt = theEvent.locationInWindow;

    float32 x = pt.x;
    float32 y = sz.height - pt.y;
    windowBackend.PostMouseMove(x, y);
}

void WindowNativeBridge::MouseWheel(NSEvent* theEvent)
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

    NSSize sz = [renderView frame].size;
    NSPoint pt = theEvent.locationInWindow;

    float32 x = pt.x;
    float32 y = sz.height - pt.y;
    float32 deltaX = [theEvent scrollingDeltaX];
    float32 deltaY = [theEvent scrollingDeltaY];
    if ([theEvent hasPreciseScrollingDeltas] == YES)
    {
        // touchpad or other precise device sends integer values (-3, -1, 0, 1, 40, etc)
        deltaX /= scrollK;
        deltaY /= scrollK;
    }
    else
    {
        // mouse sends float values from 0.1 for one wheel tick
        deltaX *= scrollK;
        deltaY *= scrollK;
    }
    windowBackend.PostMouseWheel(x, y, deltaX, deltaY);
}

void WindowNativeBridge::KeyEvent(NSEvent* theEvent)
{
    uint32 key = [theEvent keyCode];
    bool isRepeated = [theEvent isARepeat];
    if ([theEvent type] == NSKeyDown)
    {
        windowBackend.PostKeyDown(key, isRepeated);
    }
    else
    {
        windowBackend.PostKeyUp(key);
    }

    if ([theEvent type] == NSKeyDown)
    {
        NSString* chars = [theEvent characters];
        for (NSUInteger i = 0, n = [chars length]; i < n; ++i)
        {
            uint32 key = [chars characterAtIndex:i];
            windowBackend.PostKeyChar(key, false);
        }
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
