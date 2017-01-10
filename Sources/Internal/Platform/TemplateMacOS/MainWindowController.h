#import <Cocoa/Cocoa.h>
#import <IOKit/pwr_mgt/IOPMLib.h>

#import "OpenGLView.h"
#import "AppDelegate.h"

#if !defined(__DAVAENGINE_COREV2__)

@interface MainWindowController : NSWindowController<NSWindowDelegate, NSFileManagerDelegate>
{
@public
    DAVA::float32 currFPS;
    OpenGLView* openGLView;
    NSWindow* mainWindow;
    NSTimer* animationTimer;

    DAVA::ApplicationCore* core;
    bool fullScreen;
    bool willQuit;

@private
    IOPMAssertionID assertionID;
}

@property(assign) bool willQuit;

- (void)createWindows;

- (bool)isFullScreen;
- (bool)setFullScreen:(bool)_fullScreen;

- (void)allowDisplaySleep:(bool)sleep;

- (void)windowWillMiniaturize:(NSNotification*)notification;
- (void)windowDidDeminiaturize:(NSNotification*)notification;

- (void)windowDidEnterFullScreen:(NSNotification*)notification;
- (void)windowDidExitFullScreen:(NSNotification*)notification;

- (void)windowDidBecomeKey:(NSNotification*)notification;
- (void)windowDidResignKey:(NSNotification*)notification;

- (void)OnSuspend;
- (void)OnResume;

@end

#endif // #if !defined(__DAVAENGINE_COREV2__)
