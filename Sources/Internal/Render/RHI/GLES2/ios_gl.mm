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

#include "_gl.h"

#if defined(__DAVAENGINE_IPHONE__)

#import <QuartzCore/QuartzCore.h>

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "Debug/Profiler.h"

static GLuint       colorRenderbuffer   = -1;
static GLuint       depthRenderbuffer   = -1;
static int          backingWidth        = 0;
static int          backingHeight       = 0;

bool
ios_gl_resize_from_layer( void * nativeLayer )
{
    // Allocate color buffer backing based on the current layer size
    glBindRenderbuffer( GL_RENDERBUFFER, colorRenderbuffer );
    [(EAGLContext*)_GLES2_Context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)nativeLayer];
    glGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth );
    glGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight );
    
    _GLES2_DefaultFrameBuffer_Width  = backingWidth;
    _GLES2_DefaultFrameBuffer_Height = backingHeight;
    
    glGenRenderbuffers( 1, &depthRenderbuffer );
    glBindRenderbuffer( GL_RENDERBUFFER, depthRenderbuffer );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, backingWidth, backingHeight );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer );
    
    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
    {
        NSLog( @"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER) );
        return NO;
    }
    
    return YES;
}

void
ios_gl_init(void * nativeLayer)
{
    _GLES2_Native_Window = nativeLayer;
    
    _GLES2_Context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (_GLES2_Context == nil)
    {
        _GLES2_Context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    }
    
    [EAGLContext setCurrentContext:(EAGLContext*)_GLES2_Context];
    
    glGenFramebuffers( 1, &_GLES2_Default_FrameBuffer );
    glGenRenderbuffers( 1, &colorRenderbuffer );
    glBindFramebuffer( GL_FRAMEBUFFER, _GLES2_Default_FrameBuffer );
    glBindRenderbuffer( GL_RENDERBUFFER, colorRenderbuffer );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer );
    
    _GLES2_Binded_FrameBuffer = _GLES2_Default_FrameBuffer;
    
    ios_gl_resize_from_layer( _GLES2_Native_Window );
}

void
ios_gl_begin_frame()
{
    SCOPED_NAMED_TIMING("ios_GL_begin_frame");
    glViewport( 0, 0, backingWidth, backingHeight );
}

void
ios_gl_end_frame()
{
    SCOPED_NAMED_TIMING("ios_GL_end_frame");
    const GLenum discards[]  = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
    
    glBindFramebuffer( GL_FRAMEBUFFER, _GLES2_Default_FrameBuffer );
    glDiscardFramebufferEXT( GL_FRAMEBUFFER, 2, discards );
    
    glBindRenderbuffer( GL_RENDERBUFFER, colorRenderbuffer );
    [(EAGLContext*)_GLES2_Context presentRenderbuffer:GL_RENDERBUFFER];
}

void 
ios_gl_acquire_context()
{
    [EAGLContext setCurrentContext:(EAGLContext*)_GLES2_Context];
}

void
ios_gl_release_context()
{
//    [EAGLContext setCurrentContext:(EAGLContext*)_GLES2_Context];
}


#endif
