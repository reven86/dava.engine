#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/Window/WindowNativeBridgeiOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Window.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/iOS/Window/WindowBackendiOS.h"

#include "Platform/SystemTimer.h"
#include "Logger/Logger.h"

#import <sys/utsname.h>
#import <UIKit/UIKit.h>
#import "Engine/Private/iOS/Window/RenderViewiOS.h"
#import "Engine/Private/iOS/Window/RenderViewControlleriOS.h"
#import "Engine/Private/iOS/Window/NativeViewPooliOS.h"

namespace DAVA
{
namespace Private
{

float32 GetDpi(CGRect rect, float32 scale)
{
    enum eIosDpi
    {
        IPHONE_3_IPAD_MINI = 163,
        IPHONE_4_5_6_SE_IPAD_MINI2_MINI3 = 326,
        IPAD_1_2 = 132,
        IPAD_3_4_AIR_AIR2_PRO = 264,
        IPHONE_6_PLUS = 401,
        IPHONE_6_PLUS_ZOOM = 461,
    };

    struct AppleDevice
    {
        int minSide;
        int dpi;
        const char* machineTag;
    };

    static AppleDevice listOfAppleDevices[] =
    {
      { 320, IPHONE_3_IPAD_MINI, "" },
      { 640, IPHONE_4_5_6_SE_IPAD_MINI2_MINI3, "" },
      { 750, IPHONE_4_5_6_SE_IPAD_MINI2_MINI3, "" },
      { 768, IPAD_1_2, "" },
      { 768, IPHONE_3_IPAD_MINI, "mini" },
      { 1080, IPHONE_6_PLUS, "" },
      { 1242, IPHONE_6_PLUS_ZOOM, "" },
      { 1536, IPAD_3_4_AIR_AIR2_PRO, "" },
      { 1536, IPHONE_4_5_6_SE_IPAD_MINI2_MINI3, "mini" },
      { 2048, IPAD_3_4_AIR_AIR2_PRO, "" }
    };

    float32 dpi = 160 * scale; // default dpi value
    float32 minSide = std::min(rect.size.width * scale, rect.size.height * scale);

    // find possible device with calculated side
    List<AppleDevice*> possibleDevices;
    for (size_t i = 0, sz = std::extent<decltype(listOfAppleDevices)>(); i < sz; ++i)
    {
        if (listOfAppleDevices[i].minSide == minSide)
        {
            possibleDevices.push_back(&listOfAppleDevices[i]);
        }
    }

    struct utsname systemInfo;
    uname(&systemInfo);

    String thisMachine = systemInfo.machine;

    // search real device from possibles
    AppleDevice* realDevice = nullptr;
    for (auto d : possibleDevices)
    {
        if (thisMachine.find(d->machineTag) != String::npos)
        {
            realDevice = d;
        }
    }

    // if found - use real device dpi
    if (nullptr != realDevice)
    {
        dpi = realDevice->dpi;
    }

    return dpi;
}

WindowNativeBridge::WindowNativeBridge(WindowBackend* windowBackend)
    : windowBackend(windowBackend)
    , window(windowBackend->window)
    , mainDispatcher(windowBackend->mainDispatcher)
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

    float32 dpi = GetDpi(rect, scale);
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCreatedEvent(window, rect.size.width, rect.size.height, rect.size.width * scale, rect.size.height * scale, dpi));
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, true));
    return true;
}

void WindowNativeBridge::TriggerPlatformEvents()
{
    dispatch_async(dispatch_get_main_queue(), [this]() {
        windowBackend->ProcessPlatformEvents();
    });
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
    float32 scale = [[ ::UIScreen mainScreen] scale];
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, w, h, w * scale, h * scale));
}

void WindowNativeBridge::TouchesBegan(NSSet* touches)
{
    MainDispatcherEvent e = MainDispatcherEvent::CreateWindowTouchEvent(window, MainDispatcherEvent::TOUCH_DOWN, 0, 0.f, 0.f);
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
    MainDispatcherEvent e = MainDispatcherEvent::CreateWindowTouchEvent(window, MainDispatcherEvent::TOUCH_MOVE, 0, 0.f, 0.f);
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
    MainDispatcherEvent e = MainDispatcherEvent::CreateWindowTouchEvent(window, MainDispatcherEvent::TOUCH_UP, 0, 0.f, 0.f);
    for (UITouch* touch in touches)
    {
        CGPoint pt = [touch locationInView:touch.view];
        e.touchEvent.x = pt.x;
        e.touchEvent.y = pt.y;
        e.touchEvent.touchId = static_cast<uint32>(reinterpret_cast<uintptr_t>(touch));
        mainDispatcher->PostEvent(e);
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
