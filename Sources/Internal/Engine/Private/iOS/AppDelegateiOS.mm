#include "Engine/Private/iOS/AppDelegateiOS.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/iOS/CoreNativeBridgeiOS.h"

namespace DAVA
{
namespace Private
{
extern CoreNativeBridge* coreNativeBridge;
}
}

@implementation AppDelegate

- (BOOL)application:(UIApplication*)application willFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    bridge = DAVA::Private::coreNativeBridge;
    return bridge->ApplicationWillFinishLaunchingWithOptions(application, launchOptions) ? YES : NO;
}

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    return bridge->ApplicationDidFinishLaunchingWithOptions(application, launchOptions) ? YES : NO;
}

- (void)applicationDidBecomeActive:(UIApplication*)application
{
    bridge->ApplicationDidBecomeActive(application);
}

- (void)applicationWillResignActive:(UIApplication*)application
{
    bridge->ApplicationWillResignActive(application);
}

- (void)applicationDidEnterBackground:(UIApplication*)application
{
    bridge->ApplicationDidEnterBackground(application);
}

- (void)applicationWillEnterForeground:(UIApplication*)application
{
    bridge->ApplicationWillEnterForeground(application);
}

- (void)applicationWillTerminate:(UIApplication*)application
{
    bridge->ApplicationWillTerminate(application);
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication*)application
{
    bridge->ApplicationDidReceiveMemoryWarning(application);
}

- (void)application:(UIApplication*)application didReceiveRemoteNotification:(NSDictionary*)userInfo
{
    bridge->DidReceiveRemoteNotification(application, userInfo);
}

- (void)application:(UIApplication*)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData*)deviceToken
{
    bridge->DidRegisterForRemoteNotificationsWithDeviceToken(application, deviceToken);
}

- (void)application:(UIApplication*)application didFailToRegisterForRemoteNotificationsWithError:(NSError*)error
{
    bridge->DidFailToRegisterForRemoteNotificationsWithError(application, error);
}

- (void)application:(UIApplication*)application didReceiveLocalNotification:(UILocalNotification*)notification
{
    bridge->ApplicationDidReceiveLocalNotification(application, notification);
}

- (void)application:(UIApplication*)application handleActionWithIdentifier:(NSString*)identifier
     forRemoteNotification:(NSDictionary*)userInfo
         completionHandler:(void (^)())completionHandler
{
    bridge->HandleActionWithIdentifier(application, identifier, userInfo, completionHandler);
}

- (bool)application:(UIApplication*)application openURL:(NSURL*)url sourceApplication:(NSString*)sourceApplication annotation:(id)annotation
{
    return bridge->OpenURL(application, url, sourceApplication, annotation) ? YES : NO;
}

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
