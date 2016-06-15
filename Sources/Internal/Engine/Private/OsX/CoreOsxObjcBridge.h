#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EngineFwd.h"

@class FrameTimer;
@class OsXAppDelegate;

namespace DAVA
{
namespace Private
{
// Bridge between C++ and Objective-C for CoreOsX class
// Responsibilities:
//  - holds neccesary Objective-C objects
//  - processes notifications from OsXAppDelegate which implements
//    interface NSApplicationDelegate
//
// CoreOsXObjcBridge is friend of CoreOsX
struct CoreOsXObjcBridge final
{
    CoreOsXObjcBridge(CoreOsX* coreOsX);
    ~CoreOsXObjcBridge();

    void InitNSApplication();
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

    CoreOsX* core = nullptr;

    OsXAppDelegate* appDelegate = nullptr;
    FrameTimer* frameTimer = nullptr;

    bool quitSent = false;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
