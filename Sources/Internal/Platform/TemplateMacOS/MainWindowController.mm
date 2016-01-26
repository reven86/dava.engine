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


#import "Platform/TemplateMacOS/MainWindowController.h"
#include "Platform/TemplateMacOS/CorePlatformMacOS.h"
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
		CoreMacOSPlatform * core = new CoreMacOSPlatform();
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
        CoreMacOSPlatform* core = new CoreMacOSPlatform();
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

- (void)setMinimumWindowSize:(DAVA::float32)width height:(DAVA::float32)height;
@end

@implementation MainWindowController

static MainWindowController * mainWindowController = nil;

/* This code disabled for now and left for the future
 */
namespace DAVA 
{
	Vector2 CoreMacOSPlatform::GetMousePosition()
	{
		NSPoint p = [mainWindowController->mainWindow mouseLocationOutsideOfEventStream]; //[NSEvent locationInWindow]; 
		p = [mainWindowController->openGLView convertPointFromBacking: p];

        Vector2 mouseLocation;
		mouseLocation.x = p.x;
		mouseLocation.y = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy - p.y;
		return mouseLocation;
	}
    
    void CoreMacOSPlatform::SetWindowMinimumSize(float32 width, float32 height)
    {
        DVASSERT((width == 0.0f && height == 0.0f) || (width > 0.0f && height > 0.0f));
        minWindowWidth = width;
        minWindowHeight = height;

        [mainWindowController setMinimumWindowSize: minWindowWidth height: minWindowHeight];
    }
    
