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
WindowNativeBridge::WindowNativeBridge(WindowBackend* wbackend)
    : windowBackend(wbackend)
{
}

WindowNativeBridge::~WindowNativeBridge() = default;

bool WindowNativeBridge::DoCreateWindow(float32 x, float32 y, float32 width, float32 height)
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
    CreateOrUpdateTrackArea();

    {
        float32 scale = [nswindow backingScaleFactor];

        windowBackend->GetWindow()->PostWindowCreated(windowBackend, viewRect.size.width, viewRect.size.height, scale, scale);
        windowBackend->GetWindow()->PostVisibilityChanged(true);
    }

    [nswindow makeKeyAndOrderFront:nil];
    return true;
}

void WindowNativeBridge::CreateOrUpdateTrackArea()
{
    int areaOptions = (NSTrackingMouseEnteredAndExited | NSTrackingActiveInActiveApp);
    if (trackingArea != nullptr)
    {
        [renderView removeTrackingArea:trackingArea];
    }
    trackingArea = [[NSTrackingArea alloc] initWithRect:[renderView bounds] options:areaOptions owner:renderView userInfo:nil];
    [renderView addTrackingArea:trackingArea];
}

void WindowNativeBridge::DoResizeWindow(float32 width, float32 height)
{
    [nswindow setContentSize:NSMakeSize(width, height)];
}

void WindowNativeBridge::DoCloseWindow()
{
    [nswindow close];
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
    windowBackend->GetWindow()->PostVisibilityChanged(false);
}

void WindowNativeBridge::WindowDidDeminiaturize()
{
    isMiniaturized = false;
}

void WindowNativeBridge::WindowDidBecomeKey()
{
    hasFocus = true;
    if (isMiniaturized || isAppHidden)
    {
        windowBackend->GetWindow()->PostVisibilityChanged(true);
    }
    windowBackend->GetWindow()->PostFocusChanged(true);
}

void WindowNativeBridge::WindowDidResignKey()
{
    hasFocus = false;
    focusChanged = true;
    if (eMouseMode::PINING == nativeMouseMode)
    {
        SetMouseVisibility(true);
        SetMouseCaptured(false);
        deferredMouseMode = true;
    }
    else if (eMouseMode::HIDE == nativeMouseMode)
    {
        SetMouseVisibility(true);
        deferredMouseMode = true;
    }
    windowBackend->GetWindow()->PostFocusChanged(false);
    if (isAppHidden)
    {
        windowBackend->GetWindow()->PostVisibilityChanged(false);
    }
}

void WindowNativeBridge::WindowDidResize()
{
    float32 scale = [nswindow backingScaleFactor];
    CGSize size = [renderView frame].size;
    CreateOrUpdateTrackArea();
    windowBackend->GetWindow()->PostSizeChanged(size.width, size.height, scale, scale);
}

void WindowNativeBridge::WindowDidChangeScreen()
{
}

bool WindowNativeBridge::WindowShouldClose()
{
    return true;
}

void WindowNativeBridge::WindowWillClose()
{
    windowBackend->GetWindow()->PostWindowDestroyed();

    [nswindow setContentView:nil];
    [nswindow setDelegate:nil];

    [renderView release];
    [windowDelegate release];
}

void WindowNativeBridge::MouseClick(NSEvent* theEvent)
{
    MainDispatcherEvent e;
    e.window = windowBackend->window;
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
    if (!DeferredMouseMode(e))
    {
        windowBackend->GetDispatcher()->PostEvent(e);
    }
}

