/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#import "MainWindowController.h"
#include "CorePlatformMacOS.h"
#include "Platform/DeviceInfo.h"
#include "Render/2D/Systems/RenderSystem2D.h"

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

namespace DAVA 
{
	int Core::Run(int argc, char *argv[], AppHandle handle)
	{
		NSAutoreleasePool * globalPool = 0;
		globalPool = [[NSAutoreleasePool alloc] init];
		DAVA::CoreMacOSPlatform * core = new DAVA::CoreMacOSPlatform();
		core->SetCommandLine(argc, argv);
		core->CreateSingletons();

		[[NSApplication sharedApplication] setDelegate:(id<NSApplicationDelegate>)[[[MainWindowController alloc] init] autorelease]];

        int retVal = NSApplicationMain(argc, (const char**)argv);
        // This method never returns, so release code transfered to termination message 
        // - (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
        // core->ReleaseSingletons() is called from there

        [globalPool release];
        globalPool = 0;
        return retVal;
    }

    int Core::RunCmdTool(int argc, char* argv[], AppHandle handle)
    {
        NSAutoreleasePool* globalPool = 0;
        globalPool = [[NSAutoreleasePool alloc] init];
        DAVA::CoreMacOSPlatform* core = new DAVA::CoreMacOSPlatform();
        core->SetCommandLine(argc, argv);
        core->EnableConsoleMode();
        core->CreateSingletons();

        Logger::Instance()->EnableConsoleMode();

        FrameworkDidLaunched();
        FrameworkWillTerminate();

        core->ReleaseSingletons();

        [globalPool release];
        globalPool = 0;
        return 0;
    }
}

@interface MainWindowController ()
- (void) startAnimationTimer;
- (void) stopAnimationTimer;
- (void) animationTimerFired:(NSTimer *)timer;

- (void)windowWillMiniaturize:(NSNotification *)notification;
- (void)windowDidMiniaturize:(NSNotification *)notification;
- (void)windowDidDeminiaturize:(NSNotification *)notification;
@end

@implementation MainWindowController

static MainWindowController * mainWindowController = nil;

- (id)init
{
    self = [super init];
	if (self)
	{
		mainWindowController = self;
		openGLView = nil;
		mainWindow = nil;
		animationTimer = nil;
		core = 0;

	}
	return self;
}

- (void)dealloc
{
	[super dealloc];
}

