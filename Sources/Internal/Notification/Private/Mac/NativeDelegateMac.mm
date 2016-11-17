#include "Notification/Private/Mac/NativeDelegateMac.h"

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

NativeDelegate::NativeDelegate(DAVA::LocalNotificationController& controller) : localNotificationController(controller)
{
    Engine::Instance()->GetNativeService()->RegisterNSApplicationDelegateListener(this);
}

NativeDelegate::~NativeDelegate()
{
    Engine::Instance()->GetNativeService()->UnregisterNSApplicationDelegateListener(this);
}

void NativeDelegate::applicationDidFinishLaunching(NSNotification* notification)
{
    //using namespace DAVA;
    NSUserNotification* userNotification = [notification userInfo][(id) @"NSApplicationLaunchUserNotificationKey"];
    if (userNotification.userInfo != nil)
    {
        NSString* uid = [[userNotification userInfo] valueForKey:@"uid"];
        if (uid != nil && [uid length] != 0)
        {
            DAVA::String uidStr = DAVA::StringFromNSString(uid);
            auto func = [this, uidStr](){
                localNotificationController.OnNotificationPressed(uidStr);
            };
            Engine::Instance()->RunAsyncOnMainThread(func);
        }
    }
    Logger::Debug("NativeDelegateMac::applicationDidFinishLaunching");
}
    
void NativeDelegate::applicationDidBecomeActive()
{
    Logger::Debug("NativeDelegateMac::applicationDidBecomeActive");
    [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
}

void NativeDelegate::didActivateNotification(NSUserNotification* notification)
{
    //using namespace DAVA;
    NSString* uid = [[notification userInfo] valueForKey:@"uid"];
    if (uid != nil && [uid length] != 0)
    {
        DAVA::String uidStr = DAVA::StringFromNSString(uid);
        auto func = [this, uidStr](){
            localNotificationController.OnNotificationPressed(uidStr);
        };
        Engine::Instance()->RunAsyncOnMainThread(func);

        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
        Engine::Instance()->PrimaryWindow()->GetNativeService()->DoWindowDeminiaturize();
    }
    Logger::Debug("NativeDelegateMac::didActivateNotification");
}
} //  namespace DAVA
#endif // __DAVAENGINE_MACOS__
