#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/Window/WindowNativeBridgeiOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Window.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/iOS/Window/WindowBackendiOS.h"

#include "Platform/SystemTimer.h"
#include "Logger/Logger.h"

#import <UIKit/UIKit.h>
#import "Engine/Private/iOS/Window/RenderViewiOS.h"
#import "Engine/Private/iOS/Window/RenderViewControlleriOS.h"
#import "Engine/Private/iOS/Window/NativeViewPooliOS.h"

namespace DAVA
{
namespace Private
{
WindowNativeBridge::WindowNativeBridge(WindowBackend* windowBackend)
    : windowBackend(*windowBackend)
{
}

WindowNativeBridge::~WindowNativeBridge() = default;

void* WindowNativeBridge::GetHandle() const
{
    return [renderView layer];
}

bool WindowNativeBridge::CreateWindow()
{
    ::UIScreen* screen = [ ::UIScreen mainScreen];
    CGRect rect = [screen bounds];
    float32 scale = [screen scale];

    uiwindow = [[UIWindow alloc] initWithFrame:rect];
    [uiwindow makeKeyAndVisible];

    renderViewController = [[RenderViewController alloc] initWithBridge:this];
    renderView = [[RenderView alloc] initWithFrame:rect andBridge:this];
    [renderView setContentScaleFactor:scale];

    nativeViewPool = [[NativeViewPool alloc] init];

    [uiwindow setRootViewController:renderViewController];

    windowBackend.PostWindowCreated(rect.size.width, rect.size.height, scale, scale);
    windowBackend.PostVisibilityChanged(true);
    return true;
}

void WindowNativeBridge::TriggerPlatformEvents()
{
    dispatch_async(dispatch_get_main_queue(), [this]() {
        windowBackend.ProcessPlatformEvents();
    });
}

void WindowNativeBridge::ApplicationDidBecomeOrResignActive(bool becomeActive)
{
    windowBackend.PostFocusChanged(becomeActive);
}

void WindowNativeBridge::ApplicationDidEnterForegroundOrBackground(bool foreground)
{
    windowBackend.PostVisibilityChanged(foreground);
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
    float32 scale = [[ ::UIScreen mainScreen] scale];
    windowBackend.PostSizeChanged(w, h, scale, scale);
}

void WindowNativeBridge::TouchesBegan(NSSet* touches)
{
    for (UITouch* touch in touches)
    {
        CGPoint pt = [touch locationInView:touch.view];
        float32 x = pt.x;
        float32 y = pt.y;
        uint32 touchId = static_cast<uint32>(reinterpret_cast<uintptr_t>(touch));
        windowBackend.PostTouchDown(touchId, x, y);
    }
}

void WindowNativeBridge::TouchesMoved(NSSet* touches)
{
    for (UITouch* touch in touches)
    {
        CGPoint pt = [touch locationInView:touch.view];
        float32 x = pt.x;
        float32 y = pt.y;
        uint32 touchId = static_cast<uint32>(reinterpret_cast<uintptr_t>(touch));
        windowBackend.PostTouchMove(touchId, x, y);
    }
}

void WindowNativeBridge::TouchesEnded(NSSet* touches)
{
    for (UITouch* touch in touches)
    {
        CGPoint pt = [touch locationInView:touch.view];
        float32 x = pt.x;
        float32 y = pt.y;
        uint32 touchId = static_cast<uint32>(reinterpret_cast<uintptr_t>(touch));
        windowBackend.PostTouchUp(touchId, x, y);
    }
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
