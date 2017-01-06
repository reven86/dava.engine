#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/PlatformApi.h"

struct NativeDelegateIos : public DAVA::PlatformApi::Ios::UIApplicationDelegateListener
{
    void didFinishLaunchingWithOptions(UIApplication* application, NSDictionary* launchOptions) override;
    void applicationDidBecomeActive() override;
    void applicationDidResignActive() override;
    void applicationWillEnterForeground() override;
    void applicationDidEnterBackground() override;
    void applicationWillTerminate() override;
};

#endif // __DAVAENGINE_MACOS__
