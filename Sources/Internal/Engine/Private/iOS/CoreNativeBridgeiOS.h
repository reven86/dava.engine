#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/EnginePrivateFwd.h"

@class NSDictionary;
@class FrameTimer;

namespace DAVA
{
namespace Private
{
// Bridge between C++ and Objective-C for iOS's PlatformCore class
// Responsibilities:
//  - holds neccesary Objective-C objects
//
// CoreNativeBridge is friend of iOS's PlatformCore
struct CoreNativeBridge final
{
    CoreNativeBridge(PlatformCore* core);
    ~CoreNativeBridge();

    void Run();
    void OnFrameTimer();

    // Callbacks from AppDelegateiOS
    bool ApplicationWillFinishLaunchingWithOptions(NSDictionary* launchOptions);
    bool ApplicationDidFinishLaunchingWithOptions(NSDictionary* launchOptions);
    void ApplicationDidBecomeActive();
    void ApplicationWillResignActive();
    void ApplicationDidEnterBackground();
    void ApplicationWillEnterForeground();
    void ApplicationWillTerminate();
    void ApplicationDidReceiveMemoryWarning();

    PlatformCore& core;
    FrameTimer* frameTimer = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
