#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#include "Concurrency/Mutex.h"
#include "Engine/Private/EnginePrivateFwd.h"

@class NSObject;
@class NSDictionary;
@class UIApplication;
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
    bool ApplicationDidFinishLaunchingWithOptions(UIApplication* application, NSDictionary* launchOptions);
    void ApplicationDidBecomeActive();
    void ApplicationWillResignActive();
    void ApplicationDidEnterBackground();
    void ApplicationWillEnterForeground();
    void ApplicationWillTerminate();
    void ApplicationDidReceiveMemoryWarning();
    void ApplicationDidReceiveLocalNotification(UILocalNotification* notification);

    void GameControllerDidConnected();
    void GameControllerDidDisconnected();

    void RegisterUIApplicationDelegateListener(PlatformApi::Ios::UIApplicationDelegateListener* listener);
    void UnregisterUIApplicationDelegateListener(PlatformApi::Ios::UIApplicationDelegateListener* listener);

    enum eNotificationType
    {
        ON_DID_FINISH_LAUNCHING,
        ON_DID_BECOME_ACTIVE,
        ON_WILL_RESIGN_ACTIVE,
        ON_WILL_ENTER_FOREGROUND,
        ON_DID_ENTER_BACKGROUND,
        ON_WILL_TERMINATE,
        ON_DID_RECEIVE_LOCAL_NOTIFICATION,
    };
    void NotifyListeners(eNotificationType type, NSObject* arg1, NSObject* arg2);

    PlatformCore* core = nullptr;
    EngineBackend* engineBackend = nullptr;
    MainDispatcher* mainDispatcher = nullptr;
    ObjectiveCInterop* objcInterop = nullptr;

    Mutex listenersMutex;
    List<PlatformApi::Ios::UIApplicationDelegateListener*> appDelegateListeners;

    int64 goBackgroundTimeRelativeToBoot = 0;
    int64 goBackgroundTime = 0;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
