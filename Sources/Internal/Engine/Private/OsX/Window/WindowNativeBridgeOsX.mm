#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/Window/WindowNativeBridgeOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSCursor.h>
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
    [nswindow setContentMinSize:NSMakeSize(128, 128)];

    {
        float32 dpi = GetDpi();
        CGSize surfaceSize = [renderView convertSizeToBacking:viewRect.size];
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCreatedEvent(window, viewRect.size.width, viewRect.size.height, surfaceSize.width, surfaceSize.height, dpi, eFullscreen::Off));
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, true));
    }

    [nswindow makeKeyAndOrderFront:nil];
    return true;
}

void WindowNativeBridge::ResizeWindow(float32 width, float32 height)
{
    NSRect r = [nswindow frame];

    float32 dx = (r.size.width - width) / 2.0;
    float32 dy = (r.size.height - height) / 2.0;

    NSPoint pos = NSMakePoint(r.origin.x + dx, r.origin.y + dy);
    NSSize sz = NSMakeSize(width, height);

    [nswindow setFrameOrigin:pos];
    [nswindow setContentSize:sz];
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

void WindowNativeBridge::SetMinimumSize(float32 width, float32 height)
{
    NSSize sz = NSMakeSize(width, height);
    [nswindow setContentMinSize:sz];
}

void WindowNativeBridge::SetFullscreen(eFullscreen newMode)
{
    bool isFullscreenRequested = newMode == eFullscreen::On;

    if (isFullscreen != isFullscreenRequested)
    {
        [nswindow toggleFullScreen:nil];

        if (isFullscreen)
        {
            // If we're entering fullscreen we want our app to also become focused
            // To handle cases when app is being opened with fullscreen mode,
            // but another app gets focus before our app's window is created,
            // thus ignoring any input afterwards
            [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
        }
    }
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
    isVisible = false;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, isVisible));
}

void WindowNativeBridge::WindowDidDeminiaturize()
{
}

void WindowNativeBridge::WindowDidBecomeKey()
{
    if (isAppHidden || !isVisible)
    {
        isVisible = true;
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, isVisible));
    }
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, true));
}

void WindowNativeBridge::WindowDidResignKey()
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, false));
    if (captureMode == eCursorCapture::PINNING)
    {
        SetCursorCapture(eCursorCapture::OFF);
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCaptureLostEvent(window));
    }
    SetCursorVisibility(true);
    if (isAppHidden)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, false));
    }
}

void WindowNativeBridge::HandleSizeChanging(bool dpiChanged)
{
    CGSize size = [renderView frame].size;
    CGSize surfSize = [renderView convertSizeToBacking:size];
    float32 surfaceScale = [renderView backbufferScale];
    eFullscreen fullscreen = isFullscreen ? eFullscreen::On : eFullscreen::Off;
    float32 dpi = GetDpi();

    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, size.width, size.height, surfSize.width, surfSize.height, surfaceScale, dpi, fullscreen));
    if (dpiChanged)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowDpiChangedEvent(window, dpi));
    }
}

void WindowNativeBridge::WindowDidResize()
{
    HandleSizeChanging(false);
}

void WindowNativeBridge::WindowWillStartLiveResize()
{
}

void WindowNativeBridge::WindowDidEndLiveResize()
{
    if (!isFullscreenToggling)
    {
        ForceBackbufferSizeUpdate();
    }
}

void WindowNativeBridge::WindowDidChangeScreen()
{
    HandleSizeChanging(true);
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

void WindowNativeBridge::WindowWillEnterFullScreen()
{
    isFullscreen = true;
    isFullscreenToggling = true;
}

void WindowNativeBridge::WindowDidEnterFullScreen()
{
    isFullscreenToggling = false;
}

void WindowNativeBridge::WindowWillExitFullScreen()
{
    isFullscreen = false;
    isFullscreenToggling = true;
}

void WindowNativeBridge::WindowDidExitFullScreen()
{
    isFullscreenToggling = false;
}

void WindowNativeBridge::MouseClick(NSEvent* theEvent)
{
    eMouseButtons button = GetMouseButton(theEvent);
    if (button != eMouseButtons::NONE)
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
        eModifierKeys modifierKeys = GetModifierKeys(theEvent);
        bool isRelative = (captureMode == eCursorCapture::PINNING);
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window, type, button, x, y, 1, modifierKeys, isRelative));
    }
}

void WindowNativeBridge::MouseMove(NSEvent* theEvent)
{
    if (mouseMoveSkipCount)
    {
        mouseMoveSkipCount--;
        return;
    }
    NSSize sz = [renderView frame].size;
    NSPoint pt = theEvent.locationInWindow;
    bool isRelative = (captureMode == eCursorCapture::PINNING);
    float32 x = pt.x;
    float32 y = sz.height - pt.y;

    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    if (isRelative)
    {
        x = [theEvent deltaX];
        y = [theEvent deltaY];
    }
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(window, x, y, modifierKeys, isRelative));
}

