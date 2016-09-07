#include "Utils.h"

#if defined(__DAVAENGINE_MACOS__)

#import "AppKit/NSView.h"

id prevActiveApp = nil;

void MakeAppForeground()
{
    NSArray* runningApps;
    runningApps = [[NSWorkspace sharedWorkspace] runningApplications];
    for (id currApp in runningApps)
    {
        if ([currApp isActive])
        {
            prevActiveApp = currApp;
            break;
        }
    }

    ProcessSerialNumber psn = { 0, kCurrentProcess };
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    [[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateIgnoringOtherApps];
}

void RestoreMenuBar()
{
    [prevActiveApp activateWithOptions:NSApplicationActivateIgnoringOtherApps];
    [[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateIgnoringOtherApps];
}

#endif // __DAVAENGINE_MACOS__