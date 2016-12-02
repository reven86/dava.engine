#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/PlatformApi.h"

namespace DAVA
{
class LocalNotificationController;

namespace Private
{
struct LocalNotificationListener : public PlatformApi::Mac::NSApplicationDelegateListener
{
    LocalNotificationListener(LocalNotificationController& controller);
    virtual ~LocalNotificationListener() override;

    void applicationDidFinishLaunching(NSNotification* notification) override;
    void applicationDidBecomeActive() override;
    void didActivateNotification(NSUserNotification* notification) override;

private:
    DAVA::LocalNotificationController& localNotificationController;
};
} // namespace Private
} // namespace DAVA
#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
