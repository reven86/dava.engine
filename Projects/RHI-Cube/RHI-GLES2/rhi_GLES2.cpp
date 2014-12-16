
    #include "rhi_GLES2.h"

    #include "../rhi_Base.h"
    #include "../RHI/rhi_Pool.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    #include "Core/Core.h"
    using DAVA::Logger;

    #include "_gl.h"


namespace rhi
{
static bool     _Inited             = false;
void*           _Context            = 0;


//------------------------------------------------------------------------------

Api
HostApi()
{
    return RHI_GLES2;
}


//------------------------------------------------------------------------------

void
Initialize()
{
    bool            success = false;
    HWND            wnd     = (HWND)DAVA::Core::Instance()->NativeWindowHandle();
    HDC             dc      = ::GetDC( wnd );

    DVASSERT(!_Inited);

    PIXELFORMATDESCRIPTOR pfd = 
    {
    sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd
    1,                                // version number
    PFD_DRAW_TO_WINDOW |              // support window
    PFD_SUPPORT_OPENGL |              // support OpenGL
    PFD_DOUBLEBUFFER,                 // double buffered
    PFD_TYPE_RGBA,                    // RGBA type
    32,                               // 32-bit color depth
    0, 0, 0, 0, 0, 0,                 // color bits ignored

    0,                                // no alpha buffer
    0,                                // shift bit ignored
    0,                                // no accumulation buffer
    0, 0, 0, 0,                       // accum bits ignored
    16,                               // 16-bit z-buffer
    0,                                // no stencil buffer
    0,                                // no auxiliary buffer
    PFD_MAIN_PLANE,                   // main layer

    0,                                // reserved
    0, 0, 0                           // layer masks ignored
    };
    int  pixel_format = ChoosePixelFormat( dc, &pfd );
    SetPixelFormat( dc, pixel_format, &pfd );
    SetMapMode( dc, MM_TEXT );


    HGLRC   ctx = wglCreateContext( dc );

    if( ctx )
    {
        Logger::Info( "GL-context created\n" );
/*
        GLint attr[] =
        {
            // here we ask for OpenGL 4.0
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            // forward compatibility mode
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            // uncomment this for Compatibility profile
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
            // we are using Core profile here
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };
*/
        wglMakeCurrent( dc, ctx );
        glewExperimental = false;
        
        if( glewInit() == GLEW_OK )
        {
/*    
            HGLRC ctx4 = wglCreateContextAttribsARB( dc, 0, attr );
            if( ctx4  &&  wglMakeCurrent( dc, ctx4 ) )
            {
    //            wglDeleteContext( ctx );
                note( "using GL 4.0\n" );
                _Context = (void*)ctx4;
            }
            else
            {
*/    
                _Context = (void*)ctx;
//            }


            ConstBufferGLES2::InitializeRingBuffer( 4*1024*1024 ); // CRAP: hardcoded default const ring-buf size

            _Inited   = true;
            success   = true;

            Logger::Info( "GL inited\n" );
            Logger::Info( "  GL version   : %s\n", glGetString( GL_VERSION ) );
            Logger::Info( "  GPU vendor   : %s\n", glGetString( GL_VENDOR ) );
            Logger::Info( "  GPU          : %s\n", glGetString( GL_RENDERER ) );
            Logger::Info( "  GLSL version : %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

            #if 0
            glEnable( GL_DEBUG_OUTPUT );
            glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE );
            glDebugMessageCallback( &_OGLErrorCallback, 0 );
            #endif
        }
        else
        {
            Logger::Error( "GLEW init failed\n" );
        }
    }
    else
    {
        Logger::Error( "can't create GL-context" ); 
    }                
}


//------------------------------------------------------------------------------

void
Uninitialize()
{
}

} // namespace rhi