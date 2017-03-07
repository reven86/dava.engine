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
    CGRect keyboardFrame = [[notification.userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    CGRect visibleFrame = [bridge->uiwindow convertRect:bridge->renderView.frame fromView:bridge->renderView];

    float32 topHeight = keyboardFrame.origin.y - visibleFrame.origin.y;
    float32 bottomHeight = visibleFrame.size.height - (keyboardFrame.origin.y + keyboardFrame.size.height);
    if (topHeight > bottomHeight)
    {
        visibleFrame.size.height = topHeight;
    }
    else
    {
        visibleFrame.origin.y = keyboardFrame.origin.y + keyboardFrame.size.height;
        visibleFrame.size.height = bottomHeight;
    }

    // Now this might be rotated, so convert it back
    visibleFrame = [bridge->renderView convertRect:visibleFrame toView:bridge->uiwindow];

    // Recalculate to virtual coordinates
    DAVA::Rect r(visibleFrame.origin.x, visibleFrame.origin.y, visibleFrame.size.width, visibleFrame.size.height);
    bridge->mainDispatcher->PostEvent(DAVA::Private::MainDispatcherEvent::CreateWindowVisibleFrameChangedEvent(bridge->window, r.x, r.y, r.dx, r.dy));
}
@end
