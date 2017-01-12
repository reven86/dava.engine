// #include "Notification/Private/Ios/DVELocalNotificationListener.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#import <Foundation/Foundation.h>
#import <UIKit/UIDevice.h>
#import <UIKit/UILocalNotification.h>
#import <UIKit/UIApplication.h>
#import <UIKit/UIUserNotificationSettings.h>

#import "Notification/Private/Ios/DVELocalNotificationListener.h"
#import "Notification/LocalNotificationController.h"

#include "Engine/Engine.h"
#include "Logger/Logger.h"
#include "Utils/NSStringUtils.h"

@implementation DVELocalNotificationListener
{
    DAVA::LocalNotificationController* notificationController;
}

- (instancetype)initWithController:(DAVA::LocalNotificationController&)controller
{
    if (self = [super init])
    {
        notificationController = &controller;
    }

    return self;
}

- (void)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    NSString* version = [[UIDevice currentDevice] systemVersion];
    if ([version compare:@"8.0" options:NSNumericSearch] != NSOrderedAscending)
    {
#if defined(__IPHONE_8_0)
        // https://developer.apple.com/reference/uikit/uiapplication/1622932-registerusernotificationsettings
        // available 8.0 and later
        if ([UIApplication instancesRespondToSelector:@selector(registerUserNotificationSettings:)])
        {
            [application registerUserNotificationSettings:[UIUserNotificationSettings settingsForTypes:UIUserNotificationTypeAlert | UIUserNotificationTypeBadge | UIUserNotificationTypeSound categories:nil]];
        }
#endif
    }
    else
    {
        UILocalNotification* notification = [launchOptions objectForKey:UIApplicationLaunchOptionsLocalNotificationKey];
        if (notification != nil && [application applicationState] != UIApplicationStateActive)
        {
            NSString* uid = [[notification userInfo] valueForKey:@"uid"];
            if (uid != nil && [uid length] != 0)
            {
                DAVA::String uidStr = DAVA::StringFromNSString(uid);
                auto func = [self, uidStr]()
                {
                    notificationController->OnNotificationPressed(uidStr);
                };
                DAVA::RunOnMainThreadAsync(func);
            }
        }
    }
}

- (void)applicationDidBecomeActive:(UIApplication*)application
{
    [[UIApplication sharedApplication] cancelAllLocalNotifications];
}

- (void)application:(UIApplication*)application didReceiveLocalNotification:(UILocalNotification*)notification;
{
    if ([application applicationState] != UIApplicationStateActive)
    {
        NSString* uid = [[notification userInfo] valueForKey:@"uid"];
        if (uid != nil && [uid length] != 0)
        {
            DAVA::String uidStr = DAVA::StringFromNSString(uid);
            auto func = [self, uidStr]() {
                notificationController->OnNotificationPressed(uidStr);
            };
            DAVA::RunOnMainThreadAsync(func);
        }
    }
}

@end

#endif // defined(__DAVAENGINE_IPHONE__)
#endif // defined(__DAVAENGINE_COREV2__)
