#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/CoreNativeBridgeOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <Foundation/Foundation.h>

#include "Engine/Window.h"
#include "Engine/OsX/NSApplicationDelegateListener.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/OsX/PlatformCoreOsx.h"
#include "Engine/Private/OsX/Window/WindowBackendOsX.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

#import "Engine/Private/OsX/AppDelegateOsX.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

// Wrapper over NSTimer to connect Objective-C NSTimer object to
// C++ class CoreNativeBridge
@interface FrameTimer : NSObject
{
    DAVA::Private::CoreNativeBridge* bridge;
    double curInterval;

    NSTimer* timer;
}

- (id)init:(DAVA::Private::CoreNativeBridge*)nativeBridge;
- (void)set:(double)interval;
- (void)cancel;
- (void)timerFired:(NSTimer*)timer;

@end

@implementation FrameTimer

- (id)init:(DAVA::Private::CoreNativeBridge*)nativeBridge
{
    self = [super init];
    if (self != nil)
    {
        bridge = nativeBridge;
        curInterval = -1.0;
    }
    return self;
}

- (void)set:(double)interval
{
    const double delta = 0.000001;
    if (std::abs(interval - curInterval) > delta)
    {
        [self cancel];
        timer = [NSTimer scheduledTimerWithTimeInterval:interval
                                                 target:self
                                               selector:@selector(timerFired:)
                                               userInfo:nil
                                                repeats:YES];
        curInterval = interval;
    }
}

- (void)cancel
{
    [timer invalidate];
    timer = nullptr;
}

- (void)timerFired:(NSTimer*)timer
{
    bridge->OnFrameTimer();
}

@end

//////////////////////////////////////////////////////////////////

