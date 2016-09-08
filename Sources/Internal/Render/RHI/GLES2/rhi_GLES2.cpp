#include "rhi_GLES2.h"

#include "../rhi_Public.h"

#include "../Common/rhi_Private.h"
#include "../Common/rhi_Pool.h"
#include "../Common/rhi_Utils.h"
#include "../Common/dbg_StatSet.h"
#include "../Common/CommonImpl.h"

#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
using DAVA::Logger;

#include "_gl.h"

#if !defined(GL_RGBA32F) && defined(GL_RGBA32F_ARB)
#define GL_RGBA32F GL_RGBA32F_ARB
#endif
#if !defined(GL_RGBA32F) && defined(GL_RGBA32F_EXT)
#define GL_RGBA32F GL_RGBA32F_EXT
#endif

#if !defined(GL_RGBA16F) && defined(GL_RGBA16F_ARB)
#define GL_RGBA16F GL_RGBA16F_ARB
#endif
#if !defined(GL_RGBA16F) && defined(GL_RGBA16F_EXT)
#define GL_RGBA16F GL_RGBA16F_EXT
#endif

GLuint _GLES2_Binded_FrameBuffer = 0;
GLuint _GLES2_Default_FrameBuffer = 0;
void* _GLES2_Native_Window = nullptr;
void* _GLES2_Context = nullptr;
void (*_GLES2_AcquireContext)() = nullptr;
void (*_GLES2_ReleaseContext)() = nullptr;
int _GLES2_DefaultFrameBuffer_Width = 0;
int _GLES2_DefaultFrameBuffer_Height = 0;
GLuint _GLES2_LastSetIB = 0;
DAVA::uint8* _GLES2_LastSetIndices = nullptr;
GLuint _GLES2_LastSetVB = 0;
GLuint _GLES2_LastSetTex0 = 0;
GLenum _GLES2_LastSetTex0Target = GL_TEXTURE_2D;
int _GLES2_LastActiveTexture = -1;
bool _GLES2_IsGlDepth24Stencil8Supported = true;
bool _GLES2_IsGlDepthNvNonLinearSupported = false;
bool _GLES2_IsSeamlessCubmapSupported = false;
bool _GLES2_UseUserProvidedIndices = false;
volatile bool _GLES2_ValidateNeonCalleeSavedRegisters = false;

DAVA::uint8 volatile pre_call_registers[64];

#if defined(__DAVAENGINE_WIN32__)
HDC _GLES2_WindowDC = 0;
#endif

