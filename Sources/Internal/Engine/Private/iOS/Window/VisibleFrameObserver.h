#pragma once

#import <UIKit/UIKit.h>

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