namespace DAVA
{
namespace Private
{
CoreNativeBridge::CoreNativeBridge(PlatformCore* core)
    : core(core)
{
    // Force init NSApplication
    [NSApplication sharedApplication];
}

CoreNativeBridge::~CoreNativeBridge() = default;

void CoreNativeBridge::Run()
{
    @autoreleasepool
    {
        appDelegate = [[AppDelegate alloc] initWithBridge:this];
        [[NSApplication sharedApplication] setDelegate:(id<NSApplicationDelegate>)appDelegate];

        // NSApplicationMain never returns
        // NSApplicationMain itself ignores the argc and argv arguments. Instead, Cocoa gets its arguments indirectly via _NSGetArgv, _NSGetArgc, and _NSGetEnviron.
        // See https://developer.apple.com/library/mac/documentation/Cocoa/Reference/ApplicationKit/Miscellaneous/AppKit_Functions/#//apple_ref/c/func/NSApplicationMain
        ::NSApplicationMain(0, nullptr);
    }
}

void CoreNativeBridge::Quit()
{
    closeRequestByApp = true;
    if (!quitSent)
    {
        quitSent = true;
        [[NSApplication sharedApplication] terminate:nil];
    }
}

void CoreNativeBridge::OnFrameTimer()
{
    int32 fps = core->OnFrame();
    if (fps <= 0)
    {
        // To prevent division by zero
        fps = std::numeric_limits<int32>::max();
    }

    if (curFps != fps)
    {
        double interval = 1.0 / fps;
        [frameTimer set:interval];
    }
}

void CoreNativeBridge::ApplicationWillFinishLaunching()
{
}

void CoreNativeBridge::ApplicationDidFinishLaunching(NSNotification* notification)
{
    core->engineBackend->OnGameLoopStarted();

    WindowBackend* primaryWindowBackend = PlatformCore::GetWindowBackend(core->engineBackend->GetPrimaryWindow());
    primaryWindowBackend->Create(640.0f, 480.0f);

    frameTimer = [[FrameTimer alloc] init:this];
    [frameTimer set:1.0 / 60.0];

    NotifyListeners(ON_DID_FINISH_LAUNCHING, notification, nullptr, nullptr);
}

void CoreNativeBridge::ApplicationDidChangeScreenParameters()
{
    Logger::Debug("****** CoreNativeBridge::ApplicationDidChangeScreenParameters");
}

void CoreNativeBridge::ApplicationDidBecomeActive()
{
    NotifyListeners(ON_DID_BECOME_ACTIVE, nullptr, nullptr, nullptr);
}

void CoreNativeBridge::ApplicationDidResignActive()
{
    NotifyListeners(ON_DID_RESIGN_ACTIVE, nullptr, nullptr, nullptr);
}

void CoreNativeBridge::ApplicationDidHide()
{
    core->didHideUnhide.Emit(true);
}

void CoreNativeBridge::ApplicationDidUnhide()
{
    core->didHideUnhide.Emit(false);
}

bool CoreNativeBridge::ApplicationShouldTerminate()
{
    if (!closeRequestByApp)
    {
        core->engineBackend->PostUserCloseRequest();
        return false;
    }

    if (!quitSent)
    {
        core->engineBackend->PostAppTerminate(false);
        return false;
    }
    return true;
}

bool CoreNativeBridge::ApplicationShouldTerminateAfterLastWindowClosed()
{
    return false;
}

void CoreNativeBridge::ApplicationWillTerminate()
{
    NotifyListeners(ON_WILL_TERMINATE, nullptr, nullptr, nullptr);

    [frameTimer cancel];

    core->engineBackend->OnGameLoopStopped();

    [[NSApplication sharedApplication] setDelegate:nil];
    [appDelegate release];
    [frameTimer release];

    int exitCode = core->engineBackend->GetExitCode();
    core->engineBackend->OnEngineCleanup();
    std::exit(exitCode);
}

void CoreNativeBridge::RegisterNSApplicationDelegateListener(NSApplicationDelegateListener* listener)
{
    DVASSERT(listener != nullptr);
    auto it = std::find(begin(appDelegateListeners), end(appDelegateListeners), listener);
    if (it == end(appDelegateListeners))
    {
        appDelegateListeners.push_back(listener);
    }
}

void CoreNativeBridge::UnregisterNSApplicationDelegateListener(NSApplicationDelegateListener* listener)
{
    auto it = std::find(begin(appDelegateListeners), end(appDelegateListeners), listener);
    if (it != end(appDelegateListeners))
    {
        appDelegateListeners.erase(it);
    }
}

void CoreNativeBridge::NotifyListeners(eNotificationType type, NSObject* arg1, NSObject* arg2, NSObject* arg3)
{
    for (auto i = begin(appDelegateListeners), e = end(appDelegateListeners); i != e;)
    {
        NSApplicationDelegateListener* l = *i;
        ++i;
        switch (type)
        {
        case ON_DID_FINISH_LAUNCHING:
            l->applicationDidFinishLaunching(static_cast<NSNotification*>(arg1));
            break;
        case ON_DID_BECOME_ACTIVE:
            l->applicationDidBecomeActive();
            break;
        case ON_DID_RESIGN_ACTIVE:
            l->applicationDidResignActive();
            break;
        case ON_WILL_TERMINATE:
            l->applicationWillTerminate();
            break;
        case ON_DID_RECEIVE_REMOTE_NOTIFICATION:
            l->didReceiveRemoteNotification(static_cast<NSApplication*>(arg1), static_cast<NSDictionary<NSString*, id>*>(arg2));
            break;
        case ON_DID_REGISTER_REMOTE_NOTIFICATION:
            l->didRegisterForRemoteNotificationsWithDeviceToken(static_cast<NSApplication*>(arg1), static_cast<NSData*>(arg2));
            break;
        case ON_DID_FAIL_TO_REGISTER_REMOTE_NOTIFICATION:
            l->didFailToRegisterForRemoteNotificationsWithError(static_cast<NSApplication*>(arg1), static_cast<NSError*>(arg2));
            break;
        default:
            break;
        }
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
