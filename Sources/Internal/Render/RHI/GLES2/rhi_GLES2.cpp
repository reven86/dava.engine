
    #include "rhi_GLES2.h"

    #include "../rhi_Public.h"

    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "../Common/dbg_StatSet.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_gl.h"


GLint       _GLES2_Binded_FrameBuffer  = 0;
GLint       _GLES2_Default_FrameBuffer  = 0;
void        (*_Make_Current)() = nullptr;
void        (*_End_Frame)() = nullptr;

namespace rhi
{
//==============================================================================

static bool     _Inited             = false;
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
        case TEXTURE_FORMAT_R8G8B8A8 :
        case TEXTURE_FORMAT_R5G5B5A1 :
        case TEXTURE_FORMAT_R5G6B5 :
        case TEXTURE_FORMAT_R4G4B4A4 :
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

        case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA :
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
#if defined(__DAVAENGINE_WIN32__)

static void GLAPIENTRY
_OGLErrorCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userdata )
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
    if( type == GL_DEBUG_TYPE_ERROR  ||  type == GL_DEBUG_TYPE_PERFORMANCE )
        Logger::Error( "[gl.error] %s\n", message );
//    else
//        Logger::Info( "[gl.info] %s\n", message );
}

#endif // defined(__DAVAENGINE_WIN32__)


//------------------------------------------------------------------------------

void
gles2_Uninitialize()
{
    UninitializeRenderThread();
}



//------------------------------------------------------------------------------

static void
gles2_Reset( const ResetParam& param )
{
}


//------------------------------------------------------------------------------

void
gles2_Initialize( const InitParam& param )
{
    _Make_Current = param.makeCurrentFunc;
    _End_Frame = param.endFrameFunc;

    DVASSERT(_End_Frame);
    DVASSERT(_Make_Current);

    _Make_Current();
    
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_GLES2_Default_FrameBuffer);
    _GLES2_Binded_FrameBuffer = _GLES2_Default_FrameBuffer;

    VertexBufferGLES2::SetupDispatch( &DispatchGLES2 );
    IndexBufferGLES2::SetupDispatch( &DispatchGLES2 );
    TextureGLES2::SetupDispatch( &DispatchGLES2 );
    PipelineStateGLES2::SetupDispatch( &DispatchGLES2 );
    ConstBufferGLES2::SetupDispatch( &DispatchGLES2 );
    DepthStencilStateGLES2::SetupDispatch( &DispatchGLES2 );
    SamplerStateGLES2::SetupDispatch( &DispatchGLES2 );
    RenderPassGLES2::SetupDispatch( &DispatchGLES2 );
    CommandBufferGLES2::SetupDispatch( &DispatchGLES2 );

    DispatchGLES2.impl_Reset                    = &gles2_Reset;
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

#if defined(__DAVAENGINE_WIN32__)
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

    _Inited = true;
}

