#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)
#if defined(__DAVAENGINE_COREV2__)

#include "Engine/OsX/NSApplicationDelegateListener.h"
namespace DAVA
{
class LocalNotificationController;

namespace Private
{
struct NativeListener : public NSApplicationDelegateListener
{
    NativeListener(LocalNotificationController& controller);
    virtual ~NativeListener();

    void applicationDidFinishLaunching(NSNotification* notification) override;
    void applicationDidBecomeActive() override;
    void didActivateNotification(NSUserNotification* notification) override;

private:
    DAVA::LocalNotificationController& localNotificationController;
};
} // namespace Private
} // namespace DAVA
#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_MACOS__
