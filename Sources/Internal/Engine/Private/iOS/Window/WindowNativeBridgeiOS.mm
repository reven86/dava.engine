#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/Window/WindowNativeBridgeiOS.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Engine/Public/Window.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/iOS/Window/WindowBackendiOS.h"

#include "Platform/SystemTimer.h"
#include "Logger/Logger.h"

#import <UIKit/UIKit.h>
#import "Engine/Private/iOS/Window/RenderViewiOS.h"
#import "Engine/Private/iOS/Window/RenderViewControlleriOS.h"

namespace DAVA
{
namespace Private
{
WindowNativeBridge::WindowNativeBridge(WindowBackend* wbackend)
    : windowBackend(wbackend)
{
}

WindowNativeBridge::~WindowNativeBridge() = default;

void* WindowNativeBridge::GetHandle() const
{
    return [renderView layer];
}

bool WindowNativeBridge::DoCreateWindow()
{
    UIScreen* screen = [UIScreen mainScreen];
    CGRect rect = [screen bounds];
    float32 scale = [screen scale];

    uiwindow = [[UIWindow alloc] initWithFrame:rect];
    [uiwindow makeKeyAndVisible];

    renderViewController = [[RenderViewController alloc] initWithBridge:this];
    renderView = [[RenderView alloc] initWithFrame:rect andBridge:this];
    [renderView setContentScaleFactor:scale];

    [uiwindow setRootViewController:renderViewController];

    windowBackend->GetWindow()->PostWindowCreated(windowBackend, rect.size.width, rect.size.height, scale, scale);
    windowBackend->GetWindow()->PostVisibilityChanged(true);
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
    windowBackend->GetWindow()->PostFocusChanged(becomeActive);
}

void WindowNativeBridge::ApplicationDidEnterForegroundOrBackground(bool foreground)
{
    windowBackend->GetWindow()->PostVisibilityChanged(foreground);
}

void WindowNativeBridge::loadView()
{
    [renderViewController setView:renderView];
}

void WindowNativeBridge::viewWillTransitionToSize(float32 w, float32 h)
{
    float32 scale = [[UIScreen mainScreen] scale];
    windowBackend->GetWindow()->PostSizeChanged(w, h, scale, scale);
}

void WindowNativeBridge::touchesBegan(NSSet* touches)
{
    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::TOUCH_DOWN;
    e.window = windowBackend->window;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();

    MainDispatcher* dispatcher = windowBackend->GetDispatcher();
    for (UITouch* touch in touches)
    {
        CGPoint pt = [touch locationInView:touch.view];
        e.tclickEvent.x = pt.x;
        e.tclickEvent.y = pt.y;
        e.tclickEvent.touchId = static_cast<uint32>(reinterpret_cast<uintptr_t>(touch));
        dispatcher->PostEvent(e);
    }
}

void WindowNativeBridge::touchesMoved(NSSet* touches)
{
    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::TOUCH_MOVE;
    e.window = windowBackend->window;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();

    MainDispatcher* dispatcher = windowBackend->GetDispatcher();
    for (UITouch* touch in touches)
    {
        CGPoint pt = [touch locationInView:touch.view];
        e.tmoveEvent.x = pt.x;
        e.tmoveEvent.y = pt.y;
        e.tmoveEvent.touchId = static_cast<uint32>(reinterpret_cast<uintptr_t>(touch));
        dispatcher->PostEvent(e);
    }
}

void WindowNativeBridge::touchesEnded(NSSet* touches)
{
    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::TOUCH_UP;
    e.window = windowBackend->window;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();

    MainDispatcher* dispatcher = windowBackend->GetDispatcher();
    for (UITouch* touch in touches)
    {
        CGPoint pt = [touch locationInView:touch.view];
        e.tclickEvent.x = pt.x;
        e.tclickEvent.y = pt.y;
        e.tclickEvent.touchId = static_cast<uint32>(reinterpret_cast<uintptr_t>(touch));
        dispatcher->PostEvent(e);
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
