#ifndef __DAVAENGINE_RENDERVIEW_H__
#define __DAVAENGINE_RENDERVIEW_H__


#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#import "UI/UIEvent.h"

//using namespace DAVA;
// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface RenderView : UIView
{
@private
    BOOL animating;
    BOOL displayLinkSupported;
    NSInteger animationFrameInterval;
    // Use of the CADisplayLink class is the preferred method for controlling your animation timing.
    // CADisplayLink will link to the main display and fire every vsync when added to a given run-loop.
    // The NSTimer class is used only as fallback when running on a pre 3.1 device where CADisplayLink
    // isn't available.
    id displayLink;
    NSTimer* animationTimer;
    bool isLoading;
    bool isLoaded;

    DAVA::int32 currFPS;

    // Yuri Coder, 2013/02/06. This flag can be used to block drawView() call
    // in case if ASSERTion happened. This is introduced to do not stuck on the RenderManager::Lock()
    // mutex (since assertion might be called in the middle of drawing, DAVA::RenderManager::Instance()->Lock()
    // mutex might be already locked so we'll got a deadlock.
    // Return to this code after RenderManager mutex will be removed.
    bool blockDrawView;

    // Used to limit fps while device keyboard is changed
    bool limitKeyboardFps;
}

@property(readonly, nonatomic, getter=isAnimating) BOOL animating;
@property(nonatomic) NSInteger animationFrameInterval;

- (void)startAnimation;
- (void)stopAnimation;
- (void)drawView:(id)sender;

// Yuri Coder, 2013/02/06. This method is introduced to block rendering
// when assertion happened.
- (void)blockDrawing;
- (void)unblockDrawing;

@end

///////////////////////////////////////////////////////////////////////

@interface MetalRenderView : RenderView
+ (Class)layerClass;
@end

@interface GLRenderView : RenderView
+ (Class)layerClass;
@end

#endif //__DAVAENGINE_IPHONE__

#endif //__DAVAENGINE_RENDERVIEW_H__