namespace rhi
{
//==============================================================================

Dispatch DispatchGLES2 = { 0 };

static bool ATC_Supported = false;
static bool PVRTC_Supported = false;
static bool PVRTC2_Supported = false;
static bool ETC1_Supported = false;
static bool ETC2_Supported = false;
static bool EAC_Supported = false;
static bool DXT_Supported = false;
static bool Float_Supported = false;
static bool Half_Supported = false;
static bool RG_Supported = false;
static bool Short_Int_Supported = false;

static RenderDeviceCaps _GLES2_DeviceCaps = {};

//------------------------------------------------------------------------------

static Api
gles2_HostApi()
{
    return RHI_GLES2;
}

//------------------------------------------------------------------------------

static const RenderDeviceCaps& gles2_DeviceCaps()
{
    return _GLES2_DeviceCaps;
}

//------------------------------------------------------------------------------

static bool
gles2_TextureFormatSupported(TextureFormat format)
{
    bool supported = false;

    switch (format)
    {
    case TEXTURE_FORMAT_R8G8B8A8:
    case TEXTURE_FORMAT_R8G8B8:
    case TEXTURE_FORMAT_R5G5B5A1:
    case TEXTURE_FORMAT_R5G6B5:
    case TEXTURE_FORMAT_R4G4B4A4:
    case TEXTURE_FORMAT_R8:
    case TEXTURE_FORMAT_R16:
        supported = true;
        break;

    case TEXTURE_FORMAT_DXT1:
    case TEXTURE_FORMAT_DXT3:
    case TEXTURE_FORMAT_DXT5:
        supported = DXT_Supported;
        break;

    case TEXTURE_FORMAT_A16R16G16B16:
    case TEXTURE_FORMAT_A32R32G32B32:
        supported = Short_Int_Supported;
        break;

    case TEXTURE_FORMAT_RGBA16F:
        supported = Half_Supported;
        break;

    case TEXTURE_FORMAT_RGBA32F:
        supported = Float_Supported;
        break;

    case TEXTURE_FORMAT_R16F:
    case TEXTURE_FORMAT_RG16F:
        supported = Half_Supported && RG_Supported;

    case TEXTURE_FORMAT_R32F:
    case TEXTURE_FORMAT_RG32F:
        supported = Float_Supported && RG_Supported;

    case TEXTURE_FORMAT_PVRTC_4BPP_RGBA:
    case TEXTURE_FORMAT_PVRTC_2BPP_RGBA:
        supported = PVRTC_Supported;
        break;

    case TEXTURE_FORMAT_PVRTC2_4BPP_RGB:
    case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA:
    case TEXTURE_FORMAT_PVRTC2_2BPP_RGB:
    case TEXTURE_FORMAT_PVRTC2_2BPP_RGBA:
        supported = PVRTC2_Supported;
        break;

    case TEXTURE_FORMAT_ATC_RGB:
    case TEXTURE_FORMAT_ATC_RGBA_EXPLICIT:
    case TEXTURE_FORMAT_ATC_RGBA_INTERPOLATED:
        supported = ATC_Supported;
        break;

    case TEXTURE_FORMAT_ETC1:
        supported = ETC1_Supported;
        break;

    case TEXTURE_FORMAT_ETC2_R8G8B8:
    case TEXTURE_FORMAT_ETC2_R8G8B8A8:
    case TEXTURE_FORMAT_ETC2_R8G8B8A1:
        supported = ETC2_Supported;
        break;

    case TEXTURE_FORMAT_EAC_R11_UNSIGNED:
    case TEXTURE_FORMAT_EAC_R11_SIGNED:
    case TEXTURE_FORMAT_EAC_R11G11_UNSIGNED:
    case TEXTURE_FORMAT_EAC_R11G11_SIGNED:
        supported = EAC_Supported;
        break;

    default:
        supported = false;
    }

    return supported;
}

static void gles_check_GL_extensions()
{
    const char* ext = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));

    if (!IsEmptyString(ext))
    {
        ATC_Supported = strstr(ext, "GL_AMD_compressed_ATC_texture") != nullptr;
        PVRTC_Supported = strstr(ext, "GL_IMG_texture_compression_pvrtc") != nullptr;
        PVRTC2_Supported = strstr(ext, "GL_IMG_texture_compression_pvrtc2") != nullptr;
        ETC1_Supported = strstr(ext, "GL_OES_compressed_ETC1_RGB8_texture") != nullptr;
        ETC2_Supported = strstr(ext, "GL_OES_compressed_ETC2_RGB8_texture") != nullptr;
        EAC_Supported = ETC2_Supported;
        DXT_Supported = (strstr(ext, "GL_EXT_texture_compression_s3tc") != nullptr) || (strstr(ext, "GL_NV_texture_compression_s3tc") != nullptr);

        Float_Supported = strstr(ext, "GL_OES_texture_float") != nullptr || strstr(ext, "ARB_texture_float") != nullptr;
        Half_Supported = strstr(ext, "GL_OES_texture_half_float") != nullptr || strstr(ext, "ARB_texture_float") != nullptr;
        RG_Supported = strstr(ext, "EXT_texture_rg") != nullptr || strstr(ext, "ARB_texture_rg") != nullptr;

        _GLES2_DeviceCaps.is32BitIndicesSupported = strstr(ext, "GL_OES_element_index_uint") != nullptr;
        _GLES2_DeviceCaps.isVertexTextureUnitsSupported = strstr(ext, "GL_EXT_shader_texture_lod") != nullptr;
        _GLES2_DeviceCaps.isFramebufferFetchSupported = strstr(ext, "GL_EXT_shader_framebuffer_fetch") != nullptr;

        _GLES2_DeviceCaps.isInstancingSupported =
        (strstr(ext, "GL_EXT_draw_instanced") || strstr(ext, "GL_ARB_draw_instanced") || strstr(ext, "GL_ARB_draw_elements_base_vertex")) &&
        (strstr(ext, "GL_EXT_instanced_arrays") || strstr(ext, "GL_ARB_instanced_arrays"));

#if defined(__DAVAENGINE_ANDROID__)
        _GLES2_IsGlDepth24Stencil8Supported = (strstr(ext, "GL_DEPTH24_STENCIL8") != nullptr) || (strstr(ext, "GL_OES_packed_depth_stencil") != nullptr) || (strstr(ext, "GL_EXT_packed_depth_stencil") != nullptr);
#else
        _GLES2_IsGlDepth24Stencil8Supported = true;
#endif

        _GLES2_IsGlDepthNvNonLinearSupported = strstr(ext, "GL_DEPTH_COMPONENT16_NONLINEAR_NV") != nullptr;

        _GLES2_IsSeamlessCubmapSupported = strstr(ext, "GL_ARB_seamless_cube_map") != nullptr;

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
        if (_GLES2_IsSeamlessCubmapSupported)
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif
        if (strstr(ext, "EXT_texture_filter_anisotropic") != nullptr)
        {
            float32 value = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &value);
            _GLES2_DeviceCaps.maxAnisotropy = static_cast<DAVA::uint32>(value);
        }
    }

    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    if (!IsEmptyString(version))
    {
        int majorVersion = 2, minorVersion = 0;
        const char* dotChar = strchr(version, '.');
        if (dotChar && dotChar != version && *(dotChar + 1))
        {
            majorVersion = atoi(dotChar - 1);
            minorVersion = atoi(dotChar + 1);
        }

        if (strstr(version, "OpenGL ES"))
        {
            if (majorVersion >= 3)
            {
                _GLES2_DeviceCaps.is32BitIndicesSupported = true;
                _GLES2_DeviceCaps.isVertexTextureUnitsSupported = true;
                _GLES2_DeviceCaps.isInstancingSupported = true;
                Short_Int_Supported = true;
            }
#ifdef __DAVAENGINE_ANDROID__
            if (majorVersion >= 3)
            {
                glDrawElementsInstanced = (PFNGLEGL_GLDRAWELEMENTSINSTANCED)eglGetProcAddress("glDrawElementsInstanced");
                glDrawArraysInstanced = (PFNGLEGL_GLDRAWARRAYSINSTANCED)eglGetProcAddress("glDrawArraysInstanced");
                glVertexAttribDivisor = (PFNGLEGL_GLVERTEXATTRIBDIVISOR)eglGetProcAddress("glVertexAttribDivisor");
            }
            else
            {
                glDrawElementsInstanced = (PFNGLEGL_GLDRAWELEMENTSINSTANCED)eglGetProcAddress("glDrawElementsInstancedEXT");
                glDrawArraysInstanced = (PFNGLEGL_GLDRAWARRAYSINSTANCED)eglGetProcAddress("glDrawArraysInstancedEXT");
                glVertexAttribDivisor = (PFNGLEGL_GLVERTEXATTRIBDIVISOR)eglGetProcAddress("glVertexAttribDivisorEXT");
            }
#endif
        }
        else
        {
            _GLES2_DeviceCaps.is32BitIndicesSupported = true;
            _GLES2_DeviceCaps.isVertexTextureUnitsSupported = true;
            _GLES2_DeviceCaps.isFramebufferFetchSupported = false;
            _GLES2_DeviceCaps.isInstancingSupported |= (majorVersion > 3) && (minorVersion > 3);

            if (majorVersion >= 3)
            {
                if ((majorVersion > 3) || (minorVersion >= 2))
                    _GLES2_IsSeamlessCubmapSupported = true;
            }

            Float_Supported |= majorVersion >= 3;
            Half_Supported |= majorVersion >= 3;
            RG_Supported |= majorVersion >= 3;
            Short_Int_Supported = true;
        }
    }

    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    if (!IsEmptyString(renderer))
    {
        memcpy(_GLES2_DeviceCaps.deviceDescription, renderer, strlen(renderer));

        if (strstr(renderer, "Mali"))
        {
            // drawing from memory is worst case scenario,
            // unless running on some buggy piece of shit
            _GLES2_UseUserProvidedIndices = true;
        }

        if (strcmp(renderer, "NVIDIA Tegra") == 0)
        {
            //Without offensive language:
            //it seems like some GL-functions in SHIELD driver implementation
            //corrupt 'callee-saved' Neon registers (q4-q7).
            //So, we just restore it after any GL-call.
            _GLES2_ValidateNeonCalleeSavedRegisters = true;
        }
    }
}

