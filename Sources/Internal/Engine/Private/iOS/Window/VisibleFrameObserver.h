#pragma once

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

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

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
