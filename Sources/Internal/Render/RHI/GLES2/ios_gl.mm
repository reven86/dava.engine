
#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)
    #include "ios_gl.h"

//    #include <OpenGLES/ES3/gl.h>
//    #include <OpenGLES/ES3/glext.h>
    #include "Platform/TemplateiOS/EAGLView.h"

    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>

    #include "Debug/Profiler.h"


static EAGLContext* _Context            = 0;
static GLuint       defaultFramebuffer  = -1;
static GLuint       colorRenderbuffer   = -1;
static GLuint       depthRenderbuffer   = -1;
static int          backingWidth        = 0;
static int          backingHeight       = 0;

BOOL
ios_GL_resize_from_layer( CAEAGLLayer* layer )
{
    // Allocate color buffer backing based on the current layer size
    glBindRenderbuffer( GL_RENDERBUFFER, colorRenderbuffer );
    [_Context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
    glGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth );
    glGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight );
    
    glGenRenderbuffers( 1, &depthRenderbuffer );
    glBindRenderbuffer( GL_RENDERBUFFER, depthRenderbuffer );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, backingWidth, backingHeight );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer );
    
    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
    {
        NSLog( @"FAILED to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER) );
        return NO;
    }
    
    return YES;
}

void
ios_GL_init(void * nativeLayer)
{
    _Context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:_Context];

    glGenFramebuffers( 1, &defaultFramebuffer );
    glGenRenderbuffers( 1, &colorRenderbuffer );
    glBindFramebuffer( GL_FRAMEBUFFER, defaultFramebuffer );
    glBindRenderbuffer( GL_RENDERBUFFER, colorRenderbuffer );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer );

    ios_GL_resize_from_layer( (CAEAGLLayer*)nativeLayer);

    [EAGLContext setCurrentContext:_Context];
}

void 
ios_GL_begin_frame()
{
SCOPED_NAMED_TIMING("ios_GL_begin_frame");
    glBindFramebuffer( GL_FRAMEBUFFER, defaultFramebuffer );
    glViewport( 0, 0, backingWidth, backingHeight );
}

void 
ios_GL_end_frame()
{
SCOPED_NAMED_TIMING("ios_GL_end_frame");
    const GLenum discards[]  = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };

    glBindFramebuffer( GL_FRAMEBUFFER, defaultFramebuffer );
    glDiscardFramebufferEXT( GL_FRAMEBUFFER, 2, discards );
    
    glBindRenderbuffer( GL_RENDERBUFFER, colorRenderbuffer );
    [_Context presentRenderbuffer:GL_RENDERBUFFER];
}


#endif