void WindowNativeBridge::MouseWheel(NSEvent* theEvent)
{
    static const float32 scrollK = 10.0f;

    NSSize sz = [renderView frame].size;
    NSPoint pt = theEvent.locationInWindow;

    float32 x = pt.x;
    float32 y = sz.height - pt.y;
    float32 wheelDeltaX = [theEvent scrollingDeltaX];
    float32 wheelDeltaY = [theEvent scrollingDeltaY];

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
        // Invert scroll directions back because MacOS do it by self when Shift pressed
        if (([theEvent modifierFlags] & NSShiftKeyMask) != 0)
        {
            std::swap(wheelDeltaX, wheelDeltaY);
        }
    }

    if ([theEvent hasPreciseScrollingDeltas] == YES)
    {
        // Touchpad or other precise device send integer values (-3, -1, 0, 1, 40, etc)
        wheelDeltaX /= scrollK;
        wheelDeltaY /= scrollK;
    }
    else
    {
        // Mouse sends float values from 0.1 for one wheel tick
        wheelDeltaX *= scrollK;
        wheelDeltaY *= scrollK;
    }
    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    bool isRelative = captureMode == eCursorCapture::PINNING;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseWheelEvent(window, x, y, wheelDeltaX, wheelDeltaY, modifierKeys, isRelative));
}

void WindowNativeBridge::KeyEvent(NSEvent* theEvent)
{
    uint32 key = [theEvent keyCode];
    bool isRepeated = [theEvent isARepeat];
    bool isPressed = [theEvent type] == NSKeyDown;

    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    MainDispatcherEvent::eType type = isPressed ? MainDispatcherEvent::KEY_DOWN : MainDispatcherEvent::KEY_UP;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, type, key, modifierKeys, isRepeated));

    // macOS translates some Ctrl key combinations into ASCII control characters.
    // It seems to me that control character are not wanted by game to handle in character message.
    if ([theEvent type] == NSKeyDown && (modifierKeys & eModifierKeys::CONTROL) == eModifierKeys::NONE)
    {
        NSString* chars = [theEvent characters];
        NSUInteger n = [chars length];
        if (n > 0)
        {
            MainDispatcherEvent e = MainDispatcherEvent::CreateWindowKeyPressEvent(window, MainDispatcherEvent::KEY_CHAR, 0, modifierKeys, false);
            for (NSUInteger i = 0; i < n; ++i)
            {
                uint32 key = [chars characterAtIndex:i];
                e.keyEvent.key = key;
                mainDispatcher->PostEvent(e);
            }
        }
    }
}

void WindowNativeBridge::FlagsChanged(NSEvent* theEvent)
{
    // Here we detect modifier key flags presses (Shift, Alt, Ctrl, Cmd, Capslock).
    // But Capslock is toggle key so we cannot determine it is pressed or unpressed
    // only is toggled and untoggled.

    static constexpr uint32 interestingFlags[] = {
        NX_DEVICELCTLKEYMASK,
        NX_DEVICERCTLKEYMASK,
        NX_DEVICELSHIFTKEYMASK,
        NX_DEVICERSHIFTKEYMASK,
        NX_DEVICELCMDKEYMASK,
        NX_DEVICERCMDKEYMASK,
        NX_DEVICELALTKEYMASK,
        NX_DEVICERALTKEYMASK,
        NX_ALPHASHIFTMASK, // Capslock
    };

    uint32 newModifierFlags = [theEvent modifierFlags];
    uint32 changedModifierFlags = newModifierFlags ^ lastModifierFlags;

    uint32 key = [theEvent keyCode];
    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    MainDispatcherEvent e = MainDispatcherEvent::CreateWindowKeyPressEvent(window, MainDispatcherEvent::KEY_DOWN, key, modifierKeys, false);
    for (uint32 flag : interestingFlags)
    {
        if (flag & changedModifierFlags)
        {
            bool isPressed = (flag & newModifierFlags) == flag;
            e.type = isPressed ? MainDispatcherEvent::KEY_DOWN : MainDispatcherEvent::KEY_UP;
            mainDispatcher->PostEvent(e);
        }
    }
    lastModifierFlags = newModifierFlags;
}

void WindowNativeBridge::MagnifyWithEvent(NSEvent* theEvent)
{
    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    float32 magnification = [theEvent magnification];
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMagnificationGestureEvent(window, magnification, modifierKeys));
}

void WindowNativeBridge::RotateWithEvent(NSEvent* theEvent)
{
    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    float32 rotation = [theEvent rotation];
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowRotationGestureEvent(window, rotation, modifierKeys));
}

void WindowNativeBridge::SwipeWithEvent(NSEvent* theEvent)
{
    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    float32 deltaX = [theEvent deltaX];
    float32 deltaY = [theEvent deltaY];
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSwipeGestureEvent(window, deltaX, deltaY, modifierKeys));
}

