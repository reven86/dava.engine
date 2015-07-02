#include "_gl.h"

#if defined(__DAVAENGINE_MACOS__)

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

void
macos_gl_init(void * glView)
{
    _GLES2_Native_Window = glView;
    _GLES2_Context = [(NSOpenGLView *)_GLES2_Native_Window openGLContext];
    
    _GLES2_DefaultFrameBuffer_Width  = ((NSOpenGLView*)_GLES2_Native_Window).frame.size.width;
    _GLES2_DefaultFrameBuffer_Height = ((NSOpenGLView*)_GLES2_Native_Window).frame.size.height;
}

void
macos_gl_end_frame()
{
    if( _GLES2_Native_Window )
    {
        [(NSOpenGLContext *)_GLES2_Context flushBuffer];
    }
}

void
macos_gl_acquire_context()
{
    if( _GLES2_Native_Window )
    {
        [(NSOpenGLContext *)_GLES2_Context makeCurrentContext];
    }
}

void
macos_gl_release_context()
{
    if( _GLES2_Native_Window )
    {
//        [(NSOpenGLContext *)_GLES2_Context clearCurrentContext];
    }
}

#endif

