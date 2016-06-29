#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/CoreOsXObjcBridge.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/OsX/CoreOsx.h"
#include "Engine/Private/OsX/WindowOsX.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"

#include "Engine/Private/OsX/OsXAppDelegate.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

// Wrapper over NSTimer to connect Objective-C NSTimer object to
// C++ class CoreOsXObjcBridge
@interface FrameTimer : NSObject
{
    DAVA::Private::CoreOsXObjcBridge* bridge;
    NSTimer* timer;
}

- (id)init:(DAVA::Private::CoreOsXObjcBridge*)objcBridge;
- (void)set:(double)interval;
- (void)cancel;
- (void)timerFired:(NSTimer*)timer;

@end

@implementation FrameTimer

- (id)init:(DAVA::Private::CoreOsXObjcBridge*)objcBridge
{
    self = [super init];
    if (self != nil)
    {
        bridge = objcBridge;
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
CoreOsXObjcBridge::CoreOsXObjcBridge(CoreOsX* coreOsX)
    : core(coreOsX)
{
}

CoreOsXObjcBridge::~CoreOsXObjcBridge()
{
    [[NSApplication sharedApplication] setDelegate:nil];
    [appDelegate release];
    [frameTimer release];
}

void CoreOsXObjcBridge::InitNSApplication()
{
    [NSApplication sharedApplication];

    appDelegate = [[OsXAppDelegate alloc] init:this];
    [[NSApplication sharedApplication] setDelegate:(id<NSApplicationDelegate>)appDelegate];
}

void CoreOsXObjcBridge::Quit()
{
    if (!quitSent)
    {
        quitSent = true;
        [[NSApplication sharedApplication] terminate:nil];
    }
}

void CoreOsXObjcBridge::OnFrameTimer()
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

void CoreOsXObjcBridge::ApplicationWillFinishLaunching()
{
}

void CoreOsXObjcBridge::ApplicationDidFinishLaunching()
{
    core->engineBackend->OnGameLoopStarted();
    core->CreateNativeWindow(core->engineBackend->GetPrimaryWindow(), 640.0f, 480.0f);

    frameTimer = [[FrameTimer alloc] init:this];
    [frameTimer set:1.0 / 60.0];
}

void CoreOsXObjcBridge::ApplicationDidChangeScreenParameters()
{
    Logger::Debug("****** CoreOsXObjcBridge::ApplicationDidChangeScreenParameters");
}

void CoreOsXObjcBridge::ApplicationDidBecomeActive()
{
}

void CoreOsXObjcBridge::ApplicationDidResignActive()
{
}

void CoreOsXObjcBridge::ApplicationDidHide()
{
    core->didHideUnhide.Emit(true);
}

void CoreOsXObjcBridge::ApplicationDidUnhide()
{
    core->didHideUnhide.Emit(false);
}

bool CoreOsXObjcBridge::ApplicationShouldTerminate()
{
    if (!quitSent)
    {
        core->engineBackend->PostAppTerminate();
        return false;
    }
    return true;
}

bool CoreOsXObjcBridge::ApplicationShouldTerminateAfterLastWindowClosed()
{
    return false;
}

void CoreOsXObjcBridge::ApplicationWillTerminate()
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
