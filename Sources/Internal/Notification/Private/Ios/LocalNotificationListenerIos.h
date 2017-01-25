#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/PlatformApi.h"
namespace DAVA
{
class LocalNotificationController;

namespace Private
{
struct LocalNotificationListener : public PlatformApi::Ios::UIApplicationDelegateListener
{
    LocalNotificationListener(LocalNotificationController& controller);
    virtual ~LocalNotificationListener() override;
    void didFinishLaunchingWithOptions(UIApplication* application, NSDictionary* launchOptions) override;
    void applicationDidBecomeActive() override;
    void didReceiveLocalNotification(UILocalNotification* notification) override;

private:
    LocalNotificationController& localNotificationController;
};
} // namespace Private
} // namespace DAVA
#endif // defined(__DAVAENGINE_MACOS__)
#endif // defined(__DAVAENGINE_COREV2__)
