
    #include "rhi_GLES2.h"

    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "../Common/dbg_StatSet.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    #include "Core/Core.h"
    using DAVA::Logger;

    #include "_gl.h"


GLuint      _GLES2_FrameBuffer  = 0;
GLint       _GLES2_Viewport[4];


namespace rhi
{
//==============================================================================

static bool     _Inited             = false;
void*           _Context            = 0;
Dispatch        DispatchGLES2       = {0};

static bool     ATC_Supported       = false;
static bool     PVRTC2_Supported    = false;
static bool     ETC1_Supported      = false;
static bool     ETC2_Supported      = false;
static bool     EAC_Supported       = false;
static bool     Float_Supported     = false;
static bool     Half_Supported      = false;


//------------------------------------------------------------------------------

static Api
gles2_HostApi()
{
    return RHI_GLES2;
}


//------------------------------------------------------------------------------

static bool
gles2_TextureFormatSupported( TextureFormat format )
{
    bool    supported = false;
    
    switch( format )
    {
        case TEXTURE_FORMAT_A8R8G8B8 :
        case TEXTURE_FORMAT_A1R5G5B5 :
        case TEXTURE_FORMAT_R5G6B5 :
        case TEXTURE_FORMAT_A4R4G4B4 :
        case TEXTURE_FORMAT_R8 :
        case TEXTURE_FORMAT_R16 :
        case TEXTURE_FORMAT_DXT1 :
        case TEXTURE_FORMAT_DXT3 :
        case TEXTURE_FORMAT_DXT5 :
            supported = true;
            break;
        
        case TEXTURE_FORMAT_A16R16G16B16 :
            supported = Half_Supported;
            break;

        case TEXTURE_FORMAT_A32R32G32B32 :
            supported = Float_Supported;
            break;

        case TEXTURE_FORMAT_PVRTC2_4BPP_RGB :
        case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA :
        case TEXTURE_FORMAT_PVRTC2_2BPP_RGB :
        case TEXTURE_FORMAT_PVRTC2_2BPP_RGBA :
            supported = PVRTC2_Supported;
            break;

        case TEXTURE_FORMAT_ATC_RGB :
        case TEXTURE_FORMAT_ATC_RGBA_EXPLICIT :
        case TEXTURE_FORMAT_ATC_RGBA_INTERPOLATED :
            supported = ATC_Supported;
            break;

        case TEXTURE_FORMAT_ETC1 :
            supported = ETC1_Supported;
            break;

        case TEXTURE_FORMAT_ETC2_R8G8B8 :
        case TEXTURE_FORMAT_ETC2_R8G8B8A8 :
        case TEXTURE_FORMAT_ETC2_R8G8B8A1 :
            supported = ETC2_Supported;
            break;
            break;

        case TEXTURE_FORMAT_EAC_R11_UNSIGNED :
        case TEXTURE_FORMAT_EAC_R11_SIGNED :
        case TEXTURE_FORMAT_EAC_R11G11_UNSIGNED :
        case TEXTURE_FORMAT_EAC_R11G11_SIGNED :
            supported = EAC_Supported;
            break;
        
        default :
            supported = false;
    }

    return supported;
}

    
//------------------------------------------------------------------------------
/*
static void GLAPIENTRY
_OGLErrorCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userdata )
{   

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

    if( type == GL_DEBUG_TYPE_ERROR  ||  type == GL_DEBUG_TYPE_PERFORMANCE )
        Logger::Error( "[gl] %s\n", message );
}
*/
//------------------------------------------------------------------------------

void
gles2_Uninitialize()
{
    UninitializeRenderThread();
}


//------------------------------------------------------------------------------

#if defined(__DAVAENGINE_WIN32__)
void
gles2_Initialize()
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

            VertexBufferGLES2::SetupDispatch( &DispatchGLES2 );
            IndexBufferGLES2::SetupDispatch( &DispatchGLES2 );
            TextureGLES2::SetupDispatch( &DispatchGLES2 );
            PipelineStateGLES2::SetupDispatch( &DispatchGLES2 );
            ConstBufferGLES2::SetupDispatch( &DispatchGLES2 );
            DepthStencilStateGLES2::SetupDispatch( &DispatchGLES2 );
            SamplerStateGLES2::SetupDispatch( &DispatchGLES2 );
            RenderPassGLES2::SetupDispatch( &DispatchGLES2 );
            CommandBufferGLES2::SetupDispatch( &DispatchGLES2 );

            DispatchGLES2.impl_Uninitialize             = &gles2_Uninitialize;
            DispatchGLES2.impl_HostApi                  = &gles2_HostApi;
            DispatchGLES2.impl_TextureFormatSupported   = &gles2_TextureFormatSupported;

            SetDispatchTable( DispatchGLES2 );
            
