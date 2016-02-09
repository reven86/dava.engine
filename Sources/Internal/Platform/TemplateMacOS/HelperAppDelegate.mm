/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Base/BaseTypes.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#if defined(__DAVAENGINE_MACOS__)

#include "Platform/TemplateMacOS/CorePlatformMacOS.h"

#import "HelperAppDelegate.h"

extern void FrameworkWillTerminate();

@implementation HelperAppDelegate

#include "Core/Core.h"
#include "UI/UIScreenManager.h"

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
    [mainWindowController OnSuspend];
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

    mainWindowController->openGLView.willQuit = true;

    Core::Instance()->SystemAppFinished();
    FrameworkWillTerminate();
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
