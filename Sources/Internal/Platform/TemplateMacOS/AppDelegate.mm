#import "AppDelegate.h"

#if defined(__DAVAENGINE_MACOS__)

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification
{
    NSLog(@"applicationDidFinishLaunching: notification");
}

- (void)orderFrontStandardAboutPanel:(id)sender
{
    // [NSApp orderFrontStandardAboutPanel:sender];
}

- (void)unhideAllApplications:(id)sender
{
    // [NSApp unhideAllApplications:sender];
}

- (void)hide:(id)sender
{
    // [NSApp hide:sender];
}

- (void)hideOtherApplications:(id)sender
{
    // [NSApp hideOtherApplications:sender];
}

- (void)terminate:(id)sender
{
    // _quit = TRUE;
}

- (void)windowWillClose:(id)sender
{
    //	_quit = TRUE;
}

- (NSSize)windowWillResize:(NSWindow*)window toSize:(NSSize)proposedFrameSize
{
    //	if (_device->isResizable())
    //		return proposedFrameSize;
    //	else
    //		return [window frame].size;
    return proposedFrameSize;
}

- (void)windowDidResize:(NSNotification*)aNotification
{
    //	NSWindow	*window;
    //	NSRect		frame;
    //
    //	window = [aNotification object];
    //	frame = [window frame];
    //	_device->setResize((int)frame.size.width,(int)frame.size.height);
}

//- (BOOL)isQuit
//{
//	return (_quit);
//}

@end

#endif // __DAVAENGINE_MACOS__
