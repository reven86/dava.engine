/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __RHI_TYPE_H__
#define __RHI_TYPE_H__

#include "Base/BaseTypes.h"
#include "Base/FastName.h"

namespace DAVA
{
class File;
}

namespace rhi
{
using DAVA::uint8;
using DAVA::uint16;
using DAVA::uint32;
using DAVA::uint64;
using DAVA::int32;
using DAVA::float32;
using DAVA::Size2i;

typedef uint32 Handle;
static const uint32 InvalidHandle = 0;
static const uint32 DefaultDepthBuffer = (uint32)(-2);

enum ResourceType
{
    RESOURCE_VERTEX_BUFFER = 11,
    RESOURCE_INDEX_BUFFER = 12,
    RESOURCE_QUERY_BUFFER = 13,
    RESOURCE_CONST_BUFFER = 22,
    RESOURCE_TEXTURE = 31,

    RESOURCE_PIPELINE_STATE = 41,
    RESOURCE_RENDER_PASS = 43,
    RESOURCE_COMMAND_BUFFER = 44,

    RESOURCE_DEPTHSTENCIL_STATE = 51,
    RESOURCE_SAMPLER_STATE = 52,

    RESOURCE_SYNC_OBJECT = 61,

    RESOURCE_PACKET_LIST = 100,
    RESOURCE_TEXTURE_SET = 101
};

enum Api
{
    RHI_DX11,
    RHI_DX9,
    RHI_GLES2,
    RHI_METAL
};

enum ProgType
{
    PROG_VERTEX,
    PROG_FRAGMENT
};

enum PrimitiveType
{
    PRIMITIVE_TRIANGLELIST = 1,
    PRIMITIVE_TRIANGLESTRIP = 2,
    PRIMITIVE_LINELIST = 10
};

enum FillMode
{
    FILLMODE_SOLID = 1,
    FILLMODE_WIREFRAME = 2
};

enum
{
    MAX_CONST_BUFFER_COUNT = 8,
    MAX_RENDER_TARGET_COUNT = 2,
    MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT = 8,
    MAX_VERTEX_TEXTURE_SAMPLER_COUNT = 2,
    MAX_VERTEX_STREAM_COUNT = 4
};

////////////////////////////////////////////////////////////////////////////////
// vertex-pipeline

enum VertexSemantics
{
    VS_POSITION = 1,
    VS_NORMAL = 2,
    VS_COLOR = 3,
    VS_TEXCOORD = 4,
    VS_TANGENT = 5,
    VS_BINORMAL = 6,
    VS_BLENDWEIGHT = 7,
    VS_BLENDINDEX = 8,

    VS_PAD = 100,

