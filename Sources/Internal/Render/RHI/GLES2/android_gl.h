#ifndef __DAVAENGINE_IOSGL_H__
#define __DAVAENGINE_IOSGL_H__

#include "Base/Platform.h"

#ifdef __DAVAENGINE_ANDROID__

void android_gl_init(void* nativeWindow);
void android_gl_reset(void* nativeWindow);
void android_gl_checkSurface();
bool android_gl_end_frame();
void android_gl_acquire_context();
void android_gl_release_context();
void android_gl_enable_debug();
void GL_APIENTRY android_gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userdata)

#endif    

#endif // __DAVAENGINE_IOSGL_H__
