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
    : windowBackend(windowBackend)
    , window(windowBackend->window)
    , mainDispatcher(windowBackend->mainDispatcher)
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
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCreatedEvent(window, viewRect.size.width, viewRect.size.height, scale, scale));
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, true));
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

void WindowNativeBridge::SetTitle(const char8* title)
{
    NSString* nsTitle = [NSString stringWithUTF8String:title];
    [nswindow setTitle:nsTitle];
    [nsTitle release];
}

void WindowNativeBridge::TriggerPlatformEvents()
{
    dispatch_async(dispatch_get_main_queue(), [this]() {
        windowBackend->ProcessPlatformEvents();
    });
}

void WindowNativeBridge::ApplicationDidHideUnhide(bool hidden)
{
    isAppHidden = hidden;
}

void WindowNativeBridge::WindowDidMiniaturize()
{
    isMiniaturized = true;
    isVisible = false;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, isVisible));
}

void WindowNativeBridge::WindowDidDeminiaturize()
{
    isMiniaturized = false;
}

void WindowNativeBridge::WindowDidBecomeKey()
{
    if (isMiniaturized || isAppHidden || !isVisible)
    {
        isVisible = true;
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, isVisible));
    }
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, true));
}

void WindowNativeBridge::WindowDidResignKey()
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, false));
    if (isAppHidden)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, false));
    }
}

void WindowNativeBridge::WindowDidResize()
{
    float32 scale = [nswindow backingScaleFactor];
    CGSize size = [renderView frame].size;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, size.width, size.height, scale, scale));
}

void WindowNativeBridge::WindowDidChangeScreen()
{
}

bool WindowNativeBridge::WindowShouldClose()
{
    if (!windowBackend->closeRequestByApp)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateUserCloseRequestEvent(window));
        return false;
    }
    return true;
}

void WindowNativeBridge::WindowWillClose()
{
    windowBackend->WindowWillClose();
    mainDispatcher->SendEvent(MainDispatcherEvent::CreateWindowDestroyedEvent(window));

    [nswindow setContentView:nil];
    [nswindow setDelegate:nil];

    [renderView release];
    [windowDelegate release];
}

void WindowNativeBridge::MouseClick(NSEvent* theEvent)
{
    MainDispatcherEvent::eType type = MainDispatcherEvent::DUMMY;
    switch ([theEvent type])
    {
    case NSLeftMouseDown:
    case NSRightMouseDown:
    case NSOtherMouseDown:
        type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
        break;
    case NSLeftMouseUp:
    case NSRightMouseUp:
    case NSOtherMouseUp:
        type = MainDispatcherEvent::MOUSE_BUTTON_UP;
        break;
    default:
        return;
    }

    NSSize sz = [renderView frame].size;
    NSPoint pt = [theEvent locationInWindow];

    float32 x = pt.x;
    float32 y = sz.height - pt.y;
    uint32 button = [theEvent buttonNumber] + 1;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window, type, button, x, y, 1, false));
}

void WindowNativeBridge::MouseMove(NSEvent* theEvent)
{
    NSSize sz = [renderView frame].size;
    NSPoint pt = theEvent.locationInWindow;

    float32 x = pt.x;
    float32 y = sz.height - pt.y;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(window, x, y, false));
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
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseWheelEvent(window, x, y, deltaX, deltaY, false));
}

void WindowNativeBridge::KeyEvent(NSEvent* theEvent)
{
    uint32 key = [theEvent keyCode];
    bool isRepeated = [theEvent isARepeat];
    bool isPressed = [theEvent type] == NSKeyDown;

    MainDispatcherEvent::eType type = isPressed ? MainDispatcherEvent::KEY_DOWN : MainDispatcherEvent::KEY_UP;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, type, key, isRepeated));

    if ([theEvent type] == NSKeyDown)
    {
        NSString* chars = [theEvent characters];
        NSUInteger n = [chars length];
        if (n > 0)
        {
            MainDispatcherEvent e = MainDispatcherEvent::CreateWindowKeyPressEvent(window, MainDispatcherEvent::KEY_CHAR, 0, false);
            for (NSUInteger i = 0; i < n; ++i)
            {
                uint32 key = [chars characterAtIndex:i];
                e.keyEvent.key = key;
                mainDispatcher->PostEvent(e);
            }
        }
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
