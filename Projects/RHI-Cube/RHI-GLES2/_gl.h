#pragma once


#include "GL/glew.h"
#include "GL/GL.h"
#include "GL/wglew.h"

#if 1
#define GL_CALL(expr) \
{ \
    expr ; \
    int err = glGetError(); \
    if( err != GL_NO_ERROR ) \
        Logger::Error( "[gl] FAILED  %s (%i) : %s", #expr, err, gluErrorString(err) ); \
}
#else
#define GL_CALL(expr) expr;
#endif