//------------------------------------------------------------------------------
#if defined(__DAVAENGINE_WIN32__)

static void GLAPIENTRY
_OGLErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userdata)
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

#endif // defined(__DAVAENGINE_WIN32__)

//------------------------------------------------------------------------------

void gles2_Uninitialize()
{
    //TODO: release GL resources
    //now it's crash cause Qt context deleted before uninit renderer
    //QueryBufferGLES2::ReleaseQueryObjectsPool();
    //UninitializeRenderThreadGLES2();
}

//------------------------------------------------------------------------------

static void gles2_Reset(const ResetParam& param)
{
    _GLES2_DefaultFrameBuffer_Width = param.width;
    _GLES2_DefaultFrameBuffer_Height = param.height;
#if defined(__DAVAENGINE_ANDROID__)
    android_gl_reset(param.window);
#elif defined(__DAVAENGINE_IPHONE__)
    ios_gl_reset(param.window);
#elif defined(__DAVAENGINE_MACOS__)
    macos_gl_reset(param);
#elif defined(__DAVAENGINE_WIN32__)
    win_gl_reset(param);
#endif
}

//------------------------------------------------------------------------------

static void
gles2_InvalidateCache()
{
    PipelineStateGLES2::InvalidateCache();
    DepthStencilStateGLES2::InvalidateCache();
    TextureGLES2::InvalidateCache();
}

