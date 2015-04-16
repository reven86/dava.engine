#pragma once


#if defined(__DAVAENGINE_WIN32__)

    #include "GL/glew.h"
    #include <GL/GL.h>
    #include "GL/wglew.h"
    
    #define GetGLErrorString gluErrorString

#elif defined(__DAVAENGINE_MACOS__)

    #include <Carbon/Carbon.h>
    #include <AGL/agl.h>
    #include <OpenGL/glext.h>
    
    #define GetGLErrorString aglErrorString

#elif defined(__DAVAENGINE_IPHONE__)

    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
    #include "ios_gl.h"
    
    #define GetGLErrorString aglErrorString

#else

    #include <GL/GL.h>

#endif

#include "../rhi_Type.h"

#if 0
#define GL_CALL(expr) \
{ \
    expr ; \
    int err = glGetError(); \
    if( err != GL_NO_ERROR ) \
        Log::Error( "gl", "FAILED  %s (%i) : %s\n", #expr, err, gluErrorString(err) ); \
}
#else
#define GL_CALL(expr) expr;
#endif


extern GLuint   _GLES2_FrameBuffer;
extern GLint    _GLES2_Viewport[4];

bool            GetGLTextureFormat( rhi::TextureFormat rhiFormat, GLint* internalFormat, GLint* format, GLenum* type );
