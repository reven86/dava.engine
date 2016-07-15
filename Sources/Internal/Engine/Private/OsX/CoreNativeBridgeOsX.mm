#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/CoreNativeBridgeOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/OsX/PlatformCoreOsx.h"
#include "Engine/Private/OsX/Window/WindowBackendOsX.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

#include "Engine/Private/OsX/AppDelegateOsX.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

// Wrapper over NSTimer to connect Objective-C NSTimer object to
// C++ class CoreNativeBridgeOsX
@interface FrameTimer : NSObject
{
    DAVA::Private::CoreNativeBridgeOsX* bridge;
    NSTimer* timer;
}

- (id)init:(DAVA::Private::CoreNativeBridgeOsX*)nativeBridge;
- (void)set:(double)interval;
- (void)cancel;
- (void)timerFired:(NSTimer*)timer;

@end

@implementation FrameTimer

- (id)init:(DAVA::Private::CoreNativeBridgeOsX*)nativeBridge
{
    self = [super init];
    if (self != nil)
    {
        bridge = nativeBridge;
    }
    return self;
}

- (void)set:(double)interval
{
    [self cancel];
    timer = [NSTimer scheduledTimerWithTimeInterval:interval
                                             target:self
                                           selector:@selector(timerFired:)
                                           userInfo:nil
                                            repeats:NO];
}

- (void)cancel
{
    [timer invalidate];
    //[timer release];  // ?????
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
CoreNativeBridgeOsX::CoreNativeBridgeOsX(PlatformCore* c)
    : core(c)
{
}

CoreNativeBridgeOsX::~CoreNativeBridgeOsX()
{
    [[NSApplication sharedApplication] setDelegate:nil];
    [appDelegate release];
    [frameTimer release];
}

void CoreNativeBridgeOsX::InitNSApplication()
{
    [NSApplication sharedApplication];

    appDelegate = [[OsXAppDelegate alloc] init:this];
    [[NSApplication sharedApplication] setDelegate:(id<NSApplicationDelegate>)appDelegate];
}

void CoreNativeBridgeOsX::Quit()
{
    if (!quitSent)
    {
        quitSent = true;
        [[NSApplication sharedApplication] terminate:nil];
    }
}

void CoreNativeBridgeOsX::OnFrameTimer()
{
    int32 fps = core->OnFrame();
    if (fps <= 0)
    {
        // To prevent division by zero
        fps = std::numeric_limits<int32>::max();
    }

    double interval = 1.0 / fps;
    [frameTimer set:interval];
}

void CoreNativeBridgeOsX::ApplicationWillFinishLaunching()
{
}

void CoreNativeBridgeOsX::ApplicationDidFinishLaunching()
{
    core->engineBackend->OnGameLoopStarted();
    core->CreateNativeWindow(core->engineBackend->GetPrimaryWindow(), 640.0f, 480.0f);

    frameTimer = [[FrameTimer alloc] init:this];
    [frameTimer set:1.0 / 60.0];
}

void CoreNativeBridgeOsX::ApplicationDidChangeScreenParameters()
{
    Logger::Debug("****** CoreNativeBridgeOsX::ApplicationDidChangeScreenParameters");
}

void CoreNativeBridgeOsX::ApplicationDidBecomeActive()
{
}

void CoreNativeBridgeOsX::ApplicationDidResignActive()
{
}

void CoreNativeBridgeOsX::ApplicationDidHide()
{
    core->didHideUnhide.Emit(true);
}

void CoreNativeBridgeOsX::ApplicationDidUnhide()
{
    core->didHideUnhide.Emit(false);
}

bool CoreNativeBridgeOsX::ApplicationShouldTerminate()
{
    if (!quitSent)
    {
        core->engineBackend->PostAppTerminate();
        return false;
    }
    return true;
}

bool CoreNativeBridgeOsX::ApplicationShouldTerminateAfterLastWindowClosed()
{
    return false;
}

void CoreNativeBridgeOsX::ApplicationWillTerminate()
{
    [frameTimer cancel];

    core->engineBackend->OnGameLoopStopped();

    [[NSApplication sharedApplication] setDelegate:nil];
    [appDelegate release];
    [frameTimer release];

    int exitCode = core->engineBackend->GetExitCode();
    core->engineBackend->OnBeforeTerminate();
    std::exit(exitCode);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