eModifierKeys WindowNativeBridge::GetModifierKeys(NSEvent* theEvent)
{
    // TODO: NSControlKeyMask, NSAlternateKeyMask, etc are deprecated in xcode 8 and replaced with NSEventModifierFlagControl, ...

    eModifierKeys result = eModifierKeys::NONE;
    NSEventModifierFlags flags = [theEvent modifierFlags];
    if (flags & NSShiftKeyMask)
    {
        result |= eModifierKeys::SHIFT;
    }
    if (flags & NSControlKeyMask)
    {
        result |= eModifierKeys::CONTROL;
    }
    if (flags & NSAlternateKeyMask)
    {
        result |= eModifierKeys::ALT;
    }
    if (flags & NSCommandKeyMask)
    {
        result |= eModifierKeys::COMMAND;
    }
    return result;
}

float32 WindowNativeBridge::GetDpi()
{
    NSScreen* screen = [NSScreen mainScreen];
    NSDictionary* description = [screen deviceDescription];
    NSSize displayPixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
    CGSize displayPhysicalSize = CGDisplayScreenSize([[description objectForKey:@"NSScreenNumber"] unsignedIntValue]);

    // there being 25.4 mm in an inch
    return (displayPixelSize.width / displayPhysicalSize.width) * 25.4f;
}

eMouseButtons WindowNativeBridge::GetMouseButton(NSEvent* theEvent)
{
    eMouseButtons result = static_cast<eMouseButtons>([theEvent buttonNumber] + 1);
    if (eMouseButtons::FIRST <= result && result <= eMouseButtons::LAST)
    {
        return result;
    }
    return eMouseButtons::NONE;
}

void WindowNativeBridge::MouseEntered(NSEvent* theEvent)
{
    cursorInside = true;
    UpdateSystemCursorVisible();
    if (eCursorCapture::PINNING == captureMode)
    {
        SetSystemCursorCapture(true);
    }
}

void WindowNativeBridge::MouseExited(NSEvent* theEvent)
{
    cursorInside = false;
    UpdateSystemCursorVisible();
    if (eCursorCapture::PINNING == captureMode)
    {
        SetSystemCursorCapture(false);
    }
}

void WindowNativeBridge::SetCursorCapture(eCursorCapture mode)
{
    if (captureMode != mode)
    {
        captureMode = mode;
        switch (mode)
        {
        case eCursorCapture::FRAME:
            //not implemented
            break;
        case eCursorCapture::PINNING:
        {
            SetSystemCursorCapture(true);
            break;
        }
        case eCursorCapture::OFF:
        {
            SetSystemCursorCapture(false);
            break;
        }
        }
    }
}

void WindowNativeBridge::SetSystemCursorCapture(bool capture)
{
    if (capture)
    {
        CGAssociateMouseAndMouseCursorPosition(false);
        // set cursor in window center
        NSRect windowRect = [nswindow frame];
        NSRect screenRect = [[NSScreen mainScreen] frame];
        // Window origin is at bottom-left edge, but CGWarpMouseCursorPosition requires point in screen coordinates
        windowRect.origin.y = screenRect.size.height - (windowRect.origin.y + windowRect.size.height);
        CGPoint cursorpos;
        cursorpos.x = windowRect.origin.x + windowRect.size.width / 2.0f;
        cursorpos.y = windowRect.origin.y + windowRect.size.height / 2.0f;
        CGWarpMouseCursorPosition(cursorpos);
        mouseMoveSkipCount = SKIP_N_MOUSE_MOVE_EVENTS;
    }
    else
    {
        CGAssociateMouseAndMouseCursorPosition(true);
    }
}

void WindowNativeBridge::UpdateSystemCursorVisible()
{
    static bool mouseVisibleState = true;

    bool visible = !cursorInside || mouseVisible;
    if (mouseVisibleState != visible)
    {
        mouseVisibleState = visible;
        if (visible)
        {
            CGDisplayShowCursor(kCGDirectMainDisplay);
        }
        else
        {
            CGDisplayHideCursor(kCGDirectMainDisplay);
        }
    }
}

void WindowNativeBridge::SetCursorVisibility(bool visible)
{
    if (mouseVisible != visible)
    {
        mouseVisible = visible;
        UpdateSystemCursorVisible();
    }
}

void WindowNativeBridge::SetSurfaceScale(const float32 scale)
{
    [renderView setBackbufferScale:scale];

    ForceBackbufferSizeUpdate();
    WindowDidResize();
}

void WindowNativeBridge::ForceBackbufferSizeUpdate()
{
    // Workaround to force change backbuffer size
    [nswindow setContentView:nil];
    [nswindow setContentView:renderView];
    [nswindow makeFirstResponder:renderView];
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
