#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/AppDelegateiOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/iOS/CoreNativeBridgeiOS.h"
#include "Engine/EngineModule.h"
#include "Utils/NSStringUtils.h"
#include "Notification/LocalNotificationController.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

namespace DAVA
{
namespace Private
{
extern CoreNativeBridge* coreNativeBridge;
}
}

@implementation AppDelegate

- (void)application:(UIApplication*)application didReceiveLocalNotification:(UILocalNotification*)notification
{
    bridge->ApplicationDidReceiveLocalNotification([application applicationState], notification);
}

- (BOOL)application:(UIApplication*)application willFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    bridge = DAVA::Private::coreNativeBridge;
    return bridge->ApplicationWillFinishLaunchingWithOptions(launchOptions) ? YES : NO;
}

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    UILocalNotification* notification = [launchOptions objectForKey:UIApplicationLaunchOptionsLocalNotificationKey];
    if (notification != nil && [application applicationState] != UIApplicationStateActive)
    {
        NSString* uid = [[notification userInfo] valueForKey:@"uid"];
        if (uid != nil && [uid length] != 0)
        {
            const DAVA::String& uidStr = DAVA::StringFromNSString(uid);
            bridge->mainDispatcher->PostEvent(DAVA::Private::MainDispatcherEvent::CreateLocalNotificationEvent(uidStr));
        }
    }
    return bridge->ApplicationDidFinishLaunchingWithOptions(launchOptions) ? YES : NO;
}

- (void)applicationDidBecomeActive:(UIApplication*)application
{
    [[UIApplication sharedApplication] cancelAllLocalNotifications];
    bridge->ApplicationDidBecomeActive();
}

- (void)applicationWillResignActive:(UIApplication*)application
{
    bridge->ApplicationWillResignActive();
}

- (void)applicationDidEnterBackground:(UIApplication*)application
{
    bridge->ApplicationDidEnterBackground();
}

- (void)applicationWillEnterForeground:(UIApplication*)application
{
    bridge->ApplicationWillEnterForeground();
}

- (void)applicationWillTerminate:(UIApplication*)application
{
    bridge->ApplicationWillTerminate();
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication*)application
{
    bridge->ApplicationDidReceiveMemoryWarning();
}

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
