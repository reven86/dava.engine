#include "Engine/Private/iOS/CoreNativeBridgeiOS.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Engine.h"
#include "Engine/PlatformApiIos.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/iOS/PlatformCoreiOS.h"
#include "Engine/Private/iOS/Window/WindowBackendiOS.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "UI/UIScreenManager.h"

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

extern NSAutoreleasePool* preMainLoopReleasePool;

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
    appDelegateListeners = [[NSMutableArray alloc] init];
}

CoreNativeBridge::~CoreNativeBridge()
{
    [appDelegateListeners release];
}

void CoreNativeBridge::Run()
{
    [preMainLoopReleasePool drain];

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

bool CoreNativeBridge::ApplicationWillFinishLaunchingWithOptions(UIApplication* app, NSDictionary* launchOptions)
{
    Logger::FrameworkDebug("******** applicationWillFinishLaunchingWithOptions");

    return NotifyListeners(ON_WILL_FINISH_LAUNCHING, app, launchOptions);
}

bool CoreNativeBridge::ApplicationDidFinishLaunchingWithOptions(UIApplication* app, NSDictionary* launchOptions)
{
    Logger::FrameworkDebug("******** applicationDidFinishLaunchingWithOptions");

    engineBackend->OnGameLoopStarted();

    WindowBackend* primaryWindowBackend = EngineBackend::GetWindowBackend(engineBackend->GetPrimaryWindow());
    primaryWindowBackend->Create();

    objcInterop = [[ObjectiveCInterop alloc] init:this];
    [objcInterop setDisplayLinkInterval:1];
    [objcInterop enableGameControllerObserver:YES];

    return NotifyListeners(ON_DID_FINISH_LAUNCHING, app, launchOptions);
}

void CoreNativeBridge::ApplicationDidBecomeActive(UIApplication* app)
{
    Logger::FrameworkDebug("******** applicationDidBecomeActive");

    core->didBecomeResignActive.Emit(true);
    NotifyListeners(ON_DID_BECOME_ACTIVE, app);
}

void CoreNativeBridge::ApplicationWillResignActive(UIApplication* app)
{
    Logger::FrameworkDebug("******** applicationWillResignActive");

    core->didBecomeResignActive.Emit(false);
    NotifyListeners(ON_WILL_RESIGN_ACTIVE, app);
}

void CoreNativeBridge::ApplicationDidEnterBackground(UIApplication* app)
{
    core->didEnterForegroundBackground.Emit(false);
    NotifyListeners(ON_DID_ENTER_BACKGROUND, app);

    mainDispatcher->SendEvent(MainDispatcherEvent(MainDispatcherEvent::APP_SUSPENDED)); // Blocking call !!!
}

void CoreNativeBridge::ApplicationWillEnterForeground(UIApplication* app)
{
    mainDispatcher->PostEvent(MainDispatcherEvent(MainDispatcherEvent::APP_RESUMED));
    core->didEnterForegroundBackground.Emit(true);
    NotifyListeners(ON_WILL_ENTER_FOREGROUND, app);
}

void CoreNativeBridge::ApplicationWillTerminate(UIApplication* app)
{
    Logger::FrameworkDebug("******** applicationWillTerminate");

    NotifyListeners(ON_WILL_TERMINATE, app);

    [objcInterop cancelDisplayLink];
    [objcInterop enableGameControllerObserver:NO];

    engineBackend->OnGameLoopStopped();
    engineBackend->OnEngineCleanup();
}

void CoreNativeBridge::ApplicationDidReceiveMemoryWarning(UIApplication* app)
{
    Logger::FrameworkDebug("******** applicationDidReceiveMemoryWarning");

    NotifyListeners(ON_DID_RECEIVE_MEMORY_WARNING, app);
}

void CoreNativeBridge::ApplicationDidReceiveLocalNotification(UIApplication* app, UILocalNotification* notification)
{
    NotifyListeners(ON_DID_RECEIVE_LOCAL_NOTIFICATION, app, notification);
}

void CoreNativeBridge::DidReceiveRemoteNotification(UIApplication* app, NSDictionary* userInfo)
{
    NotifyListeners(ON_DID_RECEIVE_REMOTE_NOTIFICATION, app, userInfo);
}

void CoreNativeBridge::DidRegisterForRemoteNotificationsWithDeviceToken(UIApplication* app, NSData* deviceToken)
{
    NotifyListeners(ON_DID_REGISTER_FOR_REMOTE_NOTIFICATION_WITH_TOKEN, app, deviceToken);
}

void CoreNativeBridge::DidFailToRegisterForRemoteNotificationsWithError(UIApplication* app, NSError* error)
{
    NotifyListeners(ON_DID_FAIL_REGISTER_FOR_REMOTE_NOTIFICATION_WITH_ERROR, app, error);
}

void CoreNativeBridge::HandleActionWithIdentifier(UIApplication* app, NSString* identifier, NSDictionary* userInfo, id completionHandler)
{
    NotifyListeners(ON_HANDLE_ACTION_WITH_IDENTIFIER, app, identifier, userInfo, completionHandler);
}

bool CoreNativeBridge::OpenURL(UIApplication* app, NSURL* url, NSString* sourceApplication, id annotation)
{
    return NotifyListeners(ON_OPEN_URL, app, url, sourceApplication, annotation);
}

void CoreNativeBridge::GameControllerDidConnected()
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateGamepadAddedEvent(0));
}

void CoreNativeBridge::GameControllerDidDisconnected()
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateGamepadRemovedEvent(0));
}

