#include "_gl.h"

#if defined(__DAVAENGINE_MACOS__)

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

void macos_gl_init(const rhi::InitParam& params)
{
    _GLES2_Native_Window = params.window;
    _GLES2_Context = [static_cast<NSOpenGLView*>(_GLES2_Native_Window) openGLContext];

    GLint swapInt = params.vsyncEnabled ? 1 : 0;
    [static_cast<NSOpenGLContext*>(_GLES2_Context) setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

void macos_gl_reset(const rhi::ResetParam& params)
{
    _GLES2_Native_Window = params.window;
    _GLES2_Context = [static_cast<NSOpenGLView*>(_GLES2_Native_Window) openGLContext];

    GLint swapInt = params.vsyncEnabled ? 1 : 0;
    [static_cast<NSOpenGLContext*>(_GLES2_Context) setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

void macos_gl_end_frame()
{
    if (_GLES2_Native_Window)
    {
        [static_cast<NSOpenGLContext*>(_GLES2_Context) flushBuffer];
    }
}

void macos_gl_acquire_context()
{
    if (_GLES2_Native_Window)
    {
        [static_cast<NSOpenGLContext*>(_GLES2_Context) makeCurrentContext];
    }
}

void macos_gl_release_context()
{
    if (_GLES2_Native_Window)
    {
        //        [(NSOpenGLContext *)_GLES2_Context clearCurrentContext];
    }
}

#endif
