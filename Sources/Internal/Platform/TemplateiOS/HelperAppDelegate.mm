#include "Base/BaseTypes.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#if defined(__DAVAENGINE_IPHONE__)

#include "Platform/DeviceInfo.h"
#include "Core/Core.h"

#import <UIKit/UIKit.h>
#import "HelperAppDelegate.h"

extern void FrameworkWillTerminate();
extern void FrameworkDidLaunched();

static RenderView* renderView = nil;

class CoreIOS : public DAVA::Core
{
public:
    void SetScreenScaleMultiplier(DAVA::float32 multiplier) override
    {
        if (fabsf(GetScreenScaleMultiplier() - multiplier) >= DAVA::EPSILON)
        {
            Core::SetScreenScaleMultiplier(multiplier);

            if (renderView)
            {
                ProcessResize();

                rhi::ResetParam params;
                params.width = rendererParams.width;
                params.height = rendererParams.height;
                params.window = [renderView layer];

                DAVA::Renderer::Reset(params);
            }
        }
    }

    void ProcessResize()
    {
        DAVA::float32 screenScale = GetScreenScaleFactor();

        [renderView setContentScaleFactor:screenScale];

        //detecting physical screen size and initing core system with this size
        const DAVA::DeviceInfo::ScreenInfo& screenInfo = DAVA::DeviceInfo::GetScreenInfo();
        DAVA::int32 width = screenInfo.width;
        DAVA::int32 height = screenInfo.height;

        eScreenOrientation orientation = GetScreenOrientation();
        if ((orientation == SCREEN_ORIENTATION_LANDSCAPE_LEFT) ||
            (orientation == SCREEN_ORIENTATION_LANDSCAPE_RIGHT) ||
            (orientation == SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE))
        {
            width = screenInfo.height;
            height = screenInfo.width;
        }

        DAVA::int32 physicalWidth = width * screenScale;
        DAVA::int32 physicalHeight = height * screenScale;

        DAVA::VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(width, height);
        DAVA::VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(physicalWidth, physicalHeight);

        rendererParams.window = [renderView layer];
        rendererParams.width = physicalWidth;
        rendererParams.height = physicalHeight;
    }
};

int DAVA::Core::Run(int argc, char* argv[], AppHandle handle)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    CoreIOS* core = new CoreIOS();
    core->SetCommandLine(argc, argv);
    core->CreateSingletons();

    FrameworkDidLaunched();

    core->ProcessResize();

    int retVal = UIApplicationMain(argc, argv, nil, @"iOSAppDelegate");

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

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    UIWindow* wnd = application.keyWindow;
    wnd.frame = [ ::UIScreen mainScreen].bounds;

    renderViewController = [[RenderViewController alloc] init];

    DVASSERT(DAVA::Core::Instance()->GetOptions()->IsKeyExists("renderer"));
    rhi::Api rhiRenderer = (rhi::Api)DAVA::Core::Instance()->GetOptions()->GetInt32("renderer");

    // CRAP: hardcoded metal-view, for testing
    renderView = ((rhi::ApiIsSupported(rhi::RHI_METAL)))
    ?
    [renderViewController createMetalView]
    :
    [renderViewController createGLView];

    /*
    if (rhiRenderer == rhi::RHI_GLES2)
    {
        renderView = [renderViewController createGLView];
    }
    else if (rhiRenderer == rhi::RHI_METAL)
    {
        renderView = [renderViewController createMetalView];
    }
*/
    ((CoreIOS*)DAVA::Core::Instance())->ProcessResize();

    DAVA::UIScreenManager::Instance()->RegisterController(CONTROLLER_GL, renderViewController);
    DAVA::UIScreenManager::Instance()->SetGLControllerId(CONTROLLER_GL);

    [application.keyWindow setRootViewController:renderViewController];

    DAVA::Core::Instance()->SystemAppStarted();

    return YES;
}

- (void)applicationDidBecomeActive:(UIApplication*)application
{
    DAVA::ApplicationCore* core = DAVA::Core::Instance()->GetApplicationCore();
    if (core)
    {
        core->OnResume();
    }
    else
    {
        DAVA::Core::Instance()->SetIsActive(true);
    }
}

- (void)applicationWillResignActive:(UIApplication*)application
{
    DAVA::ApplicationCore* core = DAVA::Core::Instance()->GetApplicationCore();
    if (core)
    {
        core->OnSuspend();
    }
    else
    {
        DAVA::Core::Instance()->SetIsActive(false);
    }
}

- (void)applicationDidEnterBackground:(UIApplication*)application
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

- (void)applicationWillEnterForeground:(UIApplication*)application
{
    DAVA::ApplicationCore* core = DAVA::Core::Instance()->GetApplicationCore();
    if (core)
    {
        DAVA::Core::Instance()->GoForeground();
    }
    else
    {
        DAVA::Core::Instance()->SetIsActive(true);
    }

    rhi::ResumeRendering();
}

- (void)applicationWillTerminate:(UIApplication*)application
{
    NSLog(@"Application termination started");
    DAVA::Core::Instance()->SystemAppFinished();
    NSLog(@"System release started");

    if (DAVA::Logger::Instance())
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
