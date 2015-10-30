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
#include "UI/UIControlSystem.h"
#include "UI/UIEvent.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#if defined(__DAVAENGINE_IPHONE__)

#if !defined(TARGET_IPHONE_SIMULATOR)
#include <QuartzCore/CAMetalLayer.h>
#endif

#import "Platform/TemplateiOS/RenderView.h"

#include "DAVAEngine.h"
#include "Utils/Utils.h"

static DAVA::uint32 KEYBOARD_FPS_LIMIT = 20;

@implementation RenderView

@synthesize animating;
@dynamic animationFrameInterval;

//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
//- (id) initWithCoder: (NSCoder *)aDecoder
- (id)initWithFrame:(CGRect)aRect
{
    if ((self = [super initWithFrame:aRect]))
    {
        float scf = DAVA::Core::Instance()->GetScreenScaleFactor();
        [self setContentScaleFactor:scf];

        // Subscribe to "keyboard change frame" notifications to block GL while keyboard change is performed (see please DF-2012 for details).
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardDidFrameChanged:) name:UIKeyboardDidChangeFrameNotification object:nil];

        self.layer.opaque = TRUE;

        DAVA::KeyedArchive* options = DAVA::Core::Instance()->GetOptions();

        switch ((DAVA::Core::eScreenOrientation)options->GetInt32("orientation", DAVA::Core::SCREEN_ORIENTATION_PORTRAIT))
        {
        case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT:
        {
            [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationPortrait animated:false];
        }
        break;
        case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
        {
            [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationLandscapeLeft animated:false];
        }
        break;
        case DAVA::Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN:
        {
            [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationPortraitUpsideDown animated:false];
        }
        break;
        case DAVA::Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
        {
            [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationLandscapeRight animated:false];
        }
        break;

        default:
            break;
        }

        //        DAVA::RenderManager::Instance()->SetRenderContextId(DAVA::EglGetCurrentContext());
        DAVA::Size2i physicalScreen = DAVA::VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize();
        //        DAVA::RenderManager::Instance()->Init(physicalScreen.dx, physicalScreen.dy);
        //        DAVA::RenderManager::Instance()->DetectRenderingCapabilities();
        //        DAVA::RenderSystem2D::Instance()->Init();

        self.multipleTouchEnabled = (DAVA::InputSystem::Instance()->GetMultitouchEnabled()) ? YES : NO;
        animating = FALSE;
        displayLinkSupported = FALSE;
        animationFrameInterval = 1;
        currFPS = 60;
        displayLink = nil;
        animationTimer = nil;
        blockDrawView = false;
        limitKeyboardFps = false;

        // A system version of 3.1 or greater is required to use CADisplayLink. The NSTimer
        // class is used as fallback when it isn't available.
        NSString* reqSysVer = @"3.1";
        NSString* currSysVer = [[UIDevice currentDevice] systemVersion];
        if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
            displayLinkSupported = TRUE;

        DAVA::Logger::Debug("RenderView Created successfully. displayLink: %d", (int)displayLinkSupported);
    }

    return self;
}

- (void)drawView:(id)sender
{
    if (blockDrawView)
    {
        // Yuri Coder, 2013/02/06. In case we are displaying ASSERT dialog we need to block rendering because RenderManager might be already locked here.
        return;
    }

    DAVA::Core::Instance()->SystemProcessFrame();

    DAVA::int32 targetFPS = 0;
    if (limitKeyboardFps)
    {
        targetFPS = KEYBOARD_FPS_LIMIT;
    }
    else
    {
        targetFPS = DAVA::Renderer::GetDesiredFPS();
    }

    if (currFPS != targetFPS)
    {
        currFPS = targetFPS;
        float interval = 60.0f / currFPS;
        if (interval < 1.0f)
        {
            interval = 1.0f;
        }
        [self setAnimationFrameInterval:(int)interval];
    }
}