            // check GL extensions
            {
                const char* ext = (const char*)glGetString( GL_EXTENSIONS );

                if( !IsEmptyString(ext) )
                {
                    ATC_Supported    = strstr( ext, "GL_AMD_compressed_ATC_texture" ) != nullptr;
                    PVRTC2_Supported = strstr( ext, "GL_IMG_texture_compression_pvrtc2" ) != nullptr;
                    ETC1_Supported   = strstr( ext, "GL_OES_compressed_ETC1_RGB8_texture" ) != nullptr;
                    ETC2_Supported   = strstr( ext, "GL_OES_compressed_ETC2_RGB8_texture" ) != nullptr;
                    EAC_Supported    = ETC2_Supported;
                    Float_Supported  = strstr( ext, "GL_OES_texture_float" ) != nullptr;
                    Half_Supported   = strstr( ext, "GL_OES_texture_half_float" ) != nullptr;
                }
            }




            ConstBufferGLES2::InitializeRingBuffer( 4*1024*1024 ); // CRAP: hardcoded default const ring-buf size

            _Inited   = true;
            success   = true;

            Logger::Info( "GL inited\n" );
            Logger::Info( "  GL version   : %s", glGetString( GL_VERSION ) );
            Logger::Info( "  GPU vendor   : %s", glGetString( GL_VENDOR ) );
            Logger::Info( "  GPU          : %s", glGetString( GL_RENDERER ) );
            Logger::Info( "  GLSL version : %s", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

            #if 0
            glEnable( GL_DEBUG_OUTPUT );
            glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE );
            glDebugMessageCallback( &_OGLErrorCallback, 0 );
            #endif

            stat_DIP        = StatSet::AddStat( "rhi'dip", "dip" );
            stat_DP         = StatSet::AddStat( "rhi'dp", "dp" );
            stat_SET_PS     = StatSet::AddStat( "rhi'set-ps", "set-ps" );
            stat_SET_TEX    = StatSet::AddStat( "rhi'set-tex", "set-tex" );
            stat_SET_CB     = StatSet::AddStat( "rhi'set-cb", "set-cb" );



            InitializeRenderThread();
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

#elif defined(__DAVAENGINE_MACOS__)

void
gles2_Initialize()
{
    ConstBufferGLES2::InitializeRingBuffer( 4*1024*1024 ); // CRAP: hardcoded default const ring-buf size
    
    _Inited = true;
    
    Logger::Info( "GL inited" );
    Logger::Info( "  GL version   : %s", glGetString( GL_VERSION ) );
    Logger::Info( "  GPU vendor   : %s", glGetString( GL_VENDOR ) );
    Logger::Info( "  GPU          : %s", glGetString( GL_RENDERER ) );
    Logger::Info( "  GLSL version : %s", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

    VertexBufferGLES2::SetupDispatch( &DispatchGLES2 );
    IndexBufferGLES2::SetupDispatch( &DispatchGLES2 );
    TextureGLES2::SetupDispatch( &DispatchGLES2 );
    PipelineStateGLES2::SetupDispatch( &DispatchGLES2 );
    ConstBufferGLES2::SetupDispatch( &DispatchGLES2 );
    DepthStencilStateGLES2::SetupDispatch( &DispatchGLES2 );
    SamplerStateGLES2::SetupDispatch( &DispatchGLES2 );
    RenderPassGLES2::SetupDispatch( &DispatchGLES2 );
    CommandBufferGLES2::SetupDispatch( &DispatchGLES2 );
    
    DispatchGLES2.impl_Uninitialize = &gles2_Uninitialize;
    DispatchGLES2.impl_HostApi      = &gles2_HostApi;
    
    SetDispatchTable( DispatchGLES2 );

    InitializeRenderThread();
    
#if 0
    glEnable( GL_DEBUG_OUTPUT );
    glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE );
    glDebugMessageCallback( &_OGLErrorCallback, 0 );
#endif

    stat_DIP        = StatSet::AddStat( "rhi'dip", "dip" );
    stat_DP         = StatSet::AddStat( "rhi'dp", "dp" );
    stat_SET_PS     = StatSet::AddStat( "rhi'set-ps", "set-ps" );
    stat_SET_TEX    = StatSet::AddStat( "rhi'set-tex", "set-tex" );
    stat_SET_CB     = StatSet::AddStat( "rhi'set-cb", "set-cb" );
}
    
#elif defined(__DAVAENGINE_IPHONE__)
    
void
gles2_Initialize()
{
    ios_GL_init();

    ConstBufferGLES2::InitializeRingBuffer( 4*1024*1024 ); // CRAP: hardcoded default const ring-buf size
        
    _Inited = true;
        
    Logger::Info( "GL inited" );
    Logger::Info( "  GL version   : %s", glGetString( GL_VERSION ) );
    Logger::Info( "  GPU vendor   : %s", glGetString( GL_VENDOR ) );
    Logger::Info( "  GPU          : %s", glGetString( GL_RENDERER ) );
    Logger::Info( "  GLSL version : %s", glGetString( GL_SHADING_LANGUAGE_VERSION ) );
        
    VertexBufferGLES2::SetupDispatch( &DispatchGLES2 );
    IndexBufferGLES2::SetupDispatch( &DispatchGLES2 );
    TextureGLES2::SetupDispatch( &DispatchGLES2 );
    PipelineStateGLES2::SetupDispatch( &DispatchGLES2 );
    ConstBufferGLES2::SetupDispatch( &DispatchGLES2 );
    DepthStencilStateGLES2::SetupDispatch( &DispatchGLES2 );
    SamplerStateGLES2::SetupDispatch( &DispatchGLES2 );
    RenderPassGLES2::SetupDispatch( &DispatchGLES2 );
    CommandBufferGLES2::SetupDispatch( &DispatchGLES2 );
    
    DispatchGLES2.impl_Uninitialize = &gles2_Uninitialize;
    DispatchGLES2.impl_HostApi      = &gles2_HostApi;
    
    SetDispatchTable( DispatchGLES2 );

    InitializeRenderThread();

#if 0
    glEnable( GL_DEBUG_OUTPUT );
    glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE );
    glDebugMessageCallback( &_OGLErrorCallback, 0 );
#endif

