#include "Base/Platform.h"
#include "Logger/Logger.h"

#ifdef __DAVAENGINE_ANDROID__

#include "android_gl.h"
#include "_gl.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <android/native_window.h>

#if !defined(EGL_OPENGL_ES3_BIT)
    #define EGL_OPENGL_ES3_BIT 0x00000040
#endif

static EGLDisplay _display = EGL_NO_DISPLAY;
static EGLSurface _surface = EGL_NO_SURFACE;
static EGLContext _context = EGL_NO_CONTEXT;
static EGLint _format = 0;
static EGLConfig _config = 0;
static ANativeWindow* _nativeWindow = nullptr;

PFNGLEGL_GLDRAWELEMENTSINSTANCED glDrawElementsInstanced = nullptr;
PFNGLEGL_GLDRAWARRAYSINSTANCED glDrawArraysInstanced = nullptr;
PFNGLEGL_GLVERTEXATTRIBDIVISOR glVertexAttribDivisor = nullptr;
PFNGLEGL_GLBLITFRAMEBUFFERANGLEPROC glBlitFramebuffer = nullptr;
PFNGLEGL_GLRENDERBUFFERSTORAGEMULTISAMPLE glRenderbufferStorageMultisample = nullptr;
PFNGL_DEBUGMESSAGECONTROLKHRPROC glDebugMessageControl;
PFNGL_DEBUGMESSAGECALLBACKKHRPROC glDebugMessageCallback;

static bool needRecreateSurface = false;

const EGLint contextAttribs[] =
{
  EGL_CONTEXT_CLIENT_VERSION, 2,
  EGL_NONE
};

bool TryChoseConfig(EGLint* attribs)
{
    EGLint numConfigs = 0;
    if (eglChooseConfig(_display, attribs, nullptr, 0, &numConfigs) == EGL_FALSE)
        return false;

    DAVA::Logger::Info("Num configs: %d", numConfigs);

    if (numConfigs == 0)
        return false;

    DAVA::Vector<EGLConfig> configs(numConfigs);
    if (eglChooseConfig(_display, attribs, configs.data(), numConfigs, &numConfigs) == EGL_FALSE)
        return false;

    for (EGLConfig config : configs)
    {
        EGLint rgbs[4] = {};
        if (eglGetConfigAttrib(_display, config, EGL_RED_SIZE, rgbs) == EGL_FALSE)
            return false;
        if (eglGetConfigAttrib(_display, config, EGL_GREEN_SIZE, rgbs + 1) == EGL_FALSE)
            return false;
        if (eglGetConfigAttrib(_display, config, EGL_BLUE_SIZE, rgbs + 2) == EGL_FALSE)
            return false;
        if (eglGetConfigAttrib(_display, config, EGL_BUFFER_SIZE, rgbs + 3) == EGL_FALSE)
            return false;

        DAVA::Logger::Info("CONFIG: [%d, %d, %d] -> %d", rgbs[0], rgbs[1], rgbs[2], rgbs[3]);

        if ((rgbs[0] == 8) && (rgbs[1] == 8) && (rgbs[2] == 8) && (rgbs[3] == 32))
        {
            DAVA::Logger::Info("Good config found.");
            _config = config;
            break;
        }
    }

    return true;
}

#define TRY_CHOOSE_CONFIG { if (TryChoseConfig(attribs)) return; }

void android_gl_chose_config()
{
    EGLint attribs[] =
    {
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_BUFFER_SIZE, 32,
      EGL_STENCIL_SIZE, 8,
      EGL_DEPTH_SIZE, 24,
      EGL_NONE, EGL_NONE,
      EGL_NONE
    };
    TRY_CHOOSE_CONFIG // 24 bit + es3

    DAVA::uint32 numAttribs = sizeof(attribs) / sizeof(attribs[0]);

    attribs[1] = EGL_OPENGL_ES2_BIT;
    TRY_CHOOSE_CONFIG // 24 bit + es2

    attribs[1] = EGL_OPENGL_ES3_BIT;
    attribs[numAttribs - 4] = 16; // depth size
    attribs[numAttribs - 3] = EGL_DEPTH_ENCODING_NV;
    attribs[numAttribs - 2] = EGL_DEPTH_ENCODING_NONLINEAR_NV;
    TRY_CHOOSE_CONFIG // 16 bit + nv + es3

    attribs[1] = EGL_OPENGL_ES2_BIT;
    TRY_CHOOSE_CONFIG // 16 bit + nv + es2

    attribs[1] = EGL_OPENGL_ES3_BIT;
    attribs[numAttribs - 3] = EGL_NONE;
    attribs[numAttribs - 2] = EGL_NONE;
    TRY_CHOOSE_CONFIG // 16 bit + es3

    attribs[1] = EGL_OPENGL_ES2_BIT;
    TRY_CHOOSE_CONFIG // 16 bit + es2

    DVASSERT_MSG(_config != nullptr, "Can't set GL configuration");
}

