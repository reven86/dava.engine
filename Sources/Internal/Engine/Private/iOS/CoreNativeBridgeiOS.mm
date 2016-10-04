#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/CoreNativeBridgeiOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/iOS/PlatformCoreiOS.h"
#include "Engine/Private/iOS/Window/WindowBackendiOS.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

#import <UIKit/UIKit.h>

// Wrapper over CADisplayLink to connect Objective-C's CADisplayLink object to
// C++ class CoreNativeBridge
@interface FrameTimer : NSObject
{
    DAVA::Private::CoreNativeBridge* bridge;
    CADisplayLink* displayLink;
    DAVA::int32 curInterval;
}

- (id)init:(DAVA::Private::CoreNativeBridge*)nativeBridge;
- (void)set:(DAVA::int32)interval;
- (void)cancel;
- (void)timerFired:(CADisplayLink*)dispLink;

@end

@implementation FrameTimer

- (id)init:(DAVA::Private::CoreNativeBridge*)nativeBridge
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
// UIApplicationMain instantiates UIApplicationDelegate-derived class and user cannot
// create UIApplicationDelegate-derived class, pass init parameters and set it to UIApplication.
// AppDelegate will receive pointer to CoreNativeBridge instance through nativeBridge global
// variable
CoreNativeBridge* coreNativeBridge = nullptr;

CoreNativeBridge::CoreNativeBridge(PlatformCore* core)
    : core(core)
    , engineBackend(core->engineBackend)
    , mainDispatcher(core->dispatcher)
{
    coreNativeBridge = this;
}

CoreNativeBridge::~CoreNativeBridge() = default;

void CoreNativeBridge::Run()
{
    @autoreleasepool
    {
        // UIApplicationMain never returns
        ::UIApplicationMain(0, nil, nil, @"AppDelegate");
    }
}

void CoreNativeBridge::OnFrameTimer()
{
    int32 fps = core->OnFrame();
    if (fps <= 0)
    {
        fps = std::numeric_limits<int32>::max();
    }

    int32 interval = static_cast<int32>(60.0 / fps + 0.5);
    [frameTimer set:interval];
}

bool CoreNativeBridge::ApplicationWillFinishLaunchingWithOptions(NSDictionary* launchOptions)
{
    Logger::FrameworkDebug("******** applicationWillFinishLaunchingWithOptions");
    return true;
}

bool CoreNativeBridge::ApplicationDidFinishLaunchingWithOptions(NSDictionary* launchOptions)
{
    Logger::FrameworkDebug("******** applicationDidFinishLaunchingWithOptions");

    engineBackend->OnGameLoopStarted();

    WindowBackend* primaryWindowBackend = PlatformCore::GetWindowBackend(engineBackend->GetPrimaryWindow());
    primaryWindowBackend->Create();

    frameTimer = [[FrameTimer alloc] init:this];
    [frameTimer set:1];
    return true;
}

void CoreNativeBridge::ApplicationDidBecomeActive()
{
    Logger::FrameworkDebug("******** applicationDidBecomeActive");

    core->didBecomeResignActive.Emit(true);
}

void CoreNativeBridge::ApplicationWillResignActive()
{
    Logger::FrameworkDebug("******** applicationWillResignActive");

    core->didBecomeResignActive.Emit(false);
}

void CoreNativeBridge::ApplicationDidEnterBackground()
{
    core->didEnterForegroundBackground.Emit(false);
    mainDispatcher->SendEvent(MainDispatcherEvent(MainDispatcherEvent::APP_SUSPENDED)); // Blocking call !!!
}

void CoreNativeBridge::ApplicationWillEnterForeground()
{
    mainDispatcher->PostEvent(MainDispatcherEvent(MainDispatcherEvent::APP_RESUMED));
    core->didEnterForegroundBackground.Emit(true);
}

void CoreNativeBridge::ApplicationWillTerminate()
{
    Logger::FrameworkDebug("******** applicationWillTerminate");

    [frameTimer cancel];

    engineBackend->OnGameLoopStopped();
    engineBackend->OnEngineCleanup();
}

void CoreNativeBridge::ApplicationDidReceiveMemoryWarning()
{
    Logger::FrameworkDebug("******** applicationDidReceiveMemoryWarning");
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
