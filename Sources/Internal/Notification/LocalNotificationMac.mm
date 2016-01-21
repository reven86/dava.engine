/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Notification/LocalNotificationMac.h"

#if defined(__DAVAENGINE_MACOS__)

#import "Notification/LocalNotificationImpl.h"

#import <Foundation/Foundation.h>

#import "Utils/NSStringUtils.h"
#import "Platform/DateTime.h"

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
}
#endif
