#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/iOS/Window/WindowNativeBridgeiOS.h"

#import "Engine/Private/iOS/Window/VisibleFrameObserver.h"
#import "Engine/Private/iOS/Window/RenderViewiOS.h"

#import <UIKit/UIKit.h>

@implementation VisibleFrameObserver

- (id)initWithBridge:(DAVA::Private::WindowNativeBridge*)nativeBridge
{
    self = [super init];
    if (self != nil)
    {
        bridge = nativeBridge;

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self
                   selector:@selector(keyboardWillShow:)
                       name:UIKeyboardWillShowNotification
                     object:nil];
    }
    return self;
}

- (void)dealloc
{
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self
                      name:UIKeyboardDidShowNotification
                    object:nil];
    [super dealloc];
}

- (void)fireWithKeyboardFrame:(const CGRect*)keyboardFrame
{
    if (bridge->uiwindow == nil || bridge->renderView == nil)
    {
        // Skip notification if window not initialized yet
        return;
    }

    CGRect visibleFrame = bridge->renderView.frame;
    if (keyboardFrame != nil)
    {
        // Convert renverView frame to window coordinates, frame is in superview's coordinates
        visibleFrame = [bridge->uiwindow convertRect:visibleFrame fromView:bridge->renderView];
        visibleFrame.size.height = keyboardFrame->origin.y - visibleFrame.origin.y;
        // Now this might be rotated, so convert it back
        visibleFrame = [bridge->uiwindow convertRect:visibleFrame toView:bridge->renderView];
    }

    bridge->mainDispatcher->PostEvent(DAVA::Private::MainDispatcherEvent::CreateWindowVisibleFrameChangedEvent(bridge->window, visibleFrame.origin.x, visibleFrame.origin.y, visibleFrame.size.width, visibleFrame.size.height));
}

- (void)keyboardWillShow:(NSNotification*)notification
{
    CGRect keyboardFrame = [[notification.userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    [self fireWithKeyboardFrame:&keyboardFrame];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self
               selector:@selector(keyboardWillHide:)
                   name:UIKeyboardWillHideNotification
                 object:nil];
}

- (void)keyboardWillHide:(NSNotification*)notification
{
    CGRect keyboardFrame = [[notification.userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    [self fireWithKeyboardFrame:nil];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self
                      name:UIKeyboardWillHideNotification
                    object:nil];
}

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
