#pragma once

#import <Foundation/NSObject.h>
#import <Foundation/NSNotification.h>

namespace DAVA
{
namespace Private
{
class WindowNativeBridge;
}
}

@interface VisibleFrameObserver : NSObject
{
    DAVA::Private::WindowNativeBridge* bridge;
}

- (id)initWithBridge:(DAVA::Private::WindowNativeBridge*)nativeBridge;
- (void)dealloc;
- (void)keyboardFrameDidChange:(NSNotification*)notification;

@end