    VS_MAXCOUNT = 16
};

//------------------------------------------------------------------------------

inline const char*
VertexSemanticsName(VertexSemantics vs)
{
    switch (vs)
    {
    case VS_POSITION:
        return "position";
    case VS_NORMAL:
        return "normal";
    case VS_COLOR:
        return "color";
    case VS_TEXCOORD:
        return "texcoord";
    case VS_TANGENT:
        return "tangent";
    case VS_BINORMAL:
        return "binormal";
    case VS_BLENDWEIGHT:
        return "blend_weight";
    case VS_BLENDINDEX:
        return "blend_index";

    case VS_PAD:
        return "<pad>";
    case VS_MAXCOUNT:
        return "<max-count>";
    }

    return "<unknown>";
}

//------------------------------------------------------------------------------

enum VertexDataType
{
    VDT_FLOAT = 1,
    VDT_UINT8 = 2,
    VDT_INT16N = 3,
    VDT_INT8N = 4,
    VDT_UINT8N = 5,
    VDT_HALF = 6
};

//------------------------------------------------------------------------------

inline const char*
VertexDataTypeName(VertexDataType t)
{
    switch (t)
    {
    case VDT_FLOAT:
        return "float";
    case VDT_UINT8:
        return "uint8";
    case VDT_INT16N:
        return "int16n";
    case VDT_INT8N:
        return "int8n";
    case VDT_UINT8N:
        return "uint8n";
    case VDT_HALF:
        return "half";
    default:
        return "<unknown>";
    }
}

class
VertexLayout
{
public:
    VertexLayout();
    ~VertexLayout();

    unsigned Stride() const;
    unsigned ElementCount() const;
    VertexSemantics ElementSemantics(unsigned elem_i) const;
    unsigned ElementSemanticsIndex(unsigned elem_i) const;
    VertexDataType ElementDataType(unsigned elem_i) const;
    unsigned ElementDataCount(unsigned elem_i) const;
    unsigned ElementOffset(unsigned elem_i) const;
    unsigned ElementSize(unsigned elem_i) const;

    bool operator==(const VertexLayout& vl) const;
    VertexLayout& operator=(const VertexLayout& src);

    void Clear();
    void AddElement(VertexSemantics usage, unsigned usage_i, VertexDataType type, unsigned dimension);
    void InsertElement(unsigned pos, VertexSemantics usage, unsigned usage_i, VertexDataType type, unsigned dimension);

    static bool IsCompatible(const VertexLayout& vbLayout, const VertexLayout& shaderLayout);
    static bool MakeCompatible(const VertexLayout& vbLayout, const VertexLayout& shaderLayout, VertexLayout* compatibleLayout);

    void Save(DAVA::File* out) const;
    void Load(DAVA::File* in);

    void Dump() const;

    static const VertexLayout* Get(uint32 uid);
    static uint32 UniqueId(const VertexLayout& layout);
    static const uint32 InvalidUID = 0;

private:
    enum
    {
        MaxElemCount = 8
    };

    struct
    Element
    {
        uint32 usage : 8;
        uint32 usage_index : 8;
        uint32 data_type : 8;
        uint32 data_count : 8;
    };

    Element _elem[MaxElemCount];
    uint32 _elem_count;
};

enum
{
    VATTR_POSITION = 0,
    VATTR_NORMAL = 1,
    VATTR_TEXCOORD_0 = 2,
    VATTR_TEXCOORD_1 = 4,
    VATTR_TEXCOORD_2 = 5,
    VATTR_TEXCOORD_3 = 6,
    VATTR_TEXCOORD_4 = 8,
    VATTR_TEXCOORD_5 = 9,
    VATTR_TEXCOORD_6 = 10,
    VATTR_TEXCOORD_7 = 11,
    VATTR_COLOR_0 = 3,
    VATTR_COLOR_1 = 7,
    VATTR_TANGENT = 12,
    VATTR_BINORMAL = 13,
    VATTR_BLENDWEIGHT = 14,
    VATTR_BLENDINDEX = 15,

    VATTR_COUNT = 16
};

////////////////////////////////////////////////////////////////////////////////
// buffer

enum Usage
{
    USAGE_DEFAULT,
    USAGE_STATICDRAW,
    USAGE_DYNAMICDRAW
};

enum Pool
{
    POOL_DEFAULT,
    POOL_LOCALMEMORY,
    POOL_SYSTEMMEMORY
};

////////////////////////////////////////////////////////////////////////////////
// texture

enum TextureType
{
    TEXTURE_TYPE_1D,
    TEXTURE_TYPE_2D,
    TEXTURE_TYPE_CUBE
};

enum TextureFormat
{
    TEXTURE_FORMAT_R8G8B8A8 = 0,
    TEXTURE_FORMAT_R8G8B8X8,

    TEXTURE_FORMAT_R8G8B8,

    TEXTURE_FORMAT_R5G5B5A1,
    TEXTURE_FORMAT_R5G6B5,

    TEXTURE_FORMAT_R4G4B4A4,

    TEXTURE_FORMAT_A16R16G16B16,
    TEXTURE_FORMAT_A32R32G32B32,

    TEXTURE_FORMAT_R8,
    TEXTURE_FORMAT_R16,

    TEXTURE_FORMAT_DXT1,
    TEXTURE_FORMAT_DXT3,
    TEXTURE_FORMAT_DXT5,