void android_gl_init(void* _window)
{
    _nativeWindow = static_cast<ANativeWindow*>(_window);

    _display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(_display, nullptr, nullptr);
    android_gl_chose_config();

    eglGetConfigAttrib(_display, _config, EGL_NATIVE_VISUAL_ID, &_format);

    ANativeWindow_setBuffersGeometry(_nativeWindow, _GLES2_DefaultFrameBuffer_Width, _GLES2_DefaultFrameBuffer_Height, _format);
    _surface = eglCreateWindowSurface(_display, _config, _nativeWindow, nullptr);

    _context = eglCreateContext(_display, _config, EGL_NO_CONTEXT, contextAttribs);
    DVASSERT(_context != nullptr);

    eglMakeCurrent(_display, _surface, _surface, _context);

    _GLES2_Context = _context;
}

void android_gl_reset(void* _window)
{
    _nativeWindow = static_cast<ANativeWindow*>(_window);

    if (nullptr != _nativeWindow)
    {
        ANativeWindow_setBuffersGeometry(_nativeWindow, _GLES2_DefaultFrameBuffer_Width, _GLES2_DefaultFrameBuffer_Height, _format);
        needRecreateSurface = true;
    }
    else
    {
        needRecreateSurface = false;
    }
}

void android_gl_checkSurface()
{
    if (needRecreateSurface)
    {
#if defined(__DAVAENGINE_COREV2__)
        // Why this should work I do not fully understand, but this solution works
        // For more info see SDL sources: SDL2-2.0.4\src\core\android\SDL_android.c, Java_org_libsdl_app_SDLActivity_onNativeSurfaceDestroyed function
        // Also see http://stackoverflow.com/questions/8762589/eglcreatewindowsurface-on-ics-and-switching-from-2d-to-3d
        eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
#endif

        eglDestroySurface(_display, _surface);
        _surface = eglCreateWindowSurface(_display, _config, _nativeWindow, nullptr);

        eglMakeCurrent(_display, _surface, _surface, _context);

        needRecreateSurface = false;
    }
}

bool android_gl_end_frame()
{
    EGLBoolean ret = eglSwapBuffers(_display, _surface);

    if (!ret && eglGetError() == EGL_CONTEXT_LOST)
    {
        DAVA::Logger::Error("Context Lost");
        eglDestroyContext(_display, _context);
        _GLES2_Context = _context = eglCreateContext(_display, _config, EGL_NO_CONTEXT, contextAttribs);

        eglMakeCurrent(_display, _surface, _surface, _context);

        return false; //if context was lost, return 'false' (need recreate all resources)
    }

    return true;
}

void android_gl_acquire_context()
{
    eglMakeCurrent(_display, _surface, _surface, _context);
}

void android_gl_release_context()
{
    eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void GL_APIENTRY android_gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                           GLsizei length, const GLchar* message, const void* userdata)
{
    if ((message != nullptr) && (length > 1))
    {
        DAVA::Logger::Info("OpenGL debug message (%d): %s", length, message);
    }
}

void android_gl_enable_debug()
{
    DVASSERT(_GLES2_IsDebugSupported);

    DAVA::Logger::Info("Enabling OpenGL debug...");

    GL_CALL(glEnable(GL_DEBUG_OUTPUT));
    GL_CALL(glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE));
    GL_CALL(glDebugMessageCallback(&android_gl_debug_callback, nullptr));
}

#endif
