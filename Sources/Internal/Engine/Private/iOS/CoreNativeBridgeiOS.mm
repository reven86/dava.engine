#include "Engine/Private/iOS/CoreNativeBridgeiOS.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Engine.h"
#include "Engine/PlatformApi.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/iOS/PlatformCoreiOS.h"
#include "Engine/Private/iOS/Window/WindowBackendiOS.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "UI/UIScreenManager.h"

#include "Logger/Logger.h"
#include "Time/SystemTimer.h"

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
- (void)pauseDisplayLink;
- (void)resumeDisplayLink;
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

- (void)pauseDisplayLink
{
    displayLink.paused = YES;
}

- (void)resumeDisplayLink
{
    displayLink.paused = NO;
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
    // Yuri Coder, 2013/02/06. This flag can be used to block drawView() call
    // in case if ASSERTion happened. This is introduced to do not stuck on the RenderManager::Lock()
    // mutex (since assertion might be called in the middle of drawing, DAVA::RenderManager::Instance()->Lock()
    // mutex might be already locked so we'll got a deadlock.
    // Return to this code after RenderManager mutex will be removed.
    if (GetEngineContext()->uiScreenManager->IsDrawBlocked())
        return;

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

bool CoreNativeBridge::ApplicationDidFinishLaunchingWithOptions(UIApplication* application, NSDictionary* launchOptions)
{
    Logger::FrameworkDebug("******** applicationDidFinishLaunchingWithOptions");

    engineBackend->OnGameLoopStarted();

    WindowBackend* primaryWindowBackend = EngineBackend::GetWindowBackend(engineBackend->GetPrimaryWindow());
    primaryWindowBackend->Create();

    objcInterop = [[ObjectiveCInterop alloc] init:this];
    [objcInterop setDisplayLinkInterval:1];

    [objcInterop enableGameControllerObserver:YES];

    NotifyListeners(ON_DID_FINISH_LAUNCHING, application, launchOptions);
    return true;
}

void CoreNativeBridge::ApplicationDidBecomeActive()
{
    Logger::FrameworkDebug("******** applicationDidBecomeActive");

    core->didBecomeResignActive.Emit(true);
    NotifyListeners(ON_DID_BECOME_ACTIVE, nullptr, nullptr);
}

void CoreNativeBridge::ApplicationWillResignActive()
{
    Logger::FrameworkDebug("******** applicationWillResignActive");

    core->didBecomeResignActive.Emit(false);
    NotifyListeners(ON_WILL_RESIGN_ACTIVE, nullptr, nullptr);
}

void CoreNativeBridge::ApplicationDidEnterBackground()
{
    core->didEnterForegroundBackground.Emit(false);
    NotifyListeners(ON_DID_ENTER_BACKGROUND, nullptr, nullptr);

    mainDispatcher->SendEvent(MainDispatcherEvent(MainDispatcherEvent::APP_SUSPENDED)); // Blocking call !!!

    [objcInterop pauseDisplayLink];

    goBackgroundTimeRelativeToBoot = SystemTimer::GetSystemUptimeMicros();
    goBackgroundTime = SystemTimer::GetAbsoluteMicros();
}

void CoreNativeBridge::ApplicationWillEnterForeground()
{
    mainDispatcher->PostEvent(MainDispatcherEvent(MainDispatcherEvent::APP_RESUMED));
    core->didEnterForegroundBackground.Emit(true);
    NotifyListeners(ON_WILL_ENTER_FOREGROUND, nullptr, nullptr);

    [objcInterop resumeDisplayLink];

    int64 timeSpentInBackground1 = SystemTimer::GetSystemUptimeMicros() - goBackgroundTimeRelativeToBoot;
    int64 timeSpentInBackground2 = SystemTimer::GetAbsoluteMicros() - goBackgroundTime;
    // Do adjustment only if SystemTimer has stopped ticking
    if (timeSpentInBackground1 - timeSpentInBackground2 > 500000l)
    {
        EngineBackend::AdjustSystemTimer(timeSpentInBackground1);
    }
}

void CoreNativeBridge::ApplicationWillTerminate()
{
    Logger::FrameworkDebug("******** applicationWillTerminate");

    NotifyListeners(ON_WILL_TERMINATE, nullptr, nullptr);

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

void CoreNativeBridge::RegisterUIApplicationDelegateListener(PlatformApi::Ios::UIApplicationDelegateListener* listener)
{
    DVASSERT(listener != nullptr);

    using std::begin;
    using std::end;

    LockGuard<Mutex> lock(listenersMutex);
    auto it = std::find(begin(appDelegateListeners), end(appDelegateListeners), listener);
    if (it == end(appDelegateListeners))
    {
        appDelegateListeners.push_back(listener);
    }
}

void CoreNativeBridge::UnregisterUIApplicationDelegateListener(PlatformApi::Ios::UIApplicationDelegateListener* listener)
{
    using std::begin;
    using std::end;

    LockGuard<Mutex> lock(listenersMutex);
    auto it = std::find(begin(appDelegateListeners), end(appDelegateListeners), listener);
    if (it != end(appDelegateListeners))
    {
        appDelegateListeners.erase(it);
    }
}

void CoreNativeBridge::NotifyListeners(eNotificationType type, NSObject* arg1, NSObject* arg2)
{
    Vector<PlatformApi::Ios::UIApplicationDelegateListener*> listenersCopy;
    {
        // Make copy to allow listeners unregistering inside a callback
        LockGuard<Mutex> lock(listenersMutex);
        listenersCopy.resize(appDelegateListeners.size());
        std::copy(appDelegateListeners.begin(), appDelegateListeners.end(), listenersCopy.begin());
    }
    for (PlatformApi::Ios::UIApplicationDelegateListener* l : listenersCopy)
    {
        switch (type)
        {
        case ON_DID_FINISH_LAUNCHING:
            l->didFinishLaunchingWithOptions(static_cast<UIApplication*>(arg1), static_cast<NSDictionary*>(arg2));
            break;
        case ON_DID_BECOME_ACTIVE:
            l->applicationDidBecomeActive();
            break;
        case ON_WILL_RESIGN_ACTIVE:
            l->applicationDidResignActive();
            break;
        case ON_WILL_ENTER_FOREGROUND:
            l->applicationWillEnterForeground();
            break;
        case ON_DID_ENTER_BACKGROUND:
            l->applicationDidEnterBackground();
            break;
        case ON_WILL_TERMINATE:
            l->applicationWillTerminate();
            break;
        case ON_DID_RECEIVE_LOCAL_NOTIFICATION:
            l->didReceiveLocalNotification(static_cast<UILocalNotification*>(arg1));
            break;
        default:
            break;
        }
    }
}

void CoreNativeBridge::ApplicationDidReceiveLocalNotification(UILocalNotification* notification)
{
    NotifyListeners(ON_DID_RECEIVE_LOCAL_NOTIFICATION, notification, nullptr);
}
} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