GLint
GL_TextureFormat( TextureFormat format )
{
    GLint   fmt = 0;
    
    switch( format ) 
    {
        case TEXTURE_FORMAT_R8G8B8      : fmt = GL_RGB; break;
        case TEXTURE_FORMAT_R8G8B8A8    : fmt = GL_RGBA; break;
/*
        TEXTURE_FORMAT_A1R5G5B5,
        TEXTURE_FORMAT_R5G6B5,


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
        case TEXTURE_FORMAT_R8G8B8A8 :
            *internalFormat = GL_RGBA;
            *format         = GL_RGBA;
            *type           = GL_UNSIGNED_BYTE;
            success         = true;
            break;

        case TEXTURE_FORMAT_R8G8B8 :
            *internalFormat = GL_RGB; 
            *format         = GL_RGB;
            *type           = GL_UNSIGNED_BYTE;
            success         = true;
            break;
        
        case TEXTURE_FORMAT_R4G4B4A4 :
            *internalFormat = GL_RGBA;
            *format         = GL_RGBA;
            *type           = GL_UNSIGNED_SHORT_4_4_4_4;
            success         = true;
            break;

        case TEXTURE_FORMAT_R8 :
            *internalFormat = GL_ALPHA; 
            *format         = GL_ALPHA;
            *type           = GL_UNSIGNED_BYTE;
            success         = true;
            break;

        case TEXTURE_FORMAT_R16:
            *internalFormat = GL_ALPHA;
            *format = GL_ALPHA;
            *type = GL_UNSIGNED_SHORT;
            success = true;
            break;

        case TEXTURE_FORMAT_A1R5G5B5:
            *internalFormat = GL_RGBA;
            *format = GL_RGBA;
            *type = GL_UNSIGNED_SHORT_5_5_5_1;
            success = true;
            break;

        case TEXTURE_FORMAT_R5G6B5:
            *internalFormat = GL_RGB;
            *format = GL_RGB;
            *type = GL_UNSIGNED_SHORT_5_6_5;
            success = true;
            break;

        case TEXTURE_FORMAT_A16R16G16B16:
            *internalFormat = GL_RGBA;
            *format = GL_RGBA;
            *type = GL_HALF_FLOAT;
            success = true;
            break;

        case TEXTURE_FORMAT_A32R32G32B32:
            *internalFormat = GL_RGBA;
            *format = GL_RGBA;
            *type = GL_FLOAT;
            success = true;
            break;

        case TEXTURE_FORMAT_DXT1:
            *internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
            *format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
            *type = GL_UNSIGNED_BYTE;
            success = true;
            break;

        case TEXTURE_FORMAT_DXT3:
            *internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            *format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            *type = GL_UNSIGNED_BYTE;
            success = true;
            break;

        case TEXTURE_FORMAT_DXT5:
            *internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            *format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            *type = GL_UNSIGNED_BYTE;
            success = true;
            break;

        case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA:
            *internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
            *format = GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
            *type = GL_UNSIGNED_BYTE;
            success = true;
            break;

        case TEXTURE_FORMAT_PVRTC2_2BPP_RGBA:
            *internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG;
            *format = GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG;
            *type = GL_UNSIGNED_BYTE;
            success = true;
            break;

        case TEXTURE_FORMAT_ETC1:
            *internalFormat = GL_ETC1_RGB8_OES;
            *format = GL_ETC1_RGB8_OES;
            *type = GL_UNSIGNED_BYTE;
            success = true;
            break;

        case TEXTURE_FORMAT_ETC2_R8G8B8:
            *internalFormat = GL_COMPRESSED_RGB8_ETC2;
            *format = GL_COMPRESSED_RGB8_ETC2;
            *type = GL_UNSIGNED_BYTE;
            success = true;
            break;

        case TEXTURE_FORMAT_ETC2_R8G8B8A8:
            *internalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
            *format = GL_COMPRESSED_RGBA8_ETC2_EAC;
            *type = GL_UNSIGNED_BYTE;
            success = true;
            break;

        case TEXTURE_FORMAT_ETC2_R8G8B8A1:
            *internalFormat = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
            *format = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
            *type = GL_UNSIGNED_BYTE;
            success = true;

        case TEXTURE_FORMAT_EAC_R11_UNSIGNED:
            *internalFormat = GL_COMPRESSED_R11_EAC;
            *format = GL_COMPRESSED_R11_EAC;
            *type = GL_UNSIGNED_BYTE;
            success = true;

        case TEXTURE_FORMAT_EAC_R11_SIGNED:
            *internalFormat = GL_COMPRESSED_SIGNED_R11_EAC;
            *format = GL_COMPRESSED_SIGNED_R11_EAC;
            *type = GL_UNSIGNED_BYTE;
            success = true;

        case TEXTURE_FORMAT_EAC_R11G11_UNSIGNED:
            *internalFormat = GL_COMPRESSED_RG11_EAC;
            *format = GL_COMPRESSED_RG11_EAC;
            *type = GL_UNSIGNED_BYTE;
            success = true;

        case TEXTURE_FORMAT_EAC_R11G11_SIGNED:
            *internalFormat = GL_COMPRESSED_SIGNED_RG11_EAC;
            *format = GL_COMPRESSED_SIGNED_RG11_EAC;
            *type = GL_UNSIGNED_BYTE;
            success = true;
            break;

        case TEXTURE_FORMAT_ATC_RGB:
            *internalFormat = GL_ATC_RGB_AMD;
            *format = GL_ATC_RGB_AMD;
            *type = GL_UNSIGNED_BYTE;
            success = true;
            break;

        case TEXTURE_FORMAT_ATC_RGBA_EXPLICIT:
            *internalFormat = GL_ATC_RGBA_EXPLICIT_ALPHA_AMD;
            *format = GL_ATC_RGBA_EXPLICIT_ALPHA_AMD;
            *type = GL_UNSIGNED_BYTE;
            success = true;
            break;

        case TEXTURE_FORMAT_ATC_RGBA_INTERPOLATED:
            *internalFormat = GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD;
            *format = GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD;
            *type = GL_UNSIGNED_BYTE;
            success = true;
            break;

        default :
            success = false;
    }

    return success;
}

