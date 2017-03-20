#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/Window/WindowNativeBridgeiOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Window.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/iOS/Window/WindowBackendiOS.h"

#include "Time/SystemTimer.h"
#include "Logger/Logger.h"

#include "Render/RHI/rhi_Public.h"

#import <sys/utsname.h>
#import <UIKit/UIKit.h>
#import "Engine/Private/iOS/Window/RenderViewiOS.h"
#import "Engine/Private/iOS/Window/RenderViewControlleriOS.h"
#import "Engine/Private/iOS/Window/NativeViewPooliOS.h"
#import "Engine/Private/iOS/Window/VisibleFrameObserver.h"
#import "DeviceManager/Private/Ios/DeviceManagerImplIos.h"

// Objective-C class used for interoperation between Objective-C and C++.
@interface ObjectiveCInteropWindow : NSObject
{
    DAVA::Private::WindowNativeBridge* bridge;
}

- (id)init:(DAVA::Private::WindowNativeBridge*)windowBridge;
- (void)processPlatformEvents;

@end

@implementation ObjectiveCInteropWindow

- (id)init:(DAVA::Private::WindowNativeBridge*)windowBridge
{
    self = [super init];
    if (self != nil)
    {
        bridge = windowBridge;
    }
    return self;
}

- (void)processPlatformEvents
{
    bridge->windowBackend->ProcessPlatformEvents();
}

@end

namespace DAVA
{
namespace Private
{
WindowNativeBridge::WindowNativeBridge(WindowBackend* windowBackend, const KeyedArchive* options)
    : windowBackend(windowBackend)
    , window(windowBackend->window)
    , mainDispatcher(windowBackend->mainDispatcher)
    , engineOptions(options)
{
    objcInterop = [[ObjectiveCInteropWindow alloc] init:this];
    visibleFrameObserver = [[VisibleFrameObserver alloc] initWithBridge:this];
}

WindowNativeBridge::~WindowNativeBridge()
{
    [visibleFrameObserver release];
    [objcInterop release];
}

void* WindowNativeBridge::GetHandle() const
{
    return [renderView layer];
}

bool WindowNativeBridge::CreateWindow()
{
    windowBackend->uiDispatcher.LinkToCurrentThread();

    ::UIScreen* screen = [ ::UIScreen mainScreen];
    CGRect rect = [screen bounds];
    CGFloat scale = [screen scale];

    uiwindow = [[UIWindow alloc] initWithFrame:rect];
    [uiwindow makeKeyAndVisible];

    renderViewController = [[RenderViewController alloc] initWithBridge:this];

    if (engineOptions->GetInt32("renderer", rhi::RHI_GLES2) == rhi::RHI_METAL)
        renderView = [[RenderViewMetal alloc] initWithFrame:rect andBridge:this];
    else
        renderView = [[RenderViewGL alloc] initWithFrame:rect andBridge:this];

    [renderView setContentScaleFactor:scale];

    nativeViewPool = [[NativeViewPool alloc] init];

    [uiwindow setRootViewController:renderViewController];

    CGRect viewRect = [renderView bounds];
    dpi = Private::DeviceManagerImpl::GetIPhoneMainScreenDpi();
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCreatedEvent(window, viewRect.size.width, viewRect.size.height, viewRect.size.width * scale, viewRect.size.height * scale, dpi, eFullscreen::On));
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, true));
    return true;
}

void WindowNativeBridge::TriggerPlatformEvents()
{
    // Use performSelectorOnMainThread instead of dispatch_async as modal dialog does not respond if
    // it is shown inside UIDispatcher handler
    [objcInterop performSelectorOnMainThread:@selector(processPlatformEvents)
                                  withObject:nil
                               waitUntilDone:NO];
}

void WindowNativeBridge::ApplicationDidBecomeOrResignActive(bool becomeActive)
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, becomeActive));
}

void WindowNativeBridge::ApplicationDidEnterForegroundOrBackground(bool foreground)
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, foreground));
}

void WindowNativeBridge::AddUIView(UIView* uiview)
{
    [renderView addSubview:uiview];
}

void WindowNativeBridge::RemoveUIView(UIView* uiview)
{
    [uiview removeFromSuperview];
}

