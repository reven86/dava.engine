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


#import "NPAPIOpenGLLayerMacOS.h"
#import "NPAPIPluginMacOS.h"

//#include "Render/RenderManager.h"
#include "Core/Core.h"

@implementation NPAPIOpenGLLayerMacOS

static CGLContextObj inFrameOpenGLContext = NULL;
static CGLPixelFormatObj inFramePixelFormat = NULL;

- (id) initWithPluginInstance:(NPAPIPluginMacOS*) instance
{
    if ([super init])
    {
        pluginInstance = instance;
        isFirstDraw = YES;

        inFrameOpenGLContext = NULL;
    }

    return self;
}

- (CGLPixelFormatObj)copyCGLPixelFormatForDisplayMask:(uint32_t)mask
{
    uint32_t attributes[] =
    {
        kCGLPFADisplayMask, mask,
        kCGLPFANoRecovery,
        kCGLPFAColorSize, 24,
        kCGLPFADepthSize, 16,
        kCGLPFAStencilSize, 8,
        kCGLPFADoubleBuffer,
        kCGLPFAAccelerated,
        0
    };

    CGLPixelFormatObj pixelFormatObj = NULL;
    GLint numPixelFormats = 0;
    CGLChoosePixelFormat((CGLPixelFormatAttribute*)attributes, &pixelFormatObj, &numPixelFormats);
    if(pixelFormatObj == NULL)
    {
        NSLog(@"Error: Could not choose pixel format!");
    }

    return pixelFormatObj;
}

- (void)releaseCGLPixelFormat:(CGLPixelFormatObj)pixelFormat
{
    CGLDestroyPixelFormat(pixelFormat);
}

- (CGLContextObj)copyCGLContextForPixelFormat:(CGLPixelFormatObj)pixelFormat
{
    CGLContextObj contextObj = NULL;
    CGLCreateContext(pixelFormat, NULL, &contextObj);
    if(contextObj == NULL)
    {
        NSLog(@"Error: Could not create context!");
    }

    // enable vsync
    GLint swapInt = 1;
    CGLSetParameter(contextObj, kCGLCPSwapInterval, &swapInt);
    CGLSetCurrentContext(contextObj);

    return contextObj;
}

- (void)releaseCGLContext:(CGLContextObj)glContext
{
    CGLDestroyContext(glContext);
}

- (void)drawInCGLContext:(CGLContextObj)ctx pixelFormat:(CGLPixelFormatObj)pf forLayerTime:(CFTimeInterval)t displayTime:(const CVTimeStamp *)ts
{
    if (isFirstDraw == YES && pluginInstance)
    {
        [pluginInstance doInitializationOnFirstDraw];
//      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        isFirstDraw = NO;

        [super drawInCGLContext:ctx pixelFormat:pf forLayerTime:t displayTime:ts];
        return;
    }
/*
    if (activeCursor != RenderManager::Instance()->GetCursor())
    {
        activeCursor = RenderManager::Instance()->GetCursor();
        [[self window] invalidateCursorRectsForView: self];
    }
*/

#if RHI_COMPLETE
    DAVA::RenderManager::Instance()->Lock();
#endif
    
    inFrameOpenGLContext = ctx;
    inFramePixelFormat = pf;

    DAVA::Core::Instance()->SystemProcessFrame();

    // These parameters are valid inside SystemProcessFrame() only.
    inFrameOpenGLContext = NULL;
    inFramePixelFormat = NULL;

    [super drawInCGLContext:ctx pixelFormat:pf forLayerTime:t displayTime:ts];
#if RHI_COMPLETE
    DAVA::RenderManager::Instance()->Unlock();
#endif
}

+ (void) getThreadChildContext:(CGLContextObj*) childContext
{
    // This method should be called only from inside SystemProcessFrame.
    DVASSERT(inFrameOpenGLContext);
    DVASSERT(inFramePixelFormat);

    CGLError err = CGLCreateContext(inFramePixelFormat, inFrameOpenGLContext, childContext);
    NSLog(@"getThreadChildContext::CGLCreateContext executed with err code %i", err);
}

@end
