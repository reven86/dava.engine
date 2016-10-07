#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/AppDelegateOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/OsX/CoreNativeBridgeOsX.h"
//remove it
#include "Utils/NSStringUtils.h"
#include "Notification/LocalNotificationController.h"

@implementation AppDelegate

- (id)initWithBridge:(DAVA::Private::CoreNativeBridge*)nativeBridge
{
    self = [super init];
    if (self != nullptr)
    {
        bridge = nativeBridge;
    }
    return self;
}

- (void)applicationWillFinishLaunching:(NSNotification*)notification
{
    [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:self];
    bridge->ApplicationWillFinishLaunching();
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    if (NSDictionary *notificationPayload =
        [[notification userInfo] objectForKey:NSApplicationLaunchRemoteNotificationKey])
    {
        NSLog(@"applicationDidFinishLaunching: notification 1!!! payload");
    }
    
    
    NSUserNotification *userNotification = [notification userInfo][(id)@"NSApplicationLaunchUserNotificationKey"];
    if (userNotification.userInfo != nil)
    {
        NSLog(@"applicationDidFinishLaunching: notification payload %@", userNotification.userInfo);
    }

    bridge->ApplicationDidFinishLaunching();
}

- (void)applicationDidChangeScreenParameters:(NSNotification*)notification
{
    bridge->ApplicationDidChangeScreenParameters();
}

- (void)applicationDidBecomeActive:(NSNotification*)notification
{
    bridge->ApplicationDidBecomeActive();
}

- (void)applicationDidResignActive:(NSNotification*)notification
{
    bridge->ApplicationDidResignActive();
}

- (void)applicationDidHide:(NSNotification*)notification
{
    bridge->ApplicationDidHide();
}

- (void)applicationDidUnhide:(NSNotification*)notification
{
    bridge->ApplicationDidUnhide();
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
{
    bool r = bridge->ApplicationShouldTerminate();
    return r ? NSTerminateNow : NSTerminateCancel;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    bool r = bridge->ApplicationShouldTerminateAfterLastWindowClosed();
    return r ? YES : NO;
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    bridge->ApplicationWillTerminate();
}

- (void)application:(NSApplication *)application didReceiveRemoteNotification:(NSDictionary<NSString *,id> *)userInfo
{
    NSLog(@"didReceiveRemoteNotification: notification 1!!! payload");
    [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification
{
    return YES;
}

- (void) userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification
{
    DAVA::String uidStr =  DAVA::StringFromNSString([[notification userInfo] valueForKey:@"uid"]);
    //DAVA::String uidStr =  DAVA::StringFromNSString([userInfo valueForKey:@"uid"]);
    DAVA::LocalNotificationController::Instance()->OnNotificationPressed(uidStr);
    DVASSERT_MSG(false, uidStr.c_str());
}

@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
