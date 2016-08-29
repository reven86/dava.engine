#include "MacOSUtils.h"

#if defined(__DAVAENGINE_MACOS__)
#import "AppKit/NSView.h"

namespace DAVA
{
namespace TArc
{
void MakeAppForeground()
{
    id activeApp = nil;
    NSArray *runningApps;
    runningApps = [[NSWorkspace sharedWorkspace] runningApplications];
    for (id currApp in runningApps)
    {
        if ([currApp isActive])
        {
            activeApp = currApp;
            break;
        }
    }
    
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    [activeApp activateWithOptions:NSApplicationActivateIgnoringOtherApps];
    [[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateIgnoringOtherApps];
}
} // namespace TArc
} // namespace DAVA
#endif