//------------------------------------------------------------------------------

static bool
gles2_NeedRestoreResources()
{
    bool needRestore = TextureGLES2::NeedRestoreCount() || VertexBufferGLES2::NeedRestoreCount() || IndexBufferGLES2::NeedRestoreCount();

    return needRestore;
}

static void gles2_CheckSurface()
{
#if defined __DAVAENGINE_ANDROID__
    android_gl_checkSurface();
#elif defined __DAVAENGINE_IPHONE__
    ios_gl_check_layer();
#endif
}

static void gles2_Suspend()
{
    GL_CALL(glFinish());
}

//------------------------------------------------------------------------------

void gles2_Initialize(const InitParam& param)
{
    _GLES2_DefaultFrameBuffer_Width = param.width;
    _GLES2_DefaultFrameBuffer_Height = param.height;

#define ENABLE_DEBUG_OUTPUT 0
#if defined(__DAVAENGINE_WIN32__)
    win32_gl_init(param);
    _GLES2_AcquireContext = (param.acquireContextFunc) ? param.acquireContextFunc : &win32_gl_acquire_context;
    _GLES2_ReleaseContext = (param.releaseContextFunc) ? param.releaseContextFunc : &win32_gl_release_context;
    #define ENABLE_DEBUG_OUTPUT 1
#elif defined(__DAVAENGINE_MACOS__)
    macos_gl_init(param);
    _GLES2_AcquireContext = (param.acquireContextFunc) ? param.acquireContextFunc : &macos_gl_acquire_context;
    _GLES2_ReleaseContext = (param.releaseContextFunc) ? param.releaseContextFunc : &macos_gl_release_context;    
#elif defined(__DAVAENGINE_IPHONE__)
    ios_gl_init(param.window);
    _GLES2_AcquireContext = (param.acquireContextFunc) ? param.acquireContextFunc : &ios_gl_acquire_context;
    _GLES2_ReleaseContext = (param.releaseContextFunc) ? param.releaseContextFunc : &ios_gl_release_context;
#elif defined(__DAVAENGINE_ANDROID__)
    android_gl_init(param.window);
    _GLES2_AcquireContext = &android_gl_acquire_context;
    _GLES2_ReleaseContext = &android_gl_release_context;                
#endif

    DispatchPlatform::InitContext = _GLES2_AcquireContext;
    DispatchPlatform::CheckSurface = &gles2_CheckSurface;
    DispatchPlatform::Suspend = &gles2_Suspend;

    if (param.maxVertexBufferCount)
        VertexBufferGLES2::Init(param.maxVertexBufferCount);
    if (param.maxIndexBufferCount)
        IndexBufferGLES2::Init(param.maxIndexBufferCount);
    if (param.maxConstBufferCount)
        ConstBufferGLES2::Init(param.maxConstBufferCount);
    if (param.maxTextureCount)
        TextureGLES2::Init(param.maxTextureCount);

    if (param.maxTextureSetCount)
        InitTextreSetPool(param.maxTextureSetCount);
    if (param.maxSamplerStateCount)
        SamplerStateGLES2::Init(param.maxSamplerStateCount);
    if (param.maxPipelineStateCount)
        PipelineStateGLES2::Init(param.maxPipelineStateCount);
    if (param.maxDepthStencilStateCount)
        DepthStencilStateGLES2::Init(param.maxDepthStencilStateCount);
    if (param.maxRenderPassCount)
        RenderPassGLES2::Init(param.maxRenderPassCount);
    if (param.maxCommandBuffer)
        CommandBufferGLES2::Init(param.maxCommandBuffer);
    if (param.maxPacketListCount)
        InitPacketListPool(param.maxPacketListCount);

    uint32 ringBufferSize = 4 * 1024 * 1024;
    if (param.shaderConstRingBufferSize)
        ringBufferSize = param.shaderConstRingBufferSize;
    ConstBufferGLES2::InitializeRingBuffer(ringBufferSize);

    Logger::FrameworkDebug("GL inited\n");
    Logger::FrameworkDebug("  GL version   : %s", glGetString(GL_VERSION));
    Logger::FrameworkDebug("  GPU vendor   : %s", glGetString(GL_VENDOR));
    Logger::FrameworkDebug("  GPU          : %s", glGetString(GL_RENDERER));
    Logger::FrameworkDebug("  GLSL version : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    VertexBufferGLES2::SetupDispatch(&DispatchGLES2);
    IndexBufferGLES2::SetupDispatch(&DispatchGLES2);
    QueryBufferGLES2::SetupDispatch(&DispatchGLES2);
    PerfQuerySetGLES2::SetupDispatch(&DispatchGLES2);
    TextureGLES2::SetupDispatch(&DispatchGLES2);
    PipelineStateGLES2::SetupDispatch(&DispatchGLES2);
    ConstBufferGLES2::SetupDispatch(&DispatchGLES2);
    DepthStencilStateGLES2::SetupDispatch(&DispatchGLES2);
    SamplerStateGLES2::SetupDispatch(&DispatchGLES2);
    RenderPassGLES2::SetupDispatch(&DispatchGLES2);
    CommandBufferGLES2::SetupDispatch(&DispatchGLES2);

    DispatchGLES2.impl_Reset = &gles2_Reset;
    DispatchGLES2.impl_Uninitialize = &gles2_Uninitialize;
    DispatchGLES2.impl_HostApi = &gles2_HostApi;
    DispatchGLES2.impl_TextureFormatSupported = &gles2_TextureFormatSupported;
    DispatchGLES2.impl_DeviceCaps = &gles2_DeviceCaps;
    DispatchGLES2.impl_NeedRestoreResources = &gles2_NeedRestoreResources;
    DispatchGLES2.impl_InvalidateCache = &gles2_InvalidateCache;

    SetDispatchTable(DispatchGLES2);

    gles_check_GL_extensions();    
    

#if ENABLE_DEBUG_OUTPUT
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
    glDebugMessageCallback(&_OGLErrorCallback, 0);
#endif

    stat_DIP = StatSet::AddStat("rhi'dip", "dip");
    stat_DP = StatSet::AddStat("rhi'dp", "dp");
    stat_DTL = StatSet::AddStat("rhi'dtl", "dtl");
    stat_DTS = StatSet::AddStat("rhi'dts", "dts");
    stat_DLL = StatSet::AddStat("rhi'dll", "dll");
    stat_SET_PS = StatSet::AddStat("rhi'set-ps", "set-ps");
    stat_SET_SS = StatSet::AddStat("rhi'set-ss", "set-ss");
    stat_SET_TEX = StatSet::AddStat("rhi'set-tex", "set-tex");
    stat_SET_CB = StatSet::AddStat("rhi'set-cb", "set-cb");
    stat_SET_VB = StatSet::AddStat("rhi'set-vb", "set-vb");
    stat_SET_IB = StatSet::AddStat("rhi'set-ib", "set-ib");

    if (param.threadedRenderEnabled)
        _GLES2_ReleaseContext();
}

} // namespace rhi

