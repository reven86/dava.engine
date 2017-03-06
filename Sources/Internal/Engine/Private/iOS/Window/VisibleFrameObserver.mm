#import "Engine/Private/iOS/Window/VisibleFrameObserver.h"

#include "Engine/Private/iOS/Window/WindowBackendiOS.h"

@implementation VisibleFrameObserver

- (id)initWithBridge:(DAVA::Private::WindowNativeBridge*)nativeBridge
{
    self = [super init];
    if (self != nil)
    {
        bridge = nativeBridge;
    }

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self
               selector:@selector(keyboardFrameDidChange:)
                   name:UIKeyboardDidChangeFrameNotification
                 object:nil];

    return self;
}

- (void)dealloc
{
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self
                      name:UIKeyboardDidChangeFrameNotification
                    object:nil];
    [super dealloc];
}

- (void)keyboardFrameDidChange:(NSNotification*)notification
{
    // Convert renverView frame to window coordinates, frame is in superview's coordinates
    CGRect renderFrame = [bridge->uiwindow convertRect:bridge->renderView.frame fromView:bridge->renderView];
    CGRect keyboardFrame = [[notification.userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    CGRect topFrame = CGRectMake(renderFrame.origin.x, renderFrame.origin.y, renderFrame.size.width, keyboardFrame.origin.y - renderFrame.origin.y);
    CGRect bottomFrame = CGRectMake(renderFrame.origin.x, keyboardFrame.origin.y + keyboardFrame.size.height, renderFrame.size.width, renderFrame.size.height - (keyboardFrame.origin.y + keyboardFrame.size.height));

    CGRect visibleFrame;
    if (topFrame.size.width * topFrame.size.height > bottomFrame.size.width * bottomFrame.size.height)
    {
        visibleFrame = topFrame;
    }
    else
    {
        visibleFrame = bottomFrame;
    }

    // Now this might be rotated, so convert it back
    visibleFrame = [bridge->renderView convertRect:visibleFrame toView:bridge->uiwindow];

    // Recalculate to virtual coordinates
    DAVA::Rect r(visibleFrame.origin.x, visibleFrame.origin.y, visibleFrame.size.width, visibleFrame.size.height);
    bridge->mainDispatcher->PostEvent(DAVA::Private::MainDispatcherEvent::CreateWindowVisibleFrameChangedEvent(bridge->window, r.x, r.y, r.dx, r.dy));
}
@end
