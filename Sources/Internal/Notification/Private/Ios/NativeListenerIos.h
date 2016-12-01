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
struct NativeListener : public PlatformApi::Ios::UIApplicationDelegateListener
{
    NativeListener(LocalNotificationController& controller);
    virtual ~NativeListener();
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
