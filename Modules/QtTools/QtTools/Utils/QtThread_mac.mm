#include "QtThread.h"

#if defined(__DAVAENGINE_MACOS__)

#import "AppKit/NSView.h"

void QtThread::run()
{
    @autoreleasepool
    {
        QThread::exec();
    }
}

#endif