    TEXTURE_FORMAT_PVRTC_4BPP_RGBA,
    TEXTURE_FORMAT_PVRTC_2BPP_RGBA,

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
};

enum TextureFace
{
    TEXTURE_FACE_POSITIVE_X,
    TEXTURE_FACE_NEGATIVE_X,
    TEXTURE_FACE_POSITIVE_Y,
    TEXTURE_FACE_NEGATIVE_Y,
    TEXTURE_FACE_POSITIVE_Z,
    TEXTURE_FACE_NEGATIVE_Z
};

enum TextureAddrMode
{
    TEXADDR_WRAP,
    TEXADDR_CLAMP,
    TEXADDR_MIRROR
};

enum TextureFilter
{
    TEXFILTER_NEAREST,
    TEXFILTER_LINEAR
};

enum TextureMipFilter
{
    TEXMIPFILTER_NONE,
    TEXMIPFILTER_NEAREST,
    TEXMIPFILTER_LINEAR
};

enum StencilOperation
{
    STENCILOP_KEEP,
    STENCILOP_ZERO,
    STENCILOP_REPLACE,
    STENCILOP_INVERT,
    STENCILOP_INCREMENT_CLAMP,
    STENCILOP_DECREMENT_CLAMP,
    STENCILOP_INCREMENT_WRAP,
    STENCILOP_DECREMENT_WRAP
};

enum LoadAction
{
    LOADACTION_NONE = 0,
    LOADACTION_CLEAR = 1,
    LOADACTION_LOAD = 2
};

enum StoreAction
{
    STOREACTION_NONE = 0,
    STOREACTION_STORE = 1
    //    STOREACTION_RESOLVE    = 2
};

namespace VertexBuffer
{
struct
Descriptor
{
    uint32 size;
    Pool pool;
    Usage usage;
    const void* initialData;
    uint32 needRestore : 1;

    Descriptor(uint32 sz = 0)
        : size(sz)
        , pool(POOL_DEFAULT)
        , usage(USAGE_DEFAULT)
        , initialData(nullptr)
        , needRestore(true)
    {
    }
};
}

enum IndexSize
{
    INDEX_SIZE_16BIT = 0,
    INDEX_SIZE_32BIT = 1
};

namespace IndexBuffer
{
struct
Descriptor
{
    uint32 size;
    IndexSize indexSize;
    Pool pool;
    Usage usage;
    const void* initialData;
    uint32 needRestore : 1;

    Descriptor(uint32 sz = 0)
        : size(sz)
        , indexSize(INDEX_SIZE_16BIT)
        , pool(POOL_DEFAULT)
        , usage(USAGE_DEFAULT)
        , initialData(nullptr)
        , needRestore(true)
    {
    }
};
}

namespace Texture
{
struct
Descriptor
{
    TextureType type;
    uint32 width;
    uint32 height;
    TextureFormat format;
    uint32 levelCount;
    void* initialData[128]; // it must be writable!
    uint32 isRenderTarget : 1;
    uint32 autoGenMipmaps : 1;
    uint32 needRestore : 1;

    Descriptor(uint32 w, uint32 h, TextureFormat fmt)
        : type(TEXTURE_TYPE_2D)
        , width(w)
        , height(h)
        , format(fmt)
        , levelCount(1)
        , isRenderTarget(false)
        , autoGenMipmaps(false)
        , needRestore(true)
    {
        memset(initialData, 0, sizeof(initialData));
    }
    Descriptor()
        : type(TEXTURE_TYPE_2D)
        , width(0)
        , height(0)
        , format(TEXTURE_FORMAT_R8G8B8A8)
        , levelCount(1)
        , isRenderTarget(false)
        , autoGenMipmaps(false)
        , needRestore(true)
    {
        memset(initialData, 0, sizeof(initialData));
    }
};
}

////////////////////////////////////////////////////////////////////////////////
// pipeline-state

enum ColorMask
{
    COLORMASK_NONE = 0,
    COLORMASK_R = (0x1 << 0),
    COLORMASK_G = (0x1 << 1),
    COLORMASK_B = (0x1 << 2),
    COLORMASK_A = (0x1 << 3),
    COLORMASK_ALL = COLORMASK_R | COLORMASK_G | COLORMASK_B | COLORMASK_A
};

struct
BlendState
{
    struct
    {
        uint32 colorFunc : 2;
        uint32 colorSrc : 3;
        uint32 colorDst : 3;
        uint32 alphaFunc : 2;
        uint32 alphaSrc : 3;
        uint32 alphaDst : 3;
        uint32 writeMask : 4;
        uint32 blendEnabled : 1;
        uint32 alphaToCoverage : 1;
    } rtBlend[MAX_RENDER_TARGET_COUNT];

    BlendState()
    {
        for (unsigned i = 0; i != MAX_RENDER_TARGET_COUNT; ++i)
        {
            rtBlend[i].writeMask = COLORMASK_ALL;
            rtBlend[i].blendEnabled = false;
            rtBlend[i].alphaToCoverage = false;
        }
    }
};

enum BlendFunc
{
};

enum BlendOp
{
    BLENDOP_ZERO,
    BLENDOP_ONE,
    BLENDOP_SRC_ALPHA,
    BLENDOP_INV_SRC_ALPHA,
    BLENDOP_SRC_COLOR,
    BLENDOP_DST_COLOR
};

namespace PipelineState
{
struct
Descriptor
{
    VertexLayout vertexLayout;
    DAVA::FastName vprogUid;
    DAVA::FastName fprogUid;
    BlendState blending;
};
}

namespace SamplerState
{
struct
Descriptor
{
    struct
    Sampler
    {
        uint32 addrU : 2;
        uint32 addrV : 2;
        uint32 addrW : 2;
        uint32 minFilter : 2;
        uint32 magFilter : 2;
        uint32 mipFilter : 2;
        uint32 pad : 20;

        Sampler()
            : addrU(TEXADDR_WRAP)
            , addrV(TEXADDR_WRAP)
            , addrW(TEXADDR_WRAP)
            , minFilter(TEXFILTER_LINEAR)
            , magFilter(TEXFILTER_LINEAR)
            , mipFilter(TEXMIPFILTER_LINEAR)
            , pad(0)
        {
        }
    };

    Sampler fragmentSampler[MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT];
    uint32 fragmentSamplerCount;

    Sampler vertexSampler[MAX_VERTEX_TEXTURE_SAMPLER_COUNT];
    uint32 vertexSamplerCount;

    Descriptor()
        : fragmentSamplerCount(0)
        , vertexSamplerCount(0)
    {
        for (uint32 s = 0; s != MAX_VERTEX_TEXTURE_SAMPLER_COUNT; ++s)
        {
            vertexSampler[s].minFilter = TEXFILTER_NEAREST;
            vertexSampler[s].magFilter = TEXFILTER_NEAREST;
            vertexSampler[s].mipFilter = TEXMIPFILTER_NONE;
        }
    }
};
}

////////////////////////////////////////////////////////////////////////////////
// depth-stencil state

enum CmpFunc
{
    CMP_NEVER,
    CMP_LESS,
    CMP_EQUAL,
    CMP_LESSEQUAL,
    CMP_GREATER,
    CMP_NOTEQUAL,
    CMP_GREATEREQUAL,
    CMP_ALWAYS
};

namespace DepthStencilState
{
struct Descriptor
{
    uint32 depthTestEnabled : 1;
    uint32 depthWriteEnabled : 1;
    uint32 depthFunc : 3;

