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

// Objective-C class used for interoperation between Objective-C and C++.
// CADisplayLink, NSNotificationCenter, etc expect Objective-C selectors to notify about events
// so this class installs Objective-C handlers which transfer control into C++ class.
@interface ObjectiveCInterop : NSObject
{
    DAVA::Private::CoreNativeBridge* bridge;
    CADisplayLink* displayLink;
    DAVA::int32 curInterval;
}

- (id)init:(DAVA::Private::CoreNativeBridge*)nativeBridge;
- (void)setDisplayLinkInterval:(DAVA::int32)interval;
- (void)cancelDisplayLink;
- (void)enableGameControllerObserver:(BOOL)enable;

@end

@implementation ObjectiveCInterop

- (id)init:(DAVA::Private::CoreNativeBridge*)nativeBridge
{
    self = [super init];
    if (self != nil)
    {
        bridge = nativeBridge;
        curInterval = 1;
        displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(displayLinkTimerFired:)];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    }
    return self;
}

- (void)setDisplayLinkInterval:(DAVA::int32)interval
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

- (void)cancelDisplayLink
{
    [displayLink invalidate];
}

- (void)displayLinkTimerFired:(CADisplayLink*)dispLink
{
    bridge->OnFrameTimer();
}

- (void)enableGameControllerObserver:(BOOL)enable
{
    if (enable)
    {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(gameControllerDidConnected)
                                                     name:@"GCControllerDidConnectNotification"
                                                   object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(gameControllerDidDisconnected)
                                                     name:@"GCControllerDidDisconnectNotification"
                                                   object:nil];
    }
    else
    {
        [[NSNotificationCenter defaultCenter] removeObserver:self];
    }
}

- (void)gameControllerDidConnected
{
    bridge->GameControllerDidConnected();
}

- (void)gameControllerDidDisconnected
{
    bridge->GameControllerDidDisconnected();
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
    [objcInterop setDisplayLinkInterval:interval];
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

    objcInterop = [[ObjectiveCInterop alloc] init:this];
    [objcInterop setDisplayLinkInterval:1];

    [objcInterop enableGameControllerObserver:YES];
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

    [objcInterop cancelDisplayLink];
    [objcInterop enableGameControllerObserver:NO];

    engineBackend->OnGameLoopStopped();
    engineBackend->OnEngineCleanup();
}

void CoreNativeBridge::ApplicationDidReceiveMemoryWarning()
{
    Logger::FrameworkDebug("******** applicationDidReceiveMemoryWarning");
}

void CoreNativeBridge::GameControllerDidConnected()
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateGamepadAddedEvent(0));
}

void CoreNativeBridge::GameControllerDidDisconnected()
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateGamepadRemovedEvent(0));
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