void WindowNativeBridge::MouseMove(NSEvent* theEvent)
{
    if (skipMouseMoveEvents)
    {
        skipMouseMoveEvents--;
        return;
    }

    MainDispatcherEvent e;
    e.window = windowBackend->window;
    e.type = MainDispatcherEvent::MOUSE_MOVE;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();

    NSSize sz = [renderView frame].size;
    NSPoint pt = theEvent.locationInWindow;
    e.mmoveEvent.x = pt.x;
    e.mmoveEvent.y = sz.height - pt.y;
    if (!DeferredMouseMode(e))
    {
        windowBackend->GetDispatcher()->PostEvent(e);
    }
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
    float32 deltaX = [theEvent scrollingDeltaX];
    float32 deltaY = [theEvent scrollingDeltaY];

    MainDispatcherEvent e;
    e.window = windowBackend->window;
    e.type = MainDispatcherEvent::MOUSE_WHEEL;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();

    NSSize sz = [renderView frame].size;
    NSPoint pt = theEvent.locationInWindow;
    e.mwheelEvent.x = pt.x;
    e.mwheelEvent.y = sz.height - pt.y;

    if ([theEvent hasPreciseScrollingDeltas] == YES)
    {
        // touchpad or other precise device sends integer values (-3, -1, 0, 1, 40, etc)
        e.mwheelEvent.deltaX = deltaX / scrollK;
        e.mwheelEvent.deltaY = deltaY / scrollK;
    }
    else
    {
        // mouse sends float values from 0.1 for one wheel tick
        e.mwheelEvent.deltaX = deltaX * scrollK;
        e.mwheelEvent.deltaY = deltaY * scrollK;
    }

    if (!DeferredMouseMode(e))
    {
        windowBackend->GetDispatcher()->PostEvent(e);
    }
}

void WindowNativeBridge::KeyEvent(NSEvent* theEvent)
{
    MainDispatcherEvent e;
    e.window = windowBackend->window;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.type = [theEvent type] == NSKeyDown ? MainDispatcherEvent::KEY_DOWN : MainDispatcherEvent::KEY_UP;
    e.keyEvent.key = [theEvent keyCode];
    e.keyEvent.isRepeated = [theEvent isARepeat];
    windowBackend->dispatcher->PostEvent(e);

    if ([theEvent type] == NSKeyDown)
    {
        e.type = MainDispatcherEvent::KEY_CHAR;

        NSString* chars = [theEvent characters];
        NSUInteger n = [chars length];
        for (NSUInteger i = 0; i < n; ++i)
        {
            e.keyEvent.key = [chars characterAtIndex:i];
            if (!DeferredMouseMode(e))
            {
                windowBackend->GetDispatcher()->PostEvent(e);
            }
        }
    }
}

void WindowNativeBridge::MouseEntered(NSEvent* theEvent)
{
    if (nativeMouseMode == eMouseMode::HIDE)
    {
        SetMouseVisibility(false);
        deferredMouseMode = false;
    }
}

void WindowNativeBridge::MouseExited(NSEvent* theEvent)
{
    if (nativeMouseMode == eMouseMode::HIDE)
    {
        SetMouseVisibility(true);
        deferredMouseMode = false;
    }
}

void WindowNativeBridge::DoChangeMouseMode(eMouseMode newMode)
{
    nativeMouseMode = newMode;
    deferredMouseMode = false;
    switch (newMode)
    {
    case DAVA::eMouseMode::FRAME:
        //not implemented
        SetMouseCaptured(false);
        SetMouseVisibility(true);
        break;
    case DAVA::eMouseMode::PINING:
    {
        if (hasFocus && !focusChanged)
        {
            SetMouseCaptured(true);
            SetMouseVisibility(false);
        }
        else
        {
            deferredMouseMode = true;
        }
        break;
    }
    case DAVA::eMouseMode::DEFAULT:
    {
        SetMouseCaptured(false);
        SetMouseVisibility(true);
        break;
    }
    case DAVA::eMouseMode::HIDE:
    {
        SetMouseCaptured(false);
        SetMouseVisibility(false);
        break;
    }
    }
}

void WindowNativeBridge::SetMouseVisibility(bool visible)
{
    if (mouseVisibled == visible)
    {
        return;
    }
    mouseVisibled = visible;
    if (visible)
    {
        [[NSCursor arrowCursor] set];
    }
    else
    {
        [static_cast<NSCursor*>(GetOrCreateBlankCursor()) set];
    }
}