    uint32 stencilEnabled : 1;
    uint32 stencilTwoSided : 1;
    uint32 pad : 25;

    struct
    {
        uint8 readMask;
        uint8 writeMask;
        uint8 refValue;
        uint8 pad8;
        uint32 func : 3;
        uint32 failOperation : 3;
        uint32 depthFailOperation : 3;
        uint32 depthStencilPassOperation : 3;
        uint32 pad32 : 20;
    } stencilFront, stencilBack;

    Descriptor()
        : depthTestEnabled(true)
        , depthWriteEnabled(true)
        , depthFunc(CMP_LESSEQUAL)
        , stencilEnabled(false)
        , stencilTwoSided(false)
        , pad(0)
    {
        stencilFront.readMask = 0xFF;
        stencilFront.writeMask = 0xFF;
        stencilFront.refValue = 0;
        stencilFront.func = CMP_ALWAYS;
        stencilFront.failOperation = STENCILOP_KEEP;
        stencilFront.depthFailOperation = STENCILOP_KEEP;
        stencilFront.depthStencilPassOperation = STENCILOP_KEEP;
        stencilFront.pad8 = 0;
        stencilFront.pad32 = 0;

        stencilBack.readMask = 0xFF;
        stencilBack.writeMask = 0xFF;
        stencilBack.refValue = 0;
        stencilBack.func = CMP_ALWAYS;
        stencilBack.failOperation = STENCILOP_KEEP;
        stencilBack.depthFailOperation = STENCILOP_KEEP;
        stencilBack.depthStencilPassOperation = STENCILOP_KEEP;
        stencilBack.pad8 = 0;
        stencilBack.pad32 = 0;
    }
};
}

struct
ProgConstInfo
{
    DAVA::FastName uid; // name
    uint32 bufferIndex;
    uint32 offset; // from start of buffer
    int type; // size deduced from type -- float4 = 4*sizeof(float) etc.
};

////////////////////////////////////////////////////////////////////////////////
// cull-mode

enum CullMode
{
    CULL_NONE = 0,
    CULL_CCW = 1,
    CULL_CW = 2
};

////////////////////////////////////////////////////////////////////////////////
// viewport

struct
Viewport
{
    uint32 x = 0;
    uint32 y = 0;
    uint32 width = 0;
    uint32 height = 0;

    Viewport() = default;

    Viewport(uint32 x_, uint32 y_, uint32 w_, uint32 h_)
        : x(x_)
        , y(y_)
        , width(w_)
        , height(h_)
    {
    }
};

////////////////////////////////////////////////////////////////////////////////
// render-target state

struct
RenderPassConfig
{
    struct
    ColorBuffer
    {
        Handle texture;
        LoadAction loadAction;
        StoreAction storeAction;
        float clearColor[4];

        ColorBuffer()
            : texture(InvalidHandle)
            , loadAction(LOADACTION_CLEAR)
            , storeAction(STOREACTION_NONE)
        {
            clearColor[0] = 0;
            clearColor[1] = 0;
            clearColor[2] = 0;
            clearColor[3] = 1.0f;
        }
    };

    struct
    DepthStencilBuffer
    {
        Handle texture;
        LoadAction loadAction;
        StoreAction storeAction;
        float clearDepth;
        uint32 clearStencil;

        DepthStencilBuffer()
            : texture(DefaultDepthBuffer)
            , loadAction(LOADACTION_CLEAR)
            , storeAction(STOREACTION_NONE)
            , clearDepth(1.0f)
            , clearStencil(0)
        {
        }
    };

    ColorBuffer colorBuffer[MAX_RENDER_TARGET_COUNT];
    DepthStencilBuffer depthStencilBuffer;

    Handle queryBuffer;
    Viewport viewport;

    int priority;
    uint32 invertCulling : 1;

    RenderPassConfig()
        : queryBuffer(InvalidHandle)
        , priority(0)
        , invertCulling(0)
    {
    }
};

////////////////////////////////////////////////////////////////////////////////
// query-buffer

namespace QueryBuffer
{
struct
Descriptor
{
};
}

////////////////////////////////////////////////////////////////////////////////
// sync-object

namespace SyncObject
{
struct
Descriptor
{
};
}

////////////////////////////////////////////////////////////////////////////////
// command-buffer

namespace CommandBuffer
{
struct
Descriptor
{
};
}

//------------------------------------------------------------------------------

struct
ScissorRect
{
    uint16 x;
    uint16 y;
    uint16 width;
    uint16 height;

    ScissorRect()
        : x(0)
        , y(0)
        , width(0)
        , height(0)
    {
    }
};

//------------------------------------------------------------------------------

uint32 NativeColorRGBA(float r, float g, float b, float a = 1.0f);

} // namespace rhi

//------------------------------------------------------------------------------
//
//

template <typename src, typename dst>
inline dst
nonaliased_cast(src x)
{
    // (quite a funny way to) ensure types are the same size
    // commented-out until acceptable way to stop the compiler-spamming is found
    //    #pragma warning( disable: 177 ) // variable "WrongSizes" was declared but never referenced
    //    static int  WrongSizes[sizeof(src) == sizeof(dst) ? 1 : -1];
    //    #pragma warning( default: 177 )

    union {
        src s;
        dst d;
    } tmp;

    tmp.s = x;

    return tmp.d;
}

#define countof(array) (sizeof(array) / sizeof(array[0]))
#define L_ALIGNED_SIZE(size, align) (((size) + ((align)-1)) & (~((align)-1)))

inline bool
IsEmptyString(const char* str)
{
    return !(str && str[0] != '\0');
}

void Trace(const char* format, ...);
#define LCP Trace("%s : %i\n", __FILE__, __LINE__);




#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)

#define stricmp strcasecmp
#define strnicmp strncasecmp

#endif

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_WIN_UAP__)

    #define stricmp _strcmpi

#endif



#endif // __RHI_TYPE_H__
