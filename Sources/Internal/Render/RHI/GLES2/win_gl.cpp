#include "win_gl.h"

#if defined(__DAVAENGINE_WIN32__)

#include "_gl.h"
#include "../rhi_Public.h"
using DAVA::Logger;

void win_gl_reset(const rhi::ResetParam& param)
{
    if (wglSwapIntervalEXT != nullptr)
    {
        wglSwapIntervalEXT(param.vsyncEnabled ? 1 : 0);
    }
}

void win32_gl_init(const rhi::InitParam& param)
{
    _GLES2_Native_Window = param.window;

    bool success = false;

    if (_GLES2_Native_Window)
    {
        _GLES2_WindowDC = ::GetDC((HWND)_GLES2_Native_Window);

        PIXELFORMATDESCRIPTOR pfd =
        {
          sizeof(PIXELFORMATDESCRIPTOR), // size of this pfd
          1, // version number
          PFD_DRAW_TO_WINDOW | // support window
          PFD_SUPPORT_OPENGL | // support OpenGL
          PFD_DOUBLEBUFFER, // double buffered
          PFD_TYPE_RGBA, // RGBA type
          32, // 32-bit color depth
          0,
          0, 0, 0, 0, 0, // color bits ignored

          0, // no alpha buffer
          0, // shift bit ignored
          0, // no accumulation buffer
          0, 0, 0, 0, // accum bits ignored
          24, // 24-bit z-buffer
          8, // 8-bit stencil buffer
          0, // no auxiliary buffer
          PFD_MAIN_PLANE, // main layer

          0, // reserved
          0, 0, 0 // layer masks ignored
        };
        int pixel_format = ChoosePixelFormat(_GLES2_WindowDC, &pfd);
        SetPixelFormat(_GLES2_WindowDC, pixel_format, &pfd);
        SetMapMode(_GLES2_WindowDC, MM_TEXT);

        HGLRC ctx = wglCreateContext(_GLES2_WindowDC);

        if (ctx)
        {
            Logger::Info("GL-context created\n");

            GLint attr[] =
            {
              // here we ask for OpenGL version
              WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
              WGL_CONTEXT_MINOR_VERSION_ARB, 2,
              // forward compatibility mode
              WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
              // uncomment this for Compatibility profile
              WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
              // we are using Core profile here
              WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
              0
            };

            wglMakeCurrent(_GLES2_WindowDC, ctx);
            glewExperimental = false;

            if (glewInit() == GLEW_OK)
            {
                HGLRC ctx4 = 0; //wglCreateContextAttribsARB( _GLES2_WindowDC, 0, attr );
                if (ctx4 && wglMakeCurrent(_GLES2_WindowDC, ctx4))
                {
                    wglDeleteContext(ctx);
                    Logger::Info("using GL %i.%i", attr[1], attr[3]);
                    _GLES2_Context = (void*)ctx4;
                }
                else
                {
                    _GLES2_Context = (void*)ctx;
                }

                success = true;
            }
            else
            {
                Logger::Error("GLEW init failed\n");
            }
        }
        else
        {
            Logger::Error("can't create GL-context");
        }
    }
    else
    {
        if (glewInit() == GLEW_OK)
            success = true;
    }
    DVASSERT(success); //not sure assert should be here, so if afterwards
    if (success)
    {
        if (_GLES2_Native_Window)
        {
            RECT rc;
            GetClientRect((HWND)_GLES2_Native_Window, &rc);
            _GLES2_DefaultFrameBuffer_Width = rc.right - rc.left;
            _GLES2_DefaultFrameBuffer_Height = rc.bottom - rc.top;
        }
        if (wglSwapIntervalEXT != nullptr)
        {
            wglSwapIntervalEXT(param.vsyncEnabled ? 1 : 0);
            DAVA::Logger::Info("GLES2 V-Sync: %s", param.vsyncEnabled ? "ON" : "OFF");
        }
    }
}

void win32_gl_acquire_context()
{
    wglMakeCurrent(_GLES2_WindowDC, (HGLRC)_GLES2_Context);
}

void win32_gl_release_context()
{
    wglMakeCurrent(NULL, NULL);
}

void GLAPIENTRY win32_gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userdata)
{
    /*
    const char* ssource     = "unknown";
    const char* stype       = "unknown";
    const char* sseverity   = "unknown";

    switch( source )
    {
    case GL_DEBUG_SOURCE_API                : ssource = "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM      : ssource = "window system"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER    : ssource = "shader compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY        : ssource = "third party"; break;
    case GL_DEBUG_SOURCE_APPLICATION        : ssource = "application"; break;
    case GL_DEBUG_SOURCE_OTHER              : ssource = "other"; break;
    default                                 : ssource= "unknown"; break;
    }

    switch( type )
    {
    case GL_DEBUG_TYPE_ERROR                : stype = "error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR  : stype = "deprecated behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR   : stype = "undefined behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY          : stype = "portabiliy"; break;
    case GL_DEBUG_TYPE_PERFORMANCE          : stype = "performance"; break;
    case GL_DEBUG_TYPE_OTHER                : stype = "other"; break;
    default                                 : stype = "unknown"; break;
    }

    switch( severity )
    {
    case GL_DEBUG_SEVERITY_HIGH             : sseverity = "high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM           : sseverity = "medium"; break;
    case GL_DEBUG_SEVERITY_LOW              : sseverity = "low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION     : sseverity = "notification"; break;
    default                                 : sseverity = "unknown"; break;
    }
    */
    if (type == GL_DEBUG_TYPE_PERFORMANCE)
        Trace("[gl.warning] %s\n", message);
    else if (type == GL_DEBUG_TYPE_ERROR)
        Trace("[gl.error] %s\n", message);
    //    else
    //        Logger::Info( "[gl] %s\n", message );
}


#endif
