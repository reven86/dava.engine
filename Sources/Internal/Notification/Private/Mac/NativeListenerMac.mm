#include "Notification/Private/Mac/NativeListenerMac.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <Foundation/Foundation.h>

#include "Engine/Engine.h"
#include "Engine/Osx/NativeServiceOsX.h"
#include "Engine/Osx/WindowNativeServiceOsX.h"
#include "Engine/Window.h"
#include "Logger/Logger.h"
#include "Notification/LocalNotificationController.h"
#include "Utils/NSStringUtils.h"

namespace DAVA
{
namespace Private
{
NativeListener::NativeListener(DAVA::LocalNotificationController& controller)
    : localNotificationController(controller)
{
    Engine::Instance()->GetNativeService()->RegisterNSApplicationDelegateListener(this);
}

NativeListener::~NativeListener()
{
    Engine::Instance()->GetNativeService()->UnregisterNSApplicationDelegateListener(this);
}

void NativeListener::applicationDidFinishLaunching(NSNotification* notification)
{
    //using namespace DAVA;
    NSUserNotification* userNotification = [notification userInfo][(id) @"NSApplicationLaunchUserNotificationKey"];
    if (userNotification.userInfo != nil)
    {
        NSString* uid = [[userNotification userInfo] valueForKey:@"uid"];
        if (uid != nil && [uid length] != 0)
        {
            DAVA::String uidStr = DAVA::StringFromNSString(uid);
            auto func = [this, uidStr]() {
                localNotificationController.OnNotificationPressed(uidStr);
            };
            Engine::Instance()->RunAsyncOnMainThread(func);
        }
    }
}

void NativeListener::applicationDidBecomeActive()
{
    [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
}

void NativeListener::didActivateNotification(NSUserNotification* notification)
{
    //using namespace DAVA;
    NSString* uid = [[notification userInfo] valueForKey:@"uid"];
    if (uid != nil && [uid length] != 0)
    {
        DAVA::String uidStr = DAVA::StringFromNSString(uid);
        auto func = [this, uidStr]() {
            localNotificationController.OnNotificationPressed(uidStr);
        };
        Engine::Instance()->RunAsyncOnMainThread(func);

        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
        Engine::Instance()->PrimaryWindow()->GetNativeService()->DoWindowDeminiaturize();
    }
}
} // namespace Private
} //  namespace DAVA
#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
