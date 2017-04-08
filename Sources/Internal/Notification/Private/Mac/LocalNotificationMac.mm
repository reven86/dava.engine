#include "Notification/Private/Mac/LocalNotificationMac.h"
#include "Utils/StringFormat.h"

#if defined(__DAVAENGINE_MACOS__)

#import "Notification/Private/LocalNotificationImpl.h"

#import <Foundation/Foundation.h>

#import "Utils/NSStringUtils.h"

namespace DAVA
{
struct NSUserNotificationWrapper
{
    NSUserNotification* impl = NULL;
};

LocalNotificationMac::LocalNotificationMac(const String& _id)
    : notification(NULL)
{
    notificationId = _id;
}

LocalNotificationMac::~LocalNotificationMac()
{
    if (notification)
    {
        if (notification->impl)
        {
            [notification->impl release];
        }
        delete notification;
    }
}

void LocalNotificationMac::SetAction(const WideString& _action)
{
}

void LocalNotificationMac::Hide()
{
    if (NULL != notification)
    {
        NSString* uid = NSStringFromString(notificationId);
        bool scheduledNotificationFoundAndRemoved = false;
        for (NSUserNotification* n in [NSUserNotificationCenter.defaultUserNotificationCenter scheduledNotifications])
        {
            NSDictionary* userInfo = n.userInfo;
            if (userInfo && [userInfo[@"uid"] isEqual:uid])
            {
                // i don't know why it is made like that - taken from ios implementation.
                //[NSUserNotificationCenter.defaultUserNotificationCenter removeScheduledNotification:n];
                scheduledNotificationFoundAndRemoved = true;
            }
        }

        [NSUserNotificationCenter.defaultUserNotificationCenter removeDeliveredNotification:notification->impl];
    }
}

void LocalNotificationMac::ShowText(const WideString& title, const WideString& text, bool useSound)
{
    if (NULL == notification)
    {
        notification = new NSUserNotificationWrapper();
        notification->impl = [[NSUserNotification alloc] init];
    }

    notification->impl.userInfo = @{ @"uid" : NSStringFromString(notificationId),
                                     @"action" : @"test action" };

    [NSUserNotificationCenter.defaultUserNotificationCenter removeDeliveredNotification:notification->impl];

    if (useSound)
    {
        notification->impl.soundName = NSUserNotificationDefaultSoundName;
    }

    notification->impl.title = NSStringFromWideString(title);
    notification->impl.informativeText = NSStringFromWideString(text);

    [NSUserNotificationCenter.defaultUserNotificationCenter deliverNotification:notification->impl];
}

void LocalNotificationMac::ShowProgress(const WideString& title, const WideString& text, uint32 total, uint32 progress, bool useSound)
{
    double percentage = (static_cast<double>(progress) / total) * 100.0;
    WideString titleText = title + Format(L" %.02f%%", percentage);

    ShowText(titleText, text, useSound);
}

void LocalNotificationMac::PostDelayedNotification(const WideString& title, const WideString& text, int delaySeconds, bool useSound)
{
    NSUserNotification* notification = [[[NSUserNotification alloc] init] autorelease];
    notification.informativeText = NSStringFromWideString(text);
    notification.title = NSStringFromWideString(title);
    notification.deliveryTimeZone = [NSTimeZone defaultTimeZone];
    notification.deliveryDate = [NSDate dateWithTimeIntervalSinceNow:delaySeconds];

    if (useSound)
    {
        notification.soundName = NSUserNotificationDefaultSoundName;
    }

    [NSUserNotificationCenter.defaultUserNotificationCenter scheduleNotification:notification];
}

void LocalNotificationMac::RemoveAllDelayedNotifications()
{
    for (NSUserNotification* notification in [NSUserNotificationCenter.defaultUserNotificationCenter scheduledNotifications])
    {
        [NSUserNotificationCenter.defaultUserNotificationCenter removeScheduledNotification:notification];
    }
}

LocalNotificationImpl* LocalNotificationImpl::Create(const String& _id)
{
    return new LocalNotificationMac(_id);
}

void LocalNotificationImpl::RequestPermissions()
{
}
}
#endif // defined(__DAVAENGINE_MACOS__)
