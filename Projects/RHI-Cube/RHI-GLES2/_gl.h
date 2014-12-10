#pragma once

#include "GL/glew.h"
#include <GL/GL.h>
#include "GL/wglew.h"

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

