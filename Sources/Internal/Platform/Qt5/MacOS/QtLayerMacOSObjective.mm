#include "Platform/Qt5/QtLayer.h"


#if defined(__DAVAENGINE_MACOS__)

#import "AppKit/NSView.h"

namespace DAVA
{
void* QtLayer::CreateAutoreleasePool()
{
    NSAutoreleasePool* autoreleasePool = [[NSAutoreleasePool alloc] init];
    return autoreleasePool;
}

void QtLayer::ReleaseAutoreleasePool(void* pool)
{
    NSAutoreleasePool* autoreleasePool = reinterpret_cast<NSAutoreleasePool*>(pool);
    [autoreleasePool release];
}

void QtLayer::MakeAppForeground(bool foreground)
{
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    TransformProcessType(&psn, foreground ? kProcessTransformToForegroundApplication : kProcessTransformToBackgroundApplication);

    [NSApp activateIgnoringOtherApps:foreground ? YES : NO];

    if (foreground)
    {
        [[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateIgnoringOtherApps];
    }
}

void QtLayer::RestoreMenuBar()
{
    [[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateIgnoringOtherApps];
}
};

#endif // #if defined(__DAVAENGINE_MACOS__)