    Vector2 CoreMacOSPlatform::GetWindowMinimumSize() const
    {
        return Vector2(minWindowWidth, minWindowHeight);
    }
}

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

    float32 minWidth = 0.0f;
    float32 minHeight = 0.0f;
    KeyedArchive* options = Core::Instance()->GetOptions();
    if(nullptr != options)
    {
        title = options->GetString("title", "[set application title using core options property 'title']");
        if(options->IsKeyExists("width") && options->IsKeyExists("height"))
        {
            width = options->GetInt32("width");
            height = options->GetInt32("height");
        }
        
        isFull = (0 != options->GetInt32("fullscreen", 0));
        minWidth = static_cast<float32>(options->GetInt32("min-width", 0));
        minHeight = static_cast<float32>(options->GetInt32("min-height", 0));
    }
    
    openGLView = [[OpenGLView alloc]initWithFrame: NSMakeRect(0, 0, width, height)];
    
    NSUInteger wStyle = NSTitledWindowMask + NSMiniaturizableWindowMask + NSClosableWindowMask + NSResizableWindowMask;
    NSRect wRect = NSMakeRect((displayRect.size.width - width) / 2, (displayRect.size.height - height) / 2, width, height);
    mainWindow = [[NSWindow alloc] initWithContentRect:wRect styleMask:wStyle backing:NSBackingStoreBuffered defer:FALSE];
    [mainWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [mainWindow setDelegate:self];
    [mainWindow setContentView: openGLView];
    [mainWindow setContentSize: NSMakeSize(width, height)];
    mainWindow.contentMinSize = NSMakeSize(width, height);
    [mainWindowController setMinimumWindowSize: 0.0f height: 0.0f];
    
    if (minWidth > 0 && minHeight > 0)
    {
        // Call Core::SetWindowMinimumSize to save minimum width and height and limit window size
        // Such a strange way due to my little knowledge of Objective-C
        Core::Instance()->SetWindowMinimumSize(minWidth, minHeight);
    }

    core = Core::GetApplicationCore();
    Core::Instance()->SetNativeView(openGLView);

#if RHI_COMPLETE
    RenderManager::Instance()->DetectRenderingCapabilities();
#endif

// start animation
    currFPS = 60;
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

-(void)setMinimumWindowSize:(DAVA::float32)width height:(DAVA::float32)height
{
    const float32 MIN_WIDTH = 64.0f;
    const float32 MIN_HEIGHT = 64.0f;
 
    // Always limit minimum window size to 64x64, as application crashes
    // when resizing window to zero height (stack overflow occures)
    // It seems that NSOpenGLView is responsible for that
    if (width < MIN_WIDTH) width = MIN_WIDTH;
    if (height < MIN_HEIGHT) height = MIN_HEIGHT;
    mainWindow.contentMinSize = NSMakeSize(width, height);
}

- (void)windowWillMiniaturize:(NSNotification *)notification
{
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
    xcore->signalAppMinimizedRestored.Emit(true);
    
    [self OnSuspend];
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
    xcore->signalAppMinimizedRestored.Emit(false);
    
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
}

- (void)mouseExited:(NSEvent *)theEvent
{
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
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application did finish launching");
    
    [self OnResume];
    
#if RHI_COMPLETE
    DAVA::Cursor * activeCursor = RenderManager::Instance()->GetCursor();
    if (activeCursor)
    {
        NSCursor * cursor = (NSCursor*)activeCursor->GetMacOSXCursor();
        [cursor set];
    }
#endif
}

static CGEventRef EventTapCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon)
{
    static bool restorePinning = false;
    static int64_t myPid = static_cast<int64_t>(getpid());
    
    int64_t targetPid = CGEventGetIntegerValueField(event, kCGEventTargetUnixProcessID);
    if (targetPid != myPid)
    {
        // Turn off mouse cature if event target is not our application and
        // current capture mode is pinning
        InputSystem::eMouseCaptureMode captureMode = InputSystem::Instance()->GetMouseCaptureMode();
        if (InputSystem::eMouseCaptureMode::PINING == captureMode)
        {
            InputSystem::Instance()->SetMouseCaptureMode(InputSystem::eMouseCaptureMode::OFF);
            restorePinning = true;
        }
    }
    else
    {
        if (restorePinning)
        {
            InputSystem::Instance()->SetMouseCaptureMode(InputSystem::eMouseCaptureMode::PINING);
            restorePinning = false;
        }
    }
    return event;
}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
	[self createWindows];
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application will finish launching: %s", [[[NSBundle mainBundle] bundlePath] UTF8String]);
    
    {
        // OS X application has no way to detect when she is no longer in control,
        // specifically when Mission Control is active or when user invoked Show Desktop (F11 key).
        // And application has no chance to release mouse capture.
        
        // Install mouse hook which determines whether Mission Control, Launchpad is active
        // and temporary turns off mouse pinning
        // https://developer.apple.com/library/mac/documentation/Carbon/Reference/QuartzEventServicesRef/index.html
        CFMachPortRef portRef = CGEventTapCreate(kCGAnnotatedSessionEventTap,
                                                 kCGTailAppendEventTap,
                                                 kCGEventTapOptionListenOnly,
                                                 NSAnyEventMask,
                                                 &EventTapCallback,
                                                 nullptr);
        
        if (portRef != nullptr)
        {
            CFRunLoopSourceRef loopSourceRef = CFMachPortCreateRunLoopSource(nullptr, portRef, 0);
            CFRunLoopAddSource(CFRunLoopGetCurrent(), loopSourceRef, kCFRunLoopDefaultMode);
            CFRelease(portRef);
            CFRelease(loopSourceRef);
        }
        else
        {
            Logger::Error("[CoreMacOSPlatform] failed to install mouse hook");
        }
    }

    float32 userScale = Core::Instance()->GetScreenScaleMultiplier();

    NSSize windowsSize =[openGLView frame].size;
    NSSize surfaceSize = [openGLView convertSizeToBacking:windowsSize];
    
    DAVA::CoreMacOSPlatform* macCore = (DAVA::CoreMacOSPlatform*)Core::Instance();
    macCore->rendererParams.window = mainWindowController->openGLView;
    macCore->rendererParams.width = surfaceSize.width;
    macCore->rendererParams.height = surfaceSize.height;
    macCore->rendererParams.scaleX = userScale;
    macCore->rendererParams.scaleY = userScale;

    VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(windowsSize.width, windowsSize.height);
    VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(surfaceSize.width * userScale, surfaceSize.height * userScale);

    Core::Instance()->SystemAppStarted();
}

- (void)applicationWillBecomeActive:(NSNotification *)aNotification
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application will become active");
}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application did become active");

    [self OnResume];
}

- (void)applicationDidResignActive:(NSNotification *)aNotification
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application did resign active");

    [self OnSuspend];
}

- (void)applicationDidChangeScreenParameters:(NSNotification *)aNotification
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application did change screen params");
}

- (void)applicationDidHide:(NSNotification *)aNotification
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application did hide");
    
    CoreMacOSPlatform* xcore = static_cast<CoreMacOSPlatform*>(Core::Instance());
    xcore->signalAppMinimizedRestored.Emit(true);
}

- (void)applicationDidUnhide:(NSNotification *)aNotification
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application did unhide");
    
    CoreMacOSPlatform* xcore = static_cast<CoreMacOSPlatform*>(Core::Instance());
    xcore->signalAppMinimizedRestored.Emit(false);
}

- (void)windowWillClose:(NSNotification *)notification
{
	[[NSApplication sharedApplication] terminate: nil];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    Logger::FrameworkDebug("[CoreMacOSPlatform] Application should terminate");
    
    mainWindowController->openGLView.willQuit = true;
    
	Core::Instance()->SystemAppFinished();
	FrameworkWillTerminate();
    Core::Instance()->ReleaseSingletons();

	NSLog(@"[CoreMacOSPlatform] Application has terminated");
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
	mainWindowController->openGLView.willQuit = true;
	[[NSApplication sharedApplication] terminate: nil];
}
	
};