void CoreNativeBridge::RegisterDVEApplicationListener(id<DVEApplicationListener> listener)
{
    DVASSERT(listener != nullptr);

    LockGuard<Mutex> lock(listenersMutex);
    if ([appDelegateListeners indexOfObject:listener] == NSNotFound)
    {
        [appDelegateListeners addObject:listener];
    }
}

void CoreNativeBridge::UnregisterDVEApplicationListener(id<DVEApplicationListener> listener)
{
    LockGuard<Mutex> lock(listenersMutex);
    [appDelegateListeners removeObject:listener];
}

bool CoreNativeBridge::NotifyListeners(eNotificationType type, NSObject* arg1, NSObject* arg2, NSObject* arg3, id arg4)
{
    bool ret = false;

    NSArray* listenersCopy = nil;
    {
        // Make copy to allow listeners unregistering inside a callback
        LockGuard<Mutex> lock(listenersMutex);
        listenersCopy = [appDelegateListeners copy];
    }

    for (id<DVEApplicationListener> listener in listenersCopy)
    {
        switch (type)
        {
        case ON_WILL_FINISH_LAUNCHING:
            if ([listener respondsToSelector:@selector(application:willFinishLaunchingWithOptions:)])
            {
                ret |= [listener application:static_cast<UIApplication*>(arg1) willFinishLaunchingWithOptions:static_cast<NSDictionary*>(arg2)];
            }
            break;

        case ON_DID_FINISH_LAUNCHING:
            if ([listener respondsToSelector:@selector(application:didFinishLaunchingWithOptions:)])
            {
                ret |= [listener application:static_cast<UIApplication*>(arg1) didFinishLaunchingWithOptions:static_cast<NSDictionary*>(arg2)];
            }
            break;
        case ON_DID_BECOME_ACTIVE:
            if ([listener respondsToSelector:@selector(application:applicationDidBecomeActive:)])
            {
                [listener applicationDidBecomeActive:static_cast<UIApplication*>(arg1)];
            }
            break;
        case ON_WILL_RESIGN_ACTIVE:
            if ([listener respondsToSelector:@selector(applicationWillResignActive:)])
            {
                [listener applicationWillResignActive:static_cast<UIApplication*>(arg1)];
            }
            break;
        case ON_DID_ENTER_BACKGROUND:
            if ([listener respondsToSelector:@selector(applicationDidEnterBackground:)])
            {
                [listener applicationDidEnterBackground:static_cast<UIApplication*>(arg1)];
            }
            break;
        case ON_WILL_ENTER_FOREGROUND:
            if ([listener respondsToSelector:@selector(applicationWillEnterForeground:)])
            {
                [listener applicationWillEnterForeground:static_cast<UIApplication*>(arg1)];
            }
            break;
        case ON_WILL_TERMINATE:
            if ([listener respondsToSelector:@selector(applicationWillTerminate:)])
            {
                [listener applicationWillTerminate:static_cast<UIApplication*>(arg1)];
            }
            break;
        case ON_DID_RECEIVE_MEMORY_WARNING:
            if ([listener respondsToSelector:@selector(applicationDidReceiveMemoryWarning:)])
            {
                [listener applicationDidReceiveMemoryWarning:static_cast<UIApplication*>(arg1)];
            }
            break;
        case ON_DID_REGISTER_FOR_REMOTE_NOTIFICATION_WITH_TOKEN:
            if ([listener respondsToSelector:@selector(application:didRegisterForRemoteNotificationsWithDeviceToken:)])
            {
                [listener application:static_cast<UIApplication*>(arg1) didRegisterForRemoteNotificationsWithDeviceToken:static_cast<NSData*>(arg2)];
            }
            break;
        case ON_DID_FAIL_REGISTER_FOR_REMOTE_NOTIFICATION_WITH_ERROR:
            if ([listener respondsToSelector:@selector(application:didFailToRegisterForRemoteNotificationsWithError:)])
            {
                [listener application:static_cast<UIApplication*>(arg1) didFailToRegisterForRemoteNotificationsWithError:static_cast<NSError*>(arg2)];
            }
            break;
        case ON_DID_RECEIVE_REMOTE_NOTIFICATION:
            if ([listener respondsToSelector:@selector(application:didReceiveRemoteNotification:)])
            {
                [listener application:static_cast<UIApplication*>(arg1) didReceiveRemoteNotification:static_cast<NSDictionary*>(arg2)];
            }
            break;
        case ON_DID_RECEIVE_LOCAL_NOTIFICATION:
            if ([listener respondsToSelector:@selector(application:didReceiveLocalNotification:)])
            {
                [listener application:static_cast<UIApplication*>(arg1) didReceiveLocalNotification:static_cast<UILocalNotification*>(arg2)];
            }
            break;
        case ON_HANDLE_ACTION_WITH_IDENTIFIER:
            if ([listener respondsToSelector:@selector(application:handleActionWithIdentifier:forRemoteNotification:completionHandler:)])
            {
                [listener application:static_cast<UIApplication*>(arg1) handleActionWithIdentifier:static_cast<NSString*>(arg2) forRemoteNotification:static_cast<NSDictionary*>(arg3) completionHandler:arg4];
            }
            break;
        case ON_OPEN_URL:
            if ([listener respondsToSelector:@selector(application:openURL:sourceApplication:annotation:)])
            {
                [listener application:static_cast<UIApplication*>(arg1) openURL:static_cast<NSURL*>(arg2) sourceApplication:static_cast<NSString*>(arg3) annotation:arg4];
            }
            break;

        default:
            DVASSERT(false);
            break;
        }
    }

    [listenersCopy release];

    return ret;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
