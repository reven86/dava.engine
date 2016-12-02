#include "Notification/Private/Mac/LocalNotificationListenerMac.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <Foundation/Foundation.h>

#include "Engine/Engine.h"
#include "Engine/PlatformApi.h"
#include "Engine/Window.h"
#include "Logger/Logger.h"
#include "Notification/LocalNotificationController.h"
#include "Utils/NSStringUtils.h"

namespace DAVA
{
namespace Private
{
LocalNotificationListener::LocalNotificationListener(DAVA::LocalNotificationController& controller)
    : localNotificationController(controller)
{
    PlatformApi::Mac::RegisterNSApplicationDelegateListener(this);
}

LocalNotificationListener::~LocalNotificationListener()
{
    PlatformApi::Mac::UnregisterNSApplicationDelegateListener(this);
}

void LocalNotificationListener::applicationDidFinishLaunching(NSNotification* notification)
{
    //using namespace DAVA;
    NSUserNotification* userNotification = [notification userInfo][(id) @"NSApplicationLaunchUserNotificationKey"];
    if (userNotification && (userNotification.userInfo != nil))
    {
        NSString* uid = [[userNotification userInfo] valueForKey:@"uid"];
        if (uid != nil && [uid length] != 0)
        {
            DAVA::String uidStr = DAVA::StringFromNSString(uid);
            auto func = [this, uidStr]() {
                localNotificationController.OnNotificationPressed(uidStr);
            };
            RunOnMainThreadAsync(func);
        }
    }
}

void LocalNotificationListener::applicationDidBecomeActive()
{
    [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
}

void LocalNotificationListener::didActivateNotification(NSUserNotification* notification)
{
    NSString* uid = [[notification userInfo] valueForKey:@"uid"];
    if (uid != nil && [uid length] != 0)
    {
        DAVA::String uidStr = DAVA::StringFromNSString(uid);
        auto func = [this, uidStr]() {
            localNotificationController.OnNotificationPressed(uidStr);
        };
        RunOnMainThreadAsync(func);

        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
        PlatformApi::Mac::PrimaryWindowDeminiaturize();
    }
}
} // namespace Private
} //  namespace DAVA
#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
