#include "_gl.h"

#if defined(__DAVAENGINE_MACOS__)

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <QuartzCore/CALayer.h>

void macos_gl_init(const rhi::InitParam& params)
{
    _GLES2_Native_Window = params.window;
    _GLES2_Context = [static_cast<NSOpenGLView*>(_GLES2_Native_Window) openGLContext];

    GLint swapInt = params.vsyncEnabled ? 1 : 0;
    [static_cast<NSOpenGLContext*>(_GLES2_Context) setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

void macos_gl_reset(const rhi::ResetParam& params)
{
    NSOpenGLView* view = static_cast<NSOpenGLView*>(_GLES2_Native_Window);
    NSOpenGLContext* context = [view openGLContext];

    _GLES2_Native_Window = params.window;
    _GLES2_DefaultFrameBuffer_Width = params.width;
    _GLES2_DefaultFrameBuffer_Height = params.height;
    _GLES2_Context = context;

    const GLint backingSize[2] = { GLint(_GLES2_DefaultFrameBuffer_Width), GLint(_GLES2_DefaultFrameBuffer_Height) };

    GLint swapInt = params.vsyncEnabled ? 1 : 0;
    [context setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    [context setValues:backingSize forParameter:NSOpenGLContextParameterSurfaceBackingSize];

    CGLEnable(context.CGLContextObj, kCGLCESurfaceBackingSize);
    [context update];

    CAOpenGLLayer* layer = static_cast<CAOpenGLLayer*>([view layer]);
    [layer setContentsScale:params.scaleX];
    [view update];
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