- (NSInteger)animationFrameInterval
{
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    // Frame interval defines how many display frames must pass between each time the
    // display link fires. The display link will only fire 30 times a second when the
    // frame internal is two on a display that refreshes 60 times a second. The default
    // frame interval setting of one will fire 60 times a second when the display refreshes
    // at 60 times a second. A frame interval setting of less than one results in undefined
    // behavior.
    if (frameInterval >= 1)
    {
        animationFrameInterval = frameInterval;

        if (animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation
{
    if (!animating)
    {
        if (displayLinkSupported)
        {
            // CADisplayLink is API new to iPhone SDK 3.1. Compiling against earlier versions will result in a warning, but can be dismissed
            // if the system version runtime check for CADisplayLink exists in -initWithCoder:. The runtime check ensures this code will
            // not be called in system versions earlier than 3.1.

            displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
            [displayLink setFrameInterval:animationFrameInterval];
            [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        }
        else
            animationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)((1.0 / 60.0) * animationFrameInterval) target:self selector:@selector(drawView:) userInfo:nil repeats:TRUE];

        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        if (displayLinkSupported)
        {
            [displayLink invalidate];
            displayLink = nil;
        }
        else
        {
            [animationTimer invalidate];
            animationTimer = nil;
        }

        animating = FALSE;
    }
}

void MoveTouchsToVector(void* inTouches, DAVA::Vector<DAVA::UIEvent>* outTouches)
{
    NSArray* ar = (NSArray*)inTouches;
    for (UITouch* curTouch in ar)
    {
        DAVA::UIEvent newTouch;
        newTouch.tid = (DAVA::int32)(DAVA::pointer_size)curTouch;

        CGPoint p = [curTouch locationInView:curTouch.view];
        newTouch.physPoint.x = p.x;
        newTouch.physPoint.y = p.y;
        newTouch.timestamp = curTouch.timestamp;
        newTouch.tapCount = static_cast<DAVA::int32>(curTouch.tapCount);
        newTouch.device = DAVA::UIEvent::Device::TOUCH_SURFACE;

        switch (curTouch.phase)
        {
        case UITouchPhaseBegan:
            newTouch.phase = DAVA::UIEvent::Phase::BEGAN;
            break;
        case UITouchPhaseEnded:
            newTouch.phase = DAVA::UIEvent::Phase::ENDED;
            break;
        case UITouchPhaseMoved:
        case UITouchPhaseStationary:
            newTouch.phase = DAVA::UIEvent::Phase::DRAG;
            break;
        case UITouchPhaseCancelled:
            newTouch.phase = DAVA::UIEvent::Phase::CANCELLED;
            break;
        }
        outTouches->push_back(newTouch);
    }
}

- (void)process:(int)touchType touch:(NSArray*)active withEvent:(NSArray*)total
{
    MoveTouchsToVector(active, &activeTouches);
    for (auto& t : activeTouches)
    {
        DAVA::UIControlSystem::Instance()->OnInput(&t);
    }
    activeTouches.clear();
    totalTouches.clear();
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
    [self process:0 touch:[touches allObjects] withEvent:[[event allTouches] allObjects]];
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
    [self process:0 touch:[touches allObjects] withEvent:[[event allTouches] allObjects]];
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
    [self process:0 touch:[touches allObjects] withEvent:[[event allTouches] allObjects]];
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
    [self process:0 touch:[touches allObjects] withEvent:[[event allTouches] allObjects]];
}

- (void)blockDrawing
{
    blockDrawView = true;
}

- (void)unblockDrawing
{
    blockDrawView = false;
}

- (void)keyboardDidFrameChanged:(NSNotification*)notification
{
    CGRect keyboardEndFrame = [[notification.userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    if (CGRectIntersectsRect(keyboardEndFrame, screenRect))
    {
        // Keyboard did show or move
        limitKeyboardFps = true;
    }
    else
    {
        // Keyboard did hide
        limitKeyboardFps = false;
    }
}
@end

///////////////////////////////////////////////////////////////////////
//////Metal View

@implementation MetalRenderView
+ (Class)layerClass
{
#if !defined(TARGET_IPHONE_SIMULATOR)
    return [CAMetalLayer class];
#else
    return [CALayer class];
#endif
}
@end

///////////////////////////////////////////////////////////////////////
//////OpenGL View

@implementation GLRenderView
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}
@end

#endif // #if defined(__DAVAENGINE_IPHONE__)