bool GetGLTextureFormat(rhi::TextureFormat rhiFormat, GLint* internalFormat, GLint* format, GLenum* type, bool* compressed)
{
    using namespace rhi;

    bool success = false;

    switch (rhiFormat)
    {
    case TEXTURE_FORMAT_R8G8B8A8:
        *internalFormat = GL_RGBA;
        *format = GL_RGBA;
        *type = GL_UNSIGNED_BYTE;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_R8G8B8:
        *internalFormat = GL_RGB;
        *format = GL_RGB;
        *type = GL_UNSIGNED_BYTE;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_R4G4B4A4:
        *internalFormat = GL_RGBA;
        *format = GL_RGBA;
        *type = GL_UNSIGNED_SHORT_4_4_4_4;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_R8:
        *internalFormat = GL_ALPHA;
        *format = GL_ALPHA;
        *type = GL_UNSIGNED_BYTE;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_R16:
        *internalFormat = GL_ALPHA;
        *format = GL_ALPHA;
        *type = GL_UNSIGNED_SHORT;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_R5G5B5A1:
        *internalFormat = GL_RGBA;
        *format = GL_RGBA;
        *type = GL_UNSIGNED_SHORT_5_5_5_1;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_R5G6B5:
        *internalFormat = GL_RGB;
        *format = GL_RGB;
        *type = GL_UNSIGNED_SHORT_5_6_5;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_A16R16G16B16:
        *internalFormat = GL_RGBA;
        *format = GL_RGBA;
        *type = GL_UNSIGNED_SHORT;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_A32R32G32B32:
        *internalFormat = GL_RGBA;
        *format = GL_RGBA;
        *type = GL_UNSIGNED_INT;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_DXT1:
        *internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        *format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_DXT3:
        *internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        *format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_DXT5:
        *internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        *format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_PVRTC_4BPP_RGBA:
        *internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
        *format = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_PVRTC_2BPP_RGBA:
        *internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        *format = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA:
        *internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
        *format = GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_PVRTC2_2BPP_RGBA:
        *internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG;
        *format = GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_ETC1:
        *internalFormat = GL_ETC1_RGB8_OES;
        *format = GL_ETC1_RGB8_OES;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_ETC2_R8G8B8:
        *internalFormat = GL_COMPRESSED_RGB8_ETC2;
        *format = GL_COMPRESSED_RGB8_ETC2;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_ETC2_R8G8B8A8:
        *internalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
        *format = GL_COMPRESSED_RGBA8_ETC2_EAC;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_ETC2_R8G8B8A1:
        *internalFormat = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
        *format = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_EAC_R11_UNSIGNED:
        *internalFormat = GL_COMPRESSED_R11_EAC;
        *format = GL_COMPRESSED_R11_EAC;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_EAC_R11_SIGNED:
        *internalFormat = GL_COMPRESSED_SIGNED_R11_EAC;
        *format = GL_COMPRESSED_SIGNED_R11_EAC;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_EAC_R11G11_UNSIGNED:
        *internalFormat = GL_COMPRESSED_RG11_EAC;
        *format = GL_COMPRESSED_RG11_EAC;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_EAC_R11G11_SIGNED:
        *internalFormat = GL_COMPRESSED_SIGNED_RG11_EAC;
        *format = GL_COMPRESSED_SIGNED_RG11_EAC;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_ATC_RGB:
        *internalFormat = GL_ATC_RGB_AMD;
        *format = GL_ATC_RGB_AMD;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_ATC_RGBA_EXPLICIT:
        *internalFormat = GL_ATC_RGBA_EXPLICIT_ALPHA_AMD;
        *format = GL_ATC_RGBA_EXPLICIT_ALPHA_AMD;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_ATC_RGBA_INTERPOLATED:
        *internalFormat = GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD;
        *format = GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD;
        *type = GL_UNSIGNED_BYTE;
        *compressed = true;
        success = true;
        break;

    case TEXTURE_FORMAT_R16F:
        *internalFormat = GL_R16F;
        *format = GL_RED;
        *type = GL_HALF_FLOAT;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_RG16F:
        *internalFormat = GL_RG16F;
        *format = GL_RG;
        *type = GL_HALF_FLOAT;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_RGBA16F:
        *internalFormat = GL_RGBA16F;
        *format = GL_RGBA;
        *type = GL_HALF_FLOAT;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_R32F:
        *internalFormat = GL_R32F;
        *format = GL_RED;
        *type = GL_FLOAT;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_RG32F:
        *internalFormat = GL_RG32F;
        *format = GL_RG;
        *type = GL_FLOAT;
        *compressed = false;
        success = true;
        break;

    case TEXTURE_FORMAT_RGBA32F:
        *internalFormat = GL_RGBA32F;
        *format = GL_RGBA;
        *type = GL_FLOAT;
        *compressed = false;
        success = true;
        break;

    default:
        success = false;
        DVASSERT_MSG(0, "Unsupported or unknown texture format specified");
    }

    return success;
}