    stat_DIP        = StatSet::AddStat( "rhi'dip", "dip" );
    stat_DP         = StatSet::AddStat( "rhi'dp", "dp" );
    stat_SET_PS     = StatSet::AddStat( "rhi'set-ps", "set-ps" );
    stat_SET_TEX    = StatSet::AddStat( "rhi'set-tex", "set-tex" );
    stat_SET_CB     = StatSet::AddStat( "rhi'set-cb", "set-cb" );
}
    
#endif


GLint
GL_TextureFormat( TextureFormat format )
{
    GLint   fmt = 0;
    
    switch( format ) 
    {
        case TEXTURE_FORMAT_R8G8B8      : fmt = GL_RGB; break;
        case TEXTURE_FORMAT_A8R8G8B8    : fmt = GL_RGBA; break;
/*
        TEXTURE_FORMAT_A1R5G5B5,
        TEXTURE_FORMAT_R5G6B5,

        TEXTURE_FORMAT_A4R4G4B4,

        TEXTURE_FORMAT_A16R16G16B16,
        TEXTURE_FORMAT_A32R32G32B32,

        TEXTURE_FORMAT_R8,
        TEXTURE_FORMAT_R16,

        TEXTURE_FORMAT_DXT1,
        TEXTURE_FORMAT_DXT3,
        TEXTURE_FORMAT_DXT5,

        TEXTURE_FORMAT_PVRTC2_4BPP_RGB,
        TEXTURE_FORMAT_PVRTC2_4BPP_RGBA,
        TEXTURE_FORMAT_PVRTC2_2BPP_RGB,
        TEXTURE_FORMAT_PVRTC2_2BPP_RGBA,

        TEXTURE_FORMAT_ATC_RGB,
        TEXTURE_FORMAT_ATC_RGBA_EXPLICIT,
        TEXTURE_FORMAT_ATC_RGBA_INTERPOLATED,

        TEXTURE_FORMAT_ETC1,
        TEXTURE_FORMAT_ETC2_R8G8B8,
        TEXTURE_FORMAT_ETC2_R8G8B8A8,
        TEXTURE_FORMAT_ETC2_R8G8B8A1,

        TEXTURE_FORMAT_EAC_R11_UNSIGNED,
        TEXTURE_FORMAT_EAC_R11_SIGNED,
        TEXTURE_FORMAT_EAC_R11G11_UNSIGNED,
        TEXTURE_FORMAT_EAC_R11G11_SIGNED,

        TEXTURE_FORMAT_D16,
        TEXTURE_FORMAT_D24S8
*/    
        default :
            {}
    }

    return fmt;
}
    
   
} // namespace rhi


bool
GetGLTextureFormat( rhi::TextureFormat rhiFormat, GLint* internalFormat, GLint* format, GLenum* type )
{
    using namespace rhi;

    bool    success = false;
    
    switch( rhiFormat )
    {
        case TEXTURE_FORMAT_A8R8G8B8 :
            *internalFormat = GL_RGBA; 
            *format         = GL_BGRA;
            *type           = GL_UNSIGNED_BYTE;
            success         = true;
            break;

        case TEXTURE_FORMAT_R8G8B8 :
            *internalFormat = GL_RGB; 
            *format         = GL_RGB;
            *type           = GL_UNSIGNED_BYTE;
            success         = true;
            break;
            
        default :
            success = false;
    }

    return success;
}

