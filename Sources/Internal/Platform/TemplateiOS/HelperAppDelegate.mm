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


#include "Base/BaseTypes.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#if defined(__DAVAENGINE_IPHONE__)

#include "Platform/DeviceInfo.h"

#import <UIKit/UIKit.h>
#import "HelperAppDelegate.h"

extern  void FrameworkWillTerminate();
extern  void FrameworkDidLaunched();

static RenderView * renderView = nil;

int DAVA::Core::Run(int argc, char * argv[], AppHandle handle)
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	DAVA::Core * core = new DAVA::Core();
    core->SetCommandLine(argc, argv);
	core->CreateSingletons();
	
    FrameworkDidLaunched();
    
	//detecting physical screen size and initing core system with this size
    const DeviceInfo::ScreenInfo & screenInfo = DeviceInfo::GetScreenInfo();
    int32 width = screenInfo.width;
    int32 height = screenInfo.height;
    
	eScreenOrientation orientation = Instance()->GetScreenOrientation();
	if ((orientation==SCREEN_ORIENTATION_LANDSCAPE_LEFT)||
		(orientation==SCREEN_ORIENTATION_LANDSCAPE_RIGHT)||
		(orientation==SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE))
	{
        width = screenInfo.height;
        height = screenInfo.width;
	}
		
    float32 scale = DAVA::Core::Instance()->GetScreenScaleFactor();
		
	VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(width, height);
    VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(width * scale, height * scale);
		
	int retVal = UIApplicationMain(argc, argv, nil, nil);
	
	[pool release];
	return retVal;
}

DAVA::Core::eDeviceFamily DAVA::Core::GetDeviceFamily()
{
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
    {
        return DAVA::Core::DEVICE_PAD;
    }

    return DAVA::Core::DEVICE_HANDSET;
}

@implementation HelperAppDelegate

@synthesize renderViewController;

#include "Core/Core.h"
#include "Core/ApplicationCore.h"
#include "UI/UIScreenManager.h"


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	UIWindow *wnd = application.keyWindow;
	wnd.frame = [::UIScreen mainScreen].bounds;
	
    renderViewController = [[RenderViewController alloc] init];
    
    DVASSERT(DAVA::Core::Instance()->GetOptions()->IsKeyExists("renderer"));
    rhi::Api rhiRenderer = (rhi::Api)DAVA::Core::Instance()->GetOptions()->GetInt32("renderer");
    if(rhiRenderer == rhi::RHI_GLES2)
    {
        renderView = [renderViewController createGLView];
    }
    else if(rhiRenderer == rhi::RHI_METAL)
    {
        renderView = [renderViewController createMetalView];
    }
    
    DAVA::Core::Instance()->rendererParams.window = [renderView layer];
    
    DAVA::Size2i screenSize = DAVA::VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize();
    DAVA::Core::Instance()->rendererParams.width = screenSize.dx;
    DAVA::Core::Instance()->rendererParams.height = screenSize.dy;
    
	DAVA::UIScreenManager::Instance()->RegisterController(CONTROLLER_GL, renderViewController);
	DAVA::UIScreenManager::Instance()->SetGLControllerId(CONTROLLER_GL);
    
    [application.keyWindow setRootViewController: renderViewController];
	
	DAVA::Core::Instance()->SystemAppStarted();
    
    return YES;
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    DAVA::ApplicationCore * core = DAVA::Core::Instance()->GetApplicationCore();
    if(core)
    {
        core->OnResume();
    }
    else 
    {
       DAVA::Core::Instance()->SetIsActive(true);
    }
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    DAVA::ApplicationCore * core = DAVA::Core::Instance()->GetApplicationCore();
    if(core)
    {
        core->OnSuspend();
    }
    else 
    {
        DAVA::Core::Instance()->SetIsActive(false);
    }
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    bool isLock = false;
    UIApplicationState state = [application applicationState];
    if (state == UIApplicationStateInactive)
    {
//        NSLog(@"Sent to background by locking screen");
        isLock = true;
    }
//    else if (state == UIApplicationStateBackground)
//    {
//        NSLog(@"Sent to background by home button/switching to other app");
//    }
	DAVA::Core::Instance()->GoBackground(isLock);

    rhi::SuspendRendering();
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    DAVA::ApplicationCore * core = DAVA::Core::Instance()->GetApplicationCore();
    if(core)
    {
		DAVA::Core::Instance()->GoForeground();
    }
    else 
    {
        DAVA::Core::Instance()->SetIsActive(true);
    }
    
    rhi::ResumeRendering();
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	NSLog(@"Application termination started");
	DAVA::Core::Instance()->SystemAppFinished();
	NSLog(@"System release started");
    
    if(DAVA::Logger::Instance())
    {
        DAVA::Logger::Instance()->SetLogFilename("");
    }
    
//	DAVA::Core::Instance()->ReleaseSingletons();
    

//	DAVA::Sprite::DumpSprites();
//	DAVA::Texture::DumpTextures();

	FrameworkWillTerminate();
	NSLog(@"Application termination finished");
}

@end
#endif