void WindowNativeBridge::SetMouseCaptured(bool capture)
{
    if (mouseCaptured == capture)
    {
        return;
    }
    mouseCaptured = capture;
    if (capture)
    {
        // mouse capture
        CGAssociateMouseAndMouseCursorPosition(false);
        // set cursor in window center
        NSRect windowRect = [nswindow frame];
        NSRect screenRect = [[NSScreen mainScreen] frame];
        // Window origin is at bottom-left edge, but CGWarpMouseCursorPosition requires point in screen coordinates
        windowRect.origin.y = screenRect.size.height - (windowRect.origin.y + windowRect.size.height);
        float x = windowRect.origin.x + windowRect.size.width / 2.0f;
        float y = windowRect.origin.y + windowRect.size.height / 2.0f;
        CGWarpMouseCursorPosition(CGPointMake(x, y));
    }
    else
    {
        CGAssociateMouseAndMouseCursorPosition(true);
    }
}

bool WindowNativeBridge::DeferredMouseMode(const MainDispatcherEvent& e)
{
    if (!hasFocus)
    {
        return true;
    }
    focusChanged = false;
    if (deferredMouseMode)
    {
        if (MainDispatcherEvent::MOUSE_MOVE != e.type && MainDispatcherEvent::MOUSE_BUTTON_UP != e.type && MainDispatcherEvent::MOUSE_BUTTON_DOWN != e.type)
        {
            deferredMouseMode = false;
            SetMouseVisibility(false);
            if (eMouseMode::PINING == nativeMouseMode)
            {
                SetMouseCaptured(true);
                skipMouseMoveEvents = SKIP_N_MOUSE_MOVE_EVENTS;
            }
            return false;
        }
        else if (MainDispatcherEvent::MOUSE_BUTTON_UP == e.type)
        {
            // mouseUp event in work area, turn on mouse capture mode
            bool mclickInRect = true;
            mclickInRect &= (e.mclickEvent.x >= 0.f && e.mclickEvent.x <= windowBackend->GetWindow()->GetWidth());
            mclickInRect &= (e.mclickEvent.y >= 0.f && e.mclickEvent.y <= windowBackend->GetWindow()->GetHeight());
            if (mclickInRect && hasFocus)
            {
                deferredMouseMode = false;
                SetMouseVisibility(false);
                if (eMouseMode::PINNING == nativeMouseMode)
                {
                    SetMouseCaptured(true);
                    skipMouseMoveEvents = SKIP_N_MOUSE_MOVE_EVENTS;
                }
                // skip this event
            }
        }
        return true;
    }
    return false;
}

void* WindowNativeBridge::GetOrCreateBlankCursor()
{
    if (blankCursor != nullptr)
    {
        return blankCursor;
    }

    // Image data -> CGDataProviderRef
    const size_t width = 1;
    const size_t height = 1;
    uint32 pixel = 0;
    CGDataProviderRef dataProvider = CGDataProviderCreateWithData(nullptr, &pixel, sizeof(pixel), nullptr);

    // CGDataProviderRef -> CGImageRef
    const size_t bitsPerComponent = 8;
    const size_t bitsPerPixel = 32;
    const size_t bytesPerRow = width * 4;
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGImageAlphaLast | kCGBitmapByteOrder32Host;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    CGImageRef imageRef = CGImageCreate(width,
                                        height,
                                        bitsPerComponent,
                                        bitsPerPixel,
                                        bytesPerRow,
                                        colorSpace,
                                        bitmapInfo,
                                        dataProvider,
                                        nullptr,
                                        false,
                                        renderingIntent);

    // CGImageRef -> NSImage
    NSImage* img = [[NSImage alloc] initWithCGImage:imageRef size:NSMakeSize(width, height)];

    // NSImage -> NSCursor
    NSCursor* cursor = [[NSCursor alloc] initWithImage:img hotSpot:NSMakePoint(0, 0)];

    CFRelease(dataProvider);
    CFRelease(colorSpace);
    CGImageRelease(imageRef);
    [img release];

    blankCursor = cursor;
    return blankCursor;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
