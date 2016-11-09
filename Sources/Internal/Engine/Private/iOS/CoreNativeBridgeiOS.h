#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/EnginePrivateFwd.h"

@class NSDictionary;
@class ObjectiveCInterop;
@class NotificationBridge;
@class UILocalNotification;

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
    void ApplicationDidReceiveLocalNotification(UILocalNotification* notification);

    void GameControllerDidConnected();
    void GameControllerDidDisconnected();

    PlatformCore* core = nullptr;
    EngineBackend* engineBackend = nullptr;
    MainDispatcher* mainDispatcher = nullptr;
    ObjectiveCInterop* objcInterop = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
