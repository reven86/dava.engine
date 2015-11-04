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


#include "Notification/LocalNotificationIOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#import "Notification/LocalNotificationImpl.h"
#import <UIKit/UIApplication.h>
#import <UIKit/UILocalNotification.h>
#import "Utils/NSStringUtils.h"
#import "Platform/DateTime.h"

namespace DAVA
{
    
struct UILocalNotificationWrapper
{
    UILocalNotification *impl = NULL;
};

LocalNotificationIOS::LocalNotificationIOS(const String &_id)
    : notification(NULL)
{
    notificationId = _id;
}
    
LocalNotificationIOS::~LocalNotificationIOS()
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

void LocalNotificationIOS::SetAction(const WideString &_action)
{
}

void LocalNotificationIOS::Hide()
{
    if (NULL != notification)
    {
        NSString *uid = NSStringFromString(notificationId);
        bool scheduledNotificationFoundAndRemoved = false;
        for(UILocalNotification *n in [[UIApplication sharedApplication] scheduledLocalNotifications])
        {
            NSDictionary *userInfo = n.userInfo;
            if(userInfo && [userInfo[@"uid"] isEqual:uid])
            {
                //[UIApplication sharedApplication] cancel
                scheduledNotificationFoundAndRemoved = true;
            }
        }
        [[UIApplication sharedApplication] cancelLocalNotification:notification->impl];
    }
}

void LocalNotificationIOS::ShowText(const WideString &title, const WideString &text, bool useSound)
{
    if (NULL == notification)
    {
        notification = new UILocalNotificationWrapper();
        notification->impl = [[UILocalNotification alloc] init];
    }
    
    notification->impl.alertBody = NSStringFromWideString(text);
    
    notification->impl.userInfo = @{ @"uid" : NSStringFromString(notificationId), @"action" : @"test action"};

    [[UIApplication sharedApplication] cancelLocalNotification:notification->impl];
    
    if (useSound)
    {
        notification->impl.soundName = UILocalNotificationDefaultSoundName;
    }
    
    [[UIApplication sharedApplication] scheduleLocalNotification:notification->impl];
}

void LocalNotificationIOS::ShowProgress(const WideString &title, const WideString &text, uint32 total, uint32 progress, bool useSound)
{
}

void LocalNotificationIOS::PostDelayedNotification(const WideString &title, const WideString &text, int delaySeconds, bool useSound)
{
    UILocalNotification *notification = [[[UILocalNotification alloc] init] autorelease];
    notification.alertBody = NSStringFromWideString(text);
    notification.timeZone = [NSTimeZone defaultTimeZone];
    notification.fireDate = [NSDate dateWithTimeIntervalSinceNow:delaySeconds];
    
    if (useSound)
    {
        notification.soundName = UILocalNotificationDefaultSoundName;
    }

    [[UIApplication sharedApplication] scheduleLocalNotification:notification];
}

void LocalNotificationIOS::RemoveAllDelayedNotifications()
{
    for(UILocalNotification *notification in [[UIApplication sharedApplication] scheduledLocalNotifications])
    {
        [[UIApplication sharedApplication] cancelLocalNotification:notification];
    }
}

LocalNotificationImpl *LocalNotificationImpl::Create(const String &_id)
{
    return new LocalNotificationIOS(_id);
}

}
#endif
