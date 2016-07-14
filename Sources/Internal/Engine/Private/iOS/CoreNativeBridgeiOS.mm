#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/CoreNativeBridgeiOS.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/iOS/PlatformCoreiOS.h"
#include "Engine/Private/iOS/Window/WindowBackendiOS.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

#import <UIKit/UIKit.h>

// Wrapper over CADisplayLink to connect Objective-C's CADisplayLink object to
// C++ class CoreNativeBridgeiOS
@interface FrameTimer : NSObject
{
    DAVA::Private::CoreNativeBridgeiOS* bridge;
    CADisplayLink* displayLink;
    DAVA::int32 curInterval;
}

- (id)init:(DAVA::Private::CoreNativeBridgeiOS*)nativeBridge;
- (void)set:(DAVA::int32)interval;
- (void)cancel;
- (void)timerFired:(CADisplayLink*)dispLink;

@end

@implementation FrameTimer

- (id)init:(DAVA::Private::CoreNativeBridgeiOS*)nativeBridge
{
    self = [super init];
    if (self != nil)
    {
        bridge = nativeBridge;
        curInterval = 1;
        displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(timerFired:)];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    }
    return self;
}

- (void)set:(DAVA::int32)interval
{
    if (interval <= 0)
    {
        interval = 1;
    }
    if (curInterval != interval)
    {
        [displayLink setFrameInterval:interval];
        curInterval = interval;
    }
}

- (void)cancel
{
    [displayLink invalidate];
}

- (void)timerFired:(CADisplayLink*)dispLink
{
    bridge->OnFrameTimer();
}

@end

namespace DAVA
{
namespace Private
{
// UIApplicationMain instantiates UIApplicationDelegate-derived class, user cannot
// create UIApplicationDelegate-derived class, pass init parameters and set it to UIApplication
// AppDelegateiOS will receive pointer to CoreNativeBridgeiOS instance through nativeBridgeiOS
CoreNativeBridgeiOS* nativeBridgeiOS = nullptr;

CoreNativeBridgeiOS::CoreNativeBridgeiOS(PlatformCore* c)
    : core(c)
{
    nativeBridgeiOS = this;
}

CoreNativeBridgeiOS::~CoreNativeBridgeiOS() = default;

void CoreNativeBridgeiOS::Run()
{
    ::UIApplicationMain(0, nil, nil, @"AppDelegateiOS");
}

void CoreNativeBridgeiOS::OnFrameTimer()
{
    int32 fps = core->OnFrame();
    if (fps <= 0)
    {
        fps = std::numeric_limits<int32>::max();
    }

    int32 interval = static_cast<int32>(60.0 / fps + 0.5);
    [frameTimer set:interval];
}

bool CoreNativeBridgeiOS::applicationWillFinishLaunchingWithOptions(NSDictionary* launchOptions)
{
    Logger::Debug("******** applicationWillFinishLaunchingWithOptions");
    return true;
}

bool CoreNativeBridgeiOS::applicationDidFinishLaunchingWithOptions(NSDictionary* launchOptions)
{
    Logger::Debug("******** applicationDidFinishLaunchingWithOptions");

    core->engineBackend->OnGameLoopStarted();
    core->CreateNativeWindow(core->engineBackend->GetPrimaryWindow(), 640.0f, 480.0f);

    frameTimer = [[FrameTimer alloc] init:this];
    [frameTimer set:1];
    return true;
}

void CoreNativeBridgeiOS::applicationDidBecomeActive()
{
    Logger::Debug("******** applicationDidBecomeActive");

    core->didBecomeResignActive.Emit(true);
}

void CoreNativeBridgeiOS::applicationWillResignActive()
{
    Logger::Debug("******** applicationWillResignActive");

    core->didBecomeResignActive.Emit(false);
}

void CoreNativeBridgeiOS::applicationDidEnterBackground()
{
    core->didEnterForegroundBackground.Emit(false);

    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::APP_SUSPENDED;
    core->dispatcher->SendEvent(e); // Blocking call !!!
}

void CoreNativeBridgeiOS::applicationWillEnterForeground()
{
    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::APP_RESUMED;
    core->dispatcher->PostEvent(e);

    core->didEnterForegroundBackground.Emit(true);
}

void CoreNativeBridgeiOS::applicationWillTerminate()
{
    Logger::Debug("******** applicationWillTerminate");

    [frameTimer cancel];

    core->engineBackend->OnGameLoopStopped();
    core->engineBackend->OnBeforeTerminate();
}

void CoreNativeBridgeiOS::applicationDidReceiveMemoryWarning()
{
    Logger::Debug("******** applicationDidReceiveMemoryWarning");
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
