#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EnginePrivateFwd.h"

@class NSObject;
@class NSNotification;

@class FrameTimer;
@class AppDelegate;
@class NSNotification;
@class NSUserNotification;

namespace DAVA
{
namespace Private
{
// Bridge between C++ and Objective-C for OsX's PlatformCore class
// Responsibilities:
//  - holds neccesary Objective-C objects
//  - processes notifications from OsXAppDelegate which implements
//    interface NSApplicationDelegate
//
// CoreNativeBridge is friend of OsX's PlatformCore
struct CoreNativeBridge final
{
    CoreNativeBridge(PlatformCore* core);
    ~CoreNativeBridge();

    void Run();
    void Quit();
    void OnFrameTimer();

    // Callbacks from OsXAppDelegate
    void ApplicationWillFinishLaunching();
    void ApplicationDidFinishLaunching(NSNotification* notification);
    void ApplicationDidChangeScreenParameters();
    void ApplicationDidBecomeActive();
    void ApplicationDidResignActive();
    void ApplicationDidHide();
    void ApplicationDidUnhide();
    bool ApplicationShouldTerminate();
    bool ApplicationShouldTerminateAfterLastWindowClosed();
    void ApplicationWillTerminate();
    void ApplicationDidActivateNotification(NSUserNotification* notification);

    void RegisterNSApplicationDelegateListener(NSApplicationDelegateListener* listener);
    void UnregisterNSApplicationDelegateListener(NSApplicationDelegateListener* listener);

    enum eNotificationType
    {
        ON_DID_FINISH_LAUNCHING,
        ON_DID_BECOME_ACTIVE,
        ON_DID_RESIGN_ACTIVE,
        ON_WILL_TERMINATE,
        ON_DID_RECEIVE_REMOTE_NOTIFICATION,
        ON_DID_REGISTER_REMOTE_NOTIFICATION,
        ON_DID_FAIL_TO_REGISTER_REMOTE_NOTIFICATION,
    };
    void NotifyListeners(eNotificationType type, NSObject* arg1, NSObject* arg2, NSObject* arg3);

    PlatformCore* core = nullptr;

    MainDispatcher* mainDispatcher = nullptr;
    AppDelegate* appDelegate = nullptr;
    FrameTimer* frameTimer = nullptr;

    List<NSApplicationDelegateListener*> appDelegateListeners;

    bool quitSent = false;
    bool closeRequestByApp = false;
    int32 curFps = 0;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
