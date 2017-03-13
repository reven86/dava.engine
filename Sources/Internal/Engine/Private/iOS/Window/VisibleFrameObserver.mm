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
                      name:UIKeyboardWillShowNotification
                    object:nil];
    [super dealloc];
}

- (void)updateVisibleFrame:(const CGRect*)keyboardFrame
{
    if (bridge->uiwindow == nil || bridge->renderView == nil)
    {
        // Skip notification if window not initialized yet
        return;
    }

    // Convert renverView frame to window coordinates, frame is in superview's coordinates
    CGRect visibleFrame = [bridge->uiwindow convertRect:bridge->renderView.frame fromView:bridge->renderView];
    if (keyboardFrame != nil)
    {
        visibleFrame.size.height = keyboardFrame->origin.y - visibleFrame.origin.y;
    }
    // Now this might be rotated, so convert it back
    visibleFrame = [bridge->renderView convertRect:visibleFrame toView:bridge->uiwindow];

    bridge->mainDispatcher->PostEvent(DAVA::Private::MainDispatcherEvent::CreateWindowVisibleFrameChangedEvent(bridge->window, visibleFrame.origin.x, visibleFrame.origin.y, visibleFrame.size.width, visibleFrame.size.height));
}

- (void)keyboardFrameDidChange:(NSNotification*)notification
{
    CGRect keyboardFrame = [[notification.userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    [self updateVisibleFrame:&keyboardFrame];
}

- (void)keyboardWillShow:(NSNotification*)notification
{
    CGRect keyboardFrame = [[notification.userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    [self updateVisibleFrame:&keyboardFrame];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self
               selector:@selector(keyboardFrameDidChange:)
                   name:UIKeyboardDidChangeFrameNotification
                 object:nil];
    [center addObserver:self
               selector:@selector(keyboardWillHide:)
                   name:UIKeyboardWillHideNotification
                 object:nil];
}

- (void)keyboardWillHide:(NSNotification*)notification
{
    CGRect keyboardFrame = [[notification.userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    [self updateVisibleFrame:nil];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self
                      name:UIKeyboardDidChangeFrameNotification
                    object:nil];
    [center removeObserver:self
                      name:UIKeyboardWillHideNotification
                    object:nil];
}

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
