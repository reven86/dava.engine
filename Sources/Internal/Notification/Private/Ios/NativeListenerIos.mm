#include "Notification/Private/Ios/NativeListenerIos.h"

#if defined(__DAVAENGINE_IPHONE__)
#if defined(__DAVAENGINE_COREV2__)

#import <Foundation/Foundation.h>

#include "Engine/Engine.h"
#include "Engine/NativeService.h"
#include "Logger/Logger.h"
#include "Utils/NSStringUtils.h"

namespace DAVA
{
namespace Private
{
NativeListener::NativeListener(LocalNotificationController& controller)
    : localNotificationController(controller)
{
    Engine::Instance()->GetNativeService()->RegisterUIApplicationDelegateListener(this);
}

NativeListener::~NativeListener()
{
    Engine::Instance()->GetNativeService()->UnregisterUIApplicationDelegateListener(this);
}

void NativeListener::didFinishLaunchingWithOptions(UIApplication* application, NSDictionary* launchOptions)
{
#if defined(__IPHONE_8_0)
    NSString* version = [[UIDevice currentDevice] systemVersion];
    if ([[[UIDevice currentDevice] systemVersion] compare:@"8.0" options:NSNumericSearch] != NSOrderedAscending)
    {
        // https://developer.apple.com/reference/uikit/uiapplication/1622932-registerusernotificationsettings
        // available 8.0 and later
        if ([UIApplication instancesRespondToSelector:@selector(registerUserNotificationSettings:)])
        {
            [application registerUserNotificationSettings:[UIUserNotificationSettings settingsForTypes:UIUserNotificationTypeAlert | UIUserNotificationTypeBadge | UIUserNotificationTypeSound categories:nil]];
        }
    }
#endif
    UILocalNotification* notification = [launchOptions objectForKey:UIApplicationLaunchOptionsLocalNotificationKey];
    if (notification != nil && [application applicationState] != UIApplicationStateActive)
    {
        NSString* uid = [[notification userInfo] valueForKey:@"uid"];
        if (uid != nil && [uid length] != 0)
        {
            const String& uidStr = StringFromNSString(uid);
            auto func = [this, uidStr]() {
                localNotificationController.OnNotificationPressed(uidStr);
            };
            Engine::Instance()->RunAsyncOnMainThread(func);
        }
    }
}

void NativeListener::applicationDidBecomeActive()
{
    [[UIApplication sharedApplication] cancelAllLocalNotifications];
}

void NativeListener::didReceiveLocalNotification(UILocalNotification* notification)
{
    NSString* uid = [[notification userInfo] valueForKey:@"uid"];
    if (uid != nil && [uid length] != 0)
    {
        const DAVA::String& uidStr = DAVA::StringFromNSString(uid);
        auto func = [this, uidStr]() {
            localNotificationController.OnNotificationPressed(uidStr);
        };
        Engine::Instance()->RunAsyncOnMainThread(func);
    }
}
} // namespace Private
} // namespace DAVA
#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_IPHONE__
