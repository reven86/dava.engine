#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/CoreNativeBridgeOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Window.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/OsX/PlatformCoreOsx.h"
#include "Engine/Private/OsX/Window/WindowBackendOsX.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

#include "Engine/Private/OsX/AppDelegateOsX.h"

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
    , mainDispatcher(core->engineBackend->GetDispatcher())
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
    NSUserNotification* userNotification = [notification userInfo][(id) @"NSApplicationLaunchUserNotificationKey"];
    if (userNotification.userInfo != nil)
    {
        NSString* uid = [[userNotification userInfo] valueForKey:@"uid"];
        if (uid != nil && [uid length] != 0)
        {
            DAVA::String uidStr = DAVA::StringFromNSString(uid);
            mainDispatcher->PostEvent(DAVA::Private::MainDispatcherEvent::CreateLocalNotificationEvent(uidStr));
        }
    }

    core->engineBackend->OnGameLoopStarted();

    WindowBackend* primaryWindowBackend = PlatformCore::GetWindowBackend(core->engineBackend->GetPrimaryWindow());
    primaryWindowBackend->Create(640.0f, 480.0f);

    frameTimer = [[FrameTimer alloc] init:this];
    [frameTimer set:1.0 / 60.0];
}

void CoreNativeBridge::ApplicationDidChangeScreenParameters()
{
    Logger::Debug("****** CoreNativeBridge::ApplicationDidChangeScreenParameters");
}

void CoreNativeBridge::ApplicationDidBecomeActive()
{
}

void CoreNativeBridge::ApplicationDidResignActive()
{
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
    [frameTimer cancel];

    core->engineBackend->OnGameLoopStopped();

    [[NSApplication sharedApplication] setDelegate:nil];
    [appDelegate release];
    [frameTimer release];

    int exitCode = core->engineBackend->GetExitCode();
    core->engineBackend->OnEngineCleanup();
    std::exit(exitCode);
}

void CoreNativeBridge::ApplicationDidActivateNotification(NSUserNotification* notification)
{
    NSString* uid = [[notification userInfo] valueForKey:@"uid"];
    if (uid != nil && [uid length] != 0)
    {
        DAVA::String uidStr = DAVA::StringFromNSString(uid);
        bridge->mainDispatcher->PostEvent(DAVA::Private::MainDispatcherEvent::CreateLocalNotificationEvent(uidStr));
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
        DAVA::Engine::Instance()->PrimaryWindow()->GetNativeService()->DoWindowDeminiaturize();
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
