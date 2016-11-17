#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/OsX/NSApplicationDelegateListener.h"
namespace DAVA
{
class LocalNotificationController;

struct NativeDelegate : public NSApplicationDelegateListener
{
    NativeDelegate(LocalNotificationController& controller);
    virtual ~NativeDelegate();

    void applicationDidFinishLaunching(NSNotification* notification) override;
    void applicationDidBecomeActive() override;
    void didActivateNotification(NSUserNotification* notification) override;
 
private:
    DAVA::LocalNotificationController& localNotificationController;
};
}
#endif // __DAVAENGINE_MACOS__
