#include "QtThread.h"

#if defined(__DAVAENGINE_MACOS__)

#import "AppKit/NSView.h"

void QtThread::run()
{
    NSAutoreleasePool* autoreleasePool = [[NSAutoreleasePool alloc] init];

    QThread::exec();

    [autoreleasePool release];
}

#endif