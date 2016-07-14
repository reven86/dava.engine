#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

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
    CoreNativeBridge(PlatformCore* c);
    ~CoreNativeBridge();

    void Run();
    void OnFrameTimer();

    // Callbacks from AppDelegateiOS
    bool applicationWillFinishLaunchingWithOptions(NSDictionary* launchOptions);
    bool applicationDidFinishLaunchingWithOptions(NSDictionary* launchOptions);
    void applicationDidBecomeActive();
    void applicationWillResignActive();
    void applicationDidEnterBackground();
    void applicationWillEnterForeground();
    void applicationWillTerminate();
    void applicationDidReceiveMemoryWarning();

    PlatformCore* core = nullptr;
    FrameTimer* frameTimer = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