-(void)createWindows
{
    NSRect displayRect = [[NSScreen mainScreen] frame];
	
	FrameworkDidLaunched();

    String title;
    int32 width = 800;
    int32 height = 600;
    bool isFull = false;

    KeyedArchive * options = DAVA::Core::Instance()->GetOptions();
    if(nullptr != options)
    {
        title = options->GetString("title", "[set application title using core options property 'title']");
        if(options->IsKeyExists("width") && options->IsKeyExists("height"))
        {
            width = options->GetInt32("width");
            height = options->GetInt32("height");
        }
        
        isFull = (0 != options->GetInt32("fullscreen", 0));
    }
    
    openGLView = [[OpenGLView alloc]initWithFrame: NSMakeRect(0, 0, width, height)];
    
    NSUInteger wStyle = NSTitledWindowMask + NSMiniaturizableWindowMask + NSClosableWindowMask + NSResizableWindowMask;
    NSRect wRect = NSMakeRect((displayRect.size.width - width) / 2, (displayRect.size.height - height) / 2, width, height);
    mainWindow = [[NSWindow alloc] initWithContentRect:wRect styleMask:wStyle backing:NSBackingStoreBuffered defer:FALSE];
    [mainWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [mainWindow setDelegate:self];
    [mainWindow setContentView: openGLView];
    [mainWindow setContentSize: NSMakeSize(width, height)];
    
    willQuit = false;
    
    core = Core::GetApplicationCore();
    Core::Instance()->SetNativeView(openGLView);

// start animation
    currFPS = Renderer::GetDesiredFPS();
    [self startAnimationTimer];

    // make window main
    [mainWindow makeKeyAndOrderFront:nil];
    [mainWindow setTitle:[NSString stringWithFormat:@"%s", title.c_str()]];
    [mainWindow setAcceptsMouseMovedEvents:YES];
    
    if(isFull)
    {
        [self setFullScreen:true];
    }
}

- (void)windowWillMiniaturize:(NSNotification *)notification
{
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    [self OnSuspend];
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    [self OnResume];
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    Core::Instance()->FocusReceived();
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    Core::Instance()->FocusLost();
    InputSystem::Instance()->GetKeyboard().ClearAllKeys();
}

- (void)windowDidEnterFullScreen:(NSNotification *)notification
{
    fullScreen = true;
    Core::Instance()->GetApplicationCore()->OnEnterFullscreen();
}

- (void)windowDidExitFullScreen:(NSNotification *)notification
{
    fullScreen = false;
    Core::Instance()->GetApplicationCore()->OnExitFullscreen();
}

-(bool) isFullScreen
{
    return fullScreen;
}

- (bool)setFullScreen:(bool)_fullScreen
{
    if (fullScreen != _fullScreen)
    {
        double macOSVer = floor(NSAppKitVersionNumber);
        // fullscreen for new 10.7+ MacOS
        if(macOSVer >= NSAppKitVersionNumber10_7)
        {
            // just toggle current state
            // fullScreen variable will be set in windowDidEnterFullScreen/windowDidExitFullScreen callbacks
            [mainWindowController->mainWindow toggleFullScreen: nil];
            return YES;
        }
        else
        {
            // fullscreen for older macOS isn't supperted
            DVASSERT_MSG(false, "Fullscreen isn't supported for this MacOS version");
            return NO;
        }
    }
    return YES;
}

- (void) keyDown:(NSEvent *)event
{
	[openGLView keyDown:event];
}

- (void) keyUp:(NSEvent *)event
{
	[openGLView keyUp:event];
}

- (void) flagsChanged :(NSEvent *)event
{
	[openGLView flagsChanged:event];
}

- (void)mouseDown:(NSEvent *)theEvent
{
	[openGLView mouseDown:theEvent];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    [openGLView scrollWheel:theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
	[openGLView mouseMoved:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent
{
	[openGLView mouseUp:theEvent];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	[openGLView mouseDragged:theEvent];
}

- (void)mouseEntered:(NSEvent *)theEvent
{
	NSLog(@"mouse ENTERED");
}
- (void)mouseExited:(NSEvent *)theEvent
{
	NSLog(@"mouse EXITED");
}
- (void)rightMouseDown:(NSEvent *)theEvent
{
	[openGLView rightMouseDown:theEvent];
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
	[openGLView rightMouseDragged:theEvent];
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
	[openGLView rightMouseUp:theEvent];
}

- (void)otherMouseDown:(NSEvent *)theEvent
{
	[openGLView otherMouseDown:theEvent];
}

- (void)otherMouseDragged:(NSEvent *)theEvent
{
	[openGLView otherMouseDragged:theEvent];
}

- (void)otherMouseUp:(NSEvent *)theEvent
{
	[openGLView otherMouseUp:theEvent];
}

- (void) startAnimationTimer
{
    if (animationTimer == nil) 
	{
        animationTimer = [[NSTimer scheduledTimerWithTimeInterval:1.0f / currFPS target:self selector:@selector(animationTimerFired:) userInfo:nil repeats:YES] retain];
    }
}

- (void) stopAnimationTimer
{
    if (animationTimer != nil) 
	{
        [animationTimer invalidate];
        [animationTimer release];
        animationTimer = nil;
    }
}

- (void) animationTimerFired:(NSTimer *)timer
{
    if(willQuit)
    {
        [self stopAnimationTimer];
        return;
    }
    
    DAVA::Core::Instance()->SystemProcessFrame();
    [openGLView setNeedsDisplay:YES];
    
    if (currFPS != Renderer::GetDesiredFPS())
    {
        currFPS = Renderer::GetDesiredFPS();
        [self stopAnimationTimer];
        [self startAnimationTimer];
    }
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	NSLog(@"[CoreMacOSPlatform] Application did finish launching");	
    
    [self OnResume];
}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
	[self createWindows];
	NSLog(@"[CoreMacOSPlatform] Application will finish launching: %@", [[NSBundle mainBundle] bundlePath]);

    NSSize windowSize = [openGLView frame].size;
    float32 backingScale = Core::Instance()->GetScreenScaleFactor();
    
    GLint backingSize[2] = {GLint(windowSize.width * backingScale), GLint(windowSize.height * backingScale)};
    CGLSetParameter([[openGLView openGLContext] CGLContextObj], kCGLCPSurfaceBackingSize, backingSize);
    CGLEnable([[openGLView openGLContext] CGLContextObj], kCGLCESurfaceBackingSize);
    CGLUpdateContext([[openGLView openGLContext] CGLContextObj]);
    
    rhi::InitParam & rendererParams = Core::Instance()->rendererParams;
    rendererParams.window = mainWindowController->openGLView;
    rendererParams.width = backingSize[0];
    rendererParams.height = backingSize[1];

    VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(windowSize.width, windowSize.height);
    VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(backingSize[0], backingSize[1]);
    
    Core::Instance()->SystemAppStarted();
}

- (void)applicationWillBecomeActive:(NSNotification *)aNotification
{
	NSLog(@"[CoreMacOSPlatform] Application will become active");
}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification
{
	NSLog(@"[CoreMacOSPlatform] Application did become active");

    [self OnResume];
}

- (void)applicationDidResignActive:(NSNotification *)aNotification
{
	NSLog(@"[CoreMacOSPlatform] Application did resign active");

    [self OnSuspend];
}

- (void)applicationDidChangeScreenParameters:(NSNotification *)aNotification
{
	NSLog(@"[CoreMacOSPlatform] Application did change screen params");
}

- (void)applicationDidHide:(NSNotification *)aNotification
{
	NSLog(@"[CoreMacOSPlatform] Application did hide");
}

- (void)applicationDidUnhide:(NSNotification *)aNotification
{
	NSLog(@"[CoreMacOSPlatform] Application did unhide");
}

- (void)windowWillClose:(NSNotification *)notification
{
	[[NSApplication sharedApplication] terminate: nil];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    mainWindowController->willQuit = true;
    
	Core::Instance()->SystemAppFinished();
	FrameworkWillTerminate();
    Core::Instance()->ReleaseSingletons();

	NSLog(@"[CoreMacOSPlatform] Application terminate");
	return NSTerminateNow;
}

- (void)OnSuspend
{
    if(core)
    {
        core->OnSuspend();
    }
    else 
    {
        Core::Instance()->SetIsActive(false);
    }
}

- (void)OnResume
{
    if(core)
    {
        core->OnResume();
    }
    else 
    {
        Core::Instance()->SetIsActive(true);
    }
}

@end

namespace DAVA 
{

Core::eScreenMode CoreMacOSPlatform::GetScreenMode()
{
    return ([mainWindowController isFullScreen]) ? Core::eScreenMode::FULLSCREEN : Core::eScreenMode::WINDOWED;
}

bool CoreMacOSPlatform::SetScreenMode(eScreenMode screenMode)
{
    if (screenMode == Core::eScreenMode::FULLSCREEN)
    {
        return [mainWindowController setFullScreen:true] == YES;
    }
    else if (screenMode == Core::eScreenMode::WINDOWED)
    {
        return [mainWindowController setFullScreen:false] == YES;
    }
    else
    {
        Logger::Error("Unsupported screen mode");
        return false;
    }
}

void CoreMacOSPlatform::Quit()
{
	mainWindowController->willQuit = true;
	[[NSApplication sharedApplication] terminate: nil];
}

void CoreMacOSPlatform::SetScreenScaleMultiplier(float32 multiplier)
{
    Core::SetScreenScaleMultiplier(multiplier);
    
    //This magick needed to correctly 'reshape' GLView and resize back-buffer.
    //Directly call [openGLView reshape] doesn't help, as an other similar 'tricks'
    [mainWindowController->mainWindow setContentView:nil];
    [mainWindowController->mainWindow setContentView:mainWindowController->openGLView];
}
    
};
