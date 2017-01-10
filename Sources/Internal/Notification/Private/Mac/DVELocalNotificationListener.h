#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <Foundation/NSObject.h>
#import "Engine/Mac/PlatformApi.h"

namespace DAVA
{
class LocalNotificationController;
}

@interface DVELocalNotificationListener : NSObject<DVEApplicationListener>

- (instancetype)initWithController:(DAVA::LocalNotificationController&)controller;

- (void)applicationDidFinishLaunching:(NSNotification*)notification;
- (void)applicationDidBecomeActive:(NSNotification*)notification;
- (void)userNotificationCenter:(NSUserNotificationCenter*)center didActivateNotification:(NSUserNotification*)notification;

@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