UIView* WindowNativeBridge::GetUIViewFromPool(const char8* className)
{
    UIView* view = [nativeViewPool queryView:[NSString stringWithUTF8String:className]];
    [renderView addSubview:view];
    [view setHidden:YES];
    return view;
}

void WindowNativeBridge::ReturnUIViewToPool(UIView* view)
{
    [view setHidden:YES];
    [view removeFromSuperview];
    [nativeViewPool returnView:view];
}

void WindowNativeBridge::LoadView()
{
    [renderViewController setView:renderView];
}

void WindowNativeBridge::ViewWillTransitionToSize(float32 w, float32 h)
{
    // viewWillTransitionToSize can be called when device orientation changes
    // In some cases this won't lead to actual size changes
    // (i.e. when rotating from Landscape Left to Landscape Right)
    // In these cases we don't want to post SizeChanged event
    const CGSize currentSize = [renderView frame].size;
    if (FLOAT_EQUAL(currentSize.width, w) && FLOAT_EQUAL(currentSize.height, h))
    {
        return;
    }

    CGSize surfaceSize = [renderView surfaceSize];
    float32 surfaceScale = [renderView surfaceScale];
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, w, h, surfaceSize.width, surfaceSize.height, surfaceScale, dpi, eFullscreen::On));
}

void WindowNativeBridge::TouchesBegan(NSSet* touches)
{
    MainDispatcherEvent e = MainDispatcherEvent::CreateWindowTouchEvent(window, MainDispatcherEvent::TOUCH_DOWN, 0, 0, 0, eModifierKeys::NONE);
    for (UITouch* touch in touches)
    {
        CGPoint pt = [touch locationInView:touch.view];
        e.touchEvent.x = pt.x;
        e.touchEvent.y = pt.y;
        e.touchEvent.touchId = static_cast<uint32>(reinterpret_cast<uintptr_t>(touch));
        mainDispatcher->PostEvent(e);
    }
}

void WindowNativeBridge::TouchesMoved(NSSet* touches)
{
    MainDispatcherEvent e = MainDispatcherEvent::CreateWindowTouchEvent(window, MainDispatcherEvent::TOUCH_MOVE, 0, 0, 0, eModifierKeys::NONE);
    for (UITouch* touch in touches)
    {
        CGPoint pt = [touch locationInView:touch.view];
        e.touchEvent.x = pt.x;
        e.touchEvent.y = pt.y;
        e.touchEvent.touchId = static_cast<uint32>(reinterpret_cast<uintptr_t>(touch));
        mainDispatcher->PostEvent(e);
    }
}

void WindowNativeBridge::TouchesEnded(NSSet* touches)
{
    MainDispatcherEvent e = MainDispatcherEvent::CreateWindowTouchEvent(window, MainDispatcherEvent::TOUCH_UP, 0, 0, 0, eModifierKeys::NONE);
    for (UITouch* touch in touches)
    {
        CGPoint pt = [touch locationInView:touch.view];
        e.touchEvent.x = pt.x;
        e.touchEvent.y = pt.y;
        e.touchEvent.touchId = static_cast<uint32>(reinterpret_cast<uintptr_t>(touch));
        mainDispatcher->PostEvent(e);
    }
}

void WindowNativeBridge::SetSurfaceScale(const float32 scale)
{
    [renderView setSurfaceScale:scale];

    CGSize size = [renderView bounds].size;
    CGSize surfaceSize = [renderView surfaceSize];
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, size.width, size.height, surfaceSize.width, surfaceSize.height, scale, dpi, eFullscreen::On));
}

UIImage* RenderUIViewToImage(UIView* view)
{
    DVASSERT(view != nullptr);

    UIImage* image = nil;
    size_t w = view.frame.size.width;
    size_t h = view.frame.size.height;
    if (w > 0 && h > 0)
    {
        UIGraphicsBeginImageContextWithOptions(CGSizeMake(w, h), NO, 0);
        // Workaround! iOS bug see http://stackoverflow.com/questions/23157653/drawviewhierarchyinrectafterscreenupdates-delays-other-animations
        [view.layer renderInContext:UIGraphicsGetCurrentContext()];

        image = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
    }
    return image;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
