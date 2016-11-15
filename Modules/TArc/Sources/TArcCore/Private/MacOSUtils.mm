#include "MacOSUtils.h"

#if defined(__DAVAENGINE_MACOS__)
#import "AppKit/NSView.h"

#include <QFont>
#include <QSysInfo>

namespace DAVA
{
namespace TArc
{
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

void FixOSXFonts()
{
    if (QSysInfo::MacintoshVersion != QSysInfo::MV_None && QSysInfo::MacintoshVersion > QSysInfo::MV_10_8)
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
}

} // namespace TArc
} // namespace DAVA
#endif