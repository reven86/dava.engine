#include "Base/Platform.h"

#ifdef __DAVAENGINE_ANDROID__

#include "android_gl.h"
#include "_gl.h"

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/native_window.h>

static EGLDisplay _display = EGL_NO_DISPLAY;
static EGLSurface _surface = EGL_NO_SURFACE;
static EGLContext _context = EGL_NO_CONTEXT;
static EGLint _format = 0;
static EGLConfig _config = 0;
static ANativeWindow * _nativeWindow = nullptr;

void android_gl_init(void * _window)
{
	_nativeWindow = static_cast<ANativeWindow *>(_window);

	const EGLint configAttribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
	    EGL_DEPTH_SIZE, 24,
	    EGL_STENCIL_SIZE, 8,
		EGL_NONE
	};

	const EGLint contextAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLint numConfigs;

	_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(_display, nullptr, nullptr);
	eglChooseConfig(_display, configAttribs, &_config, 1, &numConfigs);
	eglGetConfigAttrib(_display, _config, EGL_NATIVE_VISUAL_ID, &_format);

	ANativeWindow_setBuffersGeometry(_nativeWindow, _GLES2_DefaultFrameBuffer_Width, _GLES2_DefaultFrameBuffer_Height, _format);
	_surface = eglCreateWindowSurface(_display, _config, _nativeWindow, nullptr);

	_context = eglCreateContext(_display, _config, EGL_NO_CONTEXT, contextAttribs);
	_GLES2_Context = _context;

	eglMakeCurrent(_display, _surface, _surface, _context);
}

void android_gl_reset()
{
	if(_surface != EGL_NO_SURFACE)
		eglDestroySurface(_display, _surface);

	ANativeWindow_setBuffersGeometry(_nativeWindow, _GLES2_DefaultFrameBuffer_Width, _GLES2_DefaultFrameBuffer_Height, _format);
	_surface = eglCreateWindowSurface(_display, _config, _nativeWindow, nullptr);

	eglMakeCurrent(_display, _surface, _surface, _context);
}

void android_gl_end_frame()
{
	eglSwapBuffers(_display, _surface);
}

void android_gl_acquire_context()
{
	eglMakeCurrent(_display, _surface, _surface, _context);
}

void android_gl_release_context()
{
	eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

#endif
