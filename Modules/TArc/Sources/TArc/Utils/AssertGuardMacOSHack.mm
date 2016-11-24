#if defined(__DAVAENGINE_MACOS__)
#import "AssertGuardMacOSHack.h"

#import <objc/runtime.h>
#import <AppKit/NSApplication.h>

#include <algorithm>

namespace AssertGuardMacOSHackDetail
{
bool stopWasCalled = false;

void SwapMethodImplementation(bool isReversSwap)
{
    Class cls = [NSApp class];

    SEL originalSelector = @selector(stop:);
    SEL swizzledSelector = @selector(nsAppAssertStop:);

    Method originalMethod = class_getInstanceMethod(cls, originalSelector);
    Method swizzledMethod = class_getInstanceMethod(cls, swizzledSelector);

    if (isReversSwap)
    {
        std::swap(originalMethod, swizzledMethod);
    }

    method_exchangeImplementations(originalMethod, swizzledMethod);
}
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
    AssertGuardMacOSHackDetail::SwapMethodImplementation(false);
}

MacOSRunLoopGuard::~MacOSRunLoopGuard()
{
    AssertGuardMacOSHackDetail::SwapMethodImplementation(true);
    if (AssertGuardMacOSHackDetail::stopWasCalled == true)
    {
        [NSApp stop:NSApp];
        AssertGuardMacOSHackDetail::stopWasCalled = false;
    }
}

#endif
