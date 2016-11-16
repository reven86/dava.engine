#include "Infrastructure/NativeDelegateMac.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <Foundation/Foundation.h>

#include <Logger/Logger.h>
#include <Utils/NSStringUtils.h>

void NativeDelegateMac::applicationDidFinishLaunching(NSNotification* notification)
{
    using namespace DAVA;
    NSUserNotification* userNotification = [notification userInfo][(id) @"NSApplicationLaunchUserNotificationKey"];
    if (userNotification.userInfo != nil)
    {
        NSString* uid = [[userNotification userInfo] valueForKey:@"uid"];
        if (uid != nil && [uid length] != 0)
        {
            DAVA::String uidStr = DAVA::StringFromNSString(uid);
            mainDispatcher->PostEvent(DAVA::Private::MainDispatcherEvent::CreateLocalNotificationEvent(uidStr));
        }
    }
    Logger::Debug("NativeDelegateMac::applicationDidFinishLaunching");
}

void NativeDelegateMac::applicationDidBecomeActive()
{
    DAVA::Logger::Debug("NativeDelegateMac::applicationDidBecomeActive");
    [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
}

void NativeDelegateMac::applicationDidResignActive()
{
    DAVA::Logger::Debug("NativeDelegateMac::applicationDidResignActive");
}

void NativeDelegateMac::applicationWillTerminate()
{
    DAVA::Logger::Debug("NativeDelegateMac::applicationWillTerminate");
}

void NativeDelegateMac::didReceiveRemoteNotification(NSApplication* application, NSDictionary* userInfo)
{
    using namespace DAVA;
    Logger::Debug("TestBed.NativeDelegateMac::didReceiveRemoteNotification: enter");
    Logger::Debug("    dictionary:");
    for (NSString* key in userInfo)
    {
        String k = StringFromNSString(key);
        String d = StringFromNSString([userInfo[key] description]);
        Logger::Debug("        %s: %d", k.c_str(), d.c_str());
    }
    Logger::Debug("NativeDelegateMac::didReceiveRemoteNotification");
}

void NativeDelegateMac::didRegisterForRemoteNotificationsWithDeviceToken(NSApplication* application, NSData* deviceToken)
{
    DAVA::Logger::Debug("NativeDelegateMac::didRegisterForRemoteNotificationsWithDeviceToken");
}

void NativeDelegateMac::didFailToRegisterForRemoteNotificationsWithError(NSApplication* application, NSError* error)
{
    using namespace DAVA;
    String descr = StringFromNSString([error localizedDescription]);
    DAVA::Logger::Debug("NativeDelegateMac::didFailToRegisterForRemoteNotificationsWithError: %s", descr.c_str());
}

void NativeDelegateMac::didActivateNotification(NSUserNotification* notification)
{
    NSString* uid = [[notification userInfo] valueForKey:@"uid"];
    if (uid != nil && [uid length] != 0)
    {
        DAVA::String uidStr = DAVA::StringFromNSString(uid);
        mainDispatcher->PostEvent(DAVA::Private::MainDispatcherEvent::CreateLocalNotificationEvent(uidStr));
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
        core->engineBackend->GetPrimaryWindow()->GetNativeService()->DoWindowDeminiaturize();
    }
    DAVA::Logger::Debug("NativeDelegateMac::didActivateNotification");
}

#endif // __DAVAENGINE_MACOS__
