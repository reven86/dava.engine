#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include <Engine/PlatformApi.h>

struct NativeDelegateMac : public DAVA::PlatformApi::Mac::NSApplicationDelegateListener
{
    void applicationDidFinishLaunching(NSNotification* notification) override;
    void applicationDidBecomeActive() override;
    void applicationDidResignActive() override;
    void applicationWillTerminate() override;

    void didReceiveRemoteNotification(NSApplication* application, NSDictionary* userInfo) override;
    void didRegisterForRemoteNotificationsWithDeviceToken(NSApplication* application, NSData* deviceToken) override;
    void didFailToRegisterForRemoteNotificationsWithError(NSApplication* application, NSError* error) override;
};

#endif // __DAVAENGINE_MACOS__
