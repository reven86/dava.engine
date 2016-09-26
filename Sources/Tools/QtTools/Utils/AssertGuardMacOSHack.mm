#include "Base/Platform.h"
#import "AssertGuardMacOSHack.h"

#if defined(__DAVAENGINE_MACOS__)
#import <objc/runtime.h>
#import <AppKit/NSApplication.h>

namespace AssertGuardMacOSHackDetail
{
bool stopWasCalled = false;
}

@interface NSApplication (AssertCategory)

- (void)nsAppAssertStop:(id)sender;

@end

@implementation NSApplication (AssertCategory)
- (void)nsAppAssertStop:(id)sender
{
    AssertGuardMacOSHackDetail::stopWasCalled = true;
}
@end

MacOSRunLoopGuard::MacOSRunLoopGuard()
{
    AssertGuardMacOSHackDetail::stopWasCalled = false;
    Class cls = [NSApp class];

    SEL originalSelector = @selector(stop:);
    SEL swizzledSelector = @selector(nsAppAssertStop:);

    Method originalMethod = class_getInstanceMethod(cls, originalSelector);
    Method swizzledMethod = class_getInstanceMethod(cls, swizzledSelector);

    method_exchangeImplementations(originalMethod, swizzledMethod);
}

MacOSRunLoopGuard::~MacOSRunLoopGuard()
{
    Class cls = [NSApp class];

    SEL originalSelector = @selector(stop:);
    SEL swizzledSelector = @selector(nsAppAssertStop:);

    Method originalMethod = class_getInstanceMethod(cls, originalSelector);
    Method swizzledMethod = class_getInstanceMethod(cls, swizzledSelector);

    method_exchangeImplementations(swizzledMethod, originalMethod);

    if (AssertGuardMacOSHackDetail::stopWasCalled == true)
    {
        [NSApp stop:NSApp];
        AssertGuardMacOSHackDetail::stopWasCalled = false;
    }
}

#endif
