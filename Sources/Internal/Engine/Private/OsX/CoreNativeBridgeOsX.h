#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EnginePrivateFwd.h"

@class FrameTimer;
@class AppDelegate;

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
    void ApplicationDidFinishLaunching();
    void ApplicationDidChangeScreenParameters();
    void ApplicationDidBecomeActive();
    void ApplicationDidResignActive();
    void ApplicationDidHide();
    void ApplicationDidUnhide();
    bool ApplicationShouldTerminate();
    bool ApplicationShouldTerminateAfterLastWindowClosed();
    void ApplicationWillTerminate();

    PlatformCore* core = nullptr;

    MainDispatcher* mainDispatcher = nullptr;
    AppDelegate* appDelegate = nullptr;
    FrameTimer* frameTimer = nullptr;

    bool quitSent = false;
    bool closeRequestByApp = false;
    int32 curFps = 0;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
