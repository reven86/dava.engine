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

    DAVA::Vector<DAVA::UIEvent> totalTouches;
    DAVA::Vector<DAVA::UIEvent> activeTouches;

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
