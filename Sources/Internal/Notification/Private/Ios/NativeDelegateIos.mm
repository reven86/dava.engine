#include "Notification/Private/Ios/NativeDelegateIos.h"

#if defined(__DAVAENGINE_IPHONE__)

#import <Foundation/Foundation.h>

#include "Engine/Engine.h"
#include "Engine/NativeService.h"
#include "Logger/Logger.h"
#include "Utils/NSStringUtils.h"

namespace DAVA
{
namespace Private
{
NativeDelegate::NativeDelegate(LocalNotificationController& controller) : localNotificationController(controller)
{
    Engine::Instance()->GetNativeService()->RegisterUIApplicationDelegateListener(this);
}

NativeDelegate::~NativeDelegate()
{
    Engine::Instance()->GetNativeService()->UnregisterUIApplicationDelegateListener(this);
}

void NativeDelegate::didFinishLaunchingWithOptions(UIApplication* application, NSDictionary* launchOptions)
{
#if defined(__IPHONE_8_0)
    if ([UIApplication instancesRespondToSelector:@selector(registerUserNotificationSettings:)]){
        [application registerUserNotificationSettings:[UIUserNotificationSettings settingsForTypes:UIUserNotificationTypeAlert|UIUserNotificationTypeBadge|UIUserNotificationTypeSound categories:nil]];
    }
#endif
    UILocalNotification* notification = [launchOptions objectForKey:UIApplicationLaunchOptionsLocalNotificationKey];
    if (notification != nil && [application applicationState] != UIApplicationStateActive)
    {
        NSString* uid = [[notification userInfo] valueForKey:@"uid"];
        if (uid != nil && [uid length] != 0)
        {
            const String& uidStr = StringFromNSString(uid);
            auto func = [this, uidStr](){
                localNotificationController.OnNotificationPressed(uidStr);
            };
            Engine::Instance()->RunAsyncOnMainThread(func);
        }
    }
}

void NativeDelegate::applicationDidBecomeActive()
{
    [[UIApplication sharedApplication] cancelAllLocalNotifications];
}

void NativeDelegate::didReceiveLocalNotification(UILocalNotification* notification)
{
    NSString* uid = [[notification userInfo] valueForKey:@"uid"];
    if (uid != nil && [uid length] != 0)
    {
        const DAVA::String& uidStr = DAVA::StringFromNSString(uid);
        auto func = [this, uidStr](){
            localNotificationController.OnNotificationPressed(uidStr);
        };
        Engine::Instance()->RunAsyncOnMainThread(func);
    }
    DAVA::Logger::Debug("NativeDelegateIos::didActivateNotification");
}
} // namespace Private
} // namespace DAVA
#endif // __DAVAENGINE_MACOS__
