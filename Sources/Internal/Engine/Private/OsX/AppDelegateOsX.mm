#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/AppDelegateOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/OsX/CoreNativeBridgeOsX.h"
#include "Engine/EngineModule.h"
#include "Engine/WindowNativeService.h"
#include "Utils/NSStringUtils.h"
#include "Notification/LocalNotificationController.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

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
    NSUserNotification *userNotification = [notification userInfo][(id)@"NSApplicationLaunchUserNotificationKey"];
    if (userNotification.userInfo != nil)
    {
        NSString* uid = [[userNotification userInfo] valueForKey:@"uid"];
        if (uid != nil && [uid length] != 0)
        {
            DAVA::String uidStr = DAVA::StringFromNSString(uid);
            bridge->mainDispatcher->PostEvent(DAVA::Private::MainDispatcherEvent::CreateLocalNotificationEvent(uidStr));
        }
    }
    bridge->ApplicationDidFinishLaunching();
}

- (void)applicationDidChangeScreenParameters:(NSNotification*)notification
{
    bridge->ApplicationDidChangeScreenParameters();
}

- (void)applicationDidBecomeActive:(NSNotification*)notification
{
    [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
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

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification
{
    return YES;
}

- (void) userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification
{
    NSString* uid = [[notification userInfo] valueForKey:@"uid"];
    if (uid != nil && [uid length] != 0)
    {
        DAVA::String uidStr = DAVA::StringFromNSString(uid);
        bridge->mainDispatcher->PostEvent(DAVA::Private::MainDispatcherEvent::CreateLocalNotificationEvent(uidStr));
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
        DAVA::Engine::Instance()->PrimaryWindow()->GetNativeService()->DoWindowDeminiaturize();
    }
}

@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
