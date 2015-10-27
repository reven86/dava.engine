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
#include "Core/Core.h"
#include "Input/InputSystem.h"

#if defined(__DAVAENGINE_IPHONE__)

#import "Platform/TemplateiOS/RenderViewController.h"

@implementation RenderViewController

@synthesize backgroundView;
@synthesize renderView;

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        // Custom initialization
    }
    return self;
}
*/
- (id)init
{
    if (self = [super init])
    {
        renderView = nil;
        backgroundView = nil;
    }
    return self;
}

- (GLRenderView*)createGLView
{
    if (!renderView)
    {
        renderView = [[GLRenderView alloc] initWithFrame:[[ ::UIScreen mainScreen] bounds]];
    }
    else
    {
        NSLog(@"*** FAILED to create GLRenderView, cause RenderView already created!");
    }
    return (GLRenderView*)renderView;
}

- (MetalRenderView*)createMetalView
{
    if (!renderView)
    {
        renderView = [[MetalRenderView alloc] initWithFrame:[[ ::UIScreen mainScreen] bounds]];
    }
    else
    {
        NSLog(@"*** FAILED to create GLRenderView, cause RenderView already created!");
    }
    return (MetalRenderView*)renderView;
}

// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView
{
    // Add the background view needed to place native iOS components on it.
    backgroundView = [[BackgroundView alloc] initWithFrame:[renderView frame]];
    [backgroundView setBackgroundColor:[UIColor clearColor]];
    [backgroundView setAutoresizingMask:UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight];

    BOOL isMultipleTouchEnabled = (DAVA::InputSystem::Instance()->GetMultitouchEnabled()) ? YES : NO;
    [backgroundView setMultipleTouchEnabled:isMultipleTouchEnabled]; // to pass touches to framework

    [renderView addSubview:backgroundView];
    [backgroundView release];

    self.view = renderView;
}

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    DAVA::Core::eScreenOrientation orientation = DAVA::Core::Instance()->GetScreenOrientation();
    switch (orientation)
    {
    case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
        return interfaceOrientation == UIInterfaceOrientationLandscapeLeft;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
        return interfaceOrientation == UIInterfaceOrientationLandscapeRight;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE:
        return (interfaceOrientation == UIInterfaceOrientationLandscapeLeft) || (interfaceOrientation == UIInterfaceOrientationLandscapeRight);
        break;
    case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT:
        return interfaceOrientation == UIInterfaceOrientationPortrait;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN:
        return interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE:
        return (interfaceOrientation == UIInterfaceOrientationPortrait) || (interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown);
        break;
    default:
        return FALSE;
        break;
    }
}

- (BOOL)shouldAutorotate
{
    /*return (DAVA::Core::Instance()->GetScreenOrientation()==DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE)||(DAVA::Core::Instance()->GetScreenOrientation()==DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE);*/
    return TRUE;
}
- (NSUInteger)supportedInterfaceOrientations
{
    DAVA::Core::eScreenOrientation orientation = DAVA::Core::Instance()->GetScreenOrientation();
    switch (orientation)
    {
    case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
        return UIInterfaceOrientationMaskLandscapeLeft;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
        return UIInterfaceOrientationMaskLandscapeRight;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE:
        return UIInterfaceOrientationMaskLandscape;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT:
        return UIInterfaceOrientationMaskPortrait;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN:
        return UIInterfaceOrientationMaskPortraitUpsideDown;
        break;
    case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE:
        return UIInterfaceOrientationMaskPortrait;
        break;
    default:
        return UIInterfaceOrientationMaskPortrait;
        break;
    }
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];

    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload
{
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (void)dealloc
{
    [renderView release];
    renderView = nil;
    [super dealloc];
}

- (void)viewWillAppear:(BOOL)animating
{
    NSLog(@"RenderViewController viewWillAppear (startAnimation)");
    //	[renderView setCurrentContext];
    [renderView startAnimation];
}

- (void)viewDidDisappear:(BOOL)animating
{
    NSLog(@"RenderViewController viewDidDisappear (stopAnimation)");
    [renderView stopAnimation];
}

@end
#endif //