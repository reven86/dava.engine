#include "Base/BaseTypes.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#if defined(__DAVAENGINE_MACOS__)

#include "Platform/TemplateMacOS/CorePlatformMacOS.h"

#import "HelperAppDelegate.h"

using namespace DAVA;

extern void FrameworkWillTerminate();

@implementation HelperAppDelegate

#include "Core/Core.h"
#include "UI/UIScreenManager.h"
#include "Platform/Steam.h"

- (void)setWindowController:(MainWindowController*)ctrlr
{
    mainWindowController = ctrlr;
}

- (void)applicationWillFinishLaunching:(NSNotification*)aNotification
{
    DAVA::Logger::FrameworkDebug("[CoreMacOSPlatform] Application will finish launching: %s", [[[NSBundle mainBundle] bundlePath] UTF8String]);
    [mainWindowController createWindows];
    Core::Instance()->SystemAppStarted();
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application did finish launching");

    [mainWindowController OnResume];
}

- (BOOL)application:(NSApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    return YES;
}

- (void)applicationWillBecomeActive:(NSNotification*)aNotification
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application will become active");
}

- (void)applicationDidBecomeActive:(NSNotification*)aNotification
{
    DAVA::Logger::FrameworkDebug("[CoreMacOSPlatform] Application did become active");

    [mainWindowController OnResume];
}

- (void)applicationDidResignActive:(NSNotification*)aNotification
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application did resign active");

    [mainWindowController OnSuspend];
}

- (void)applicationDidChangeScreenParameters:(NSNotification*)aNotification
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application did change screen params");
}

- (void)applicationWillResignActive:(NSApplication*)application
{
}

- (void)applicationDidEnterBackground:(NSApplication*)application
{
}

- (void)applicationWillEnterForeground:(NSApplication*)application
{
}

- (void)applicationDidHide:(NSNotification*)aNotification
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application did hide");

    CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
    xcore->signalAppMinimizedRestored.Emit(true);
}

- (void)applicationDidUnhide:(NSNotification*)aNotification
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application did unhide");

    CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
    xcore->signalAppMinimizedRestored.Emit(false);
}

- (void)windowWillClose:(NSNotification*)notification
{
    [[NSApplication sharedApplication] terminate:nil];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application should terminate");

    mainWindowController->willQuit = true;

    Core::Instance()->SystemAppFinished();
    FrameworkWillTerminate();
#if defined(__DAVAENGINE_STEAM__)
    Steam::Deinit();
#endif

    // Wait job completion before releasing singletons
    // But client should stop its jobs in GameCore::OnAppFinished or in FrameworkWillTerminate
    JobManager::Instance()->WaitWorkerJobs();
    JobManager::Instance()->WaitMainJobs();

    Core::Instance()->ReleaseSingletons();

    NSLog(@"[CoreMacOSPlatform] Application has terminated");
    return NSTerminateNow;
}

- (void)applicationWillTerminate:(NSApplication*)application
{
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    return YES;
}

@end
#endif
