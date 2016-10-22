#include "rhi_Impl.h"
#include "../rhi_Public.h"

    #if defined(__DAVAENGINE_WIN32__)
        #include "../DX9/rhi_DX9.h"
        #include "../DX11/rhi_DX11.h"
        #include "../GLES2/rhi_GLES2.h"
    #elif defined(__DAVAENGINE_WIN_UAP__)
        #include "../DX11/rhi_DX11.h"
    #elif defined(__DAVAENGINE_MACOS__)
        #include "../GLES2/rhi_GLES2.h"
    #elif defined(__DAVAENGINE_IPHONE__)
        #include "../Metal/rhi_Metal.h"
        #include "../GLES2/rhi_GLES2.h"
    #elif defined(__DAVAENGINE_ANDROID__)
        #include "../GLES2/rhi_GLES2.h"
    #else
    #endif

    #include "Core/Core.h"
using DAVA::Logger;
    #include "Concurrency/Spinlock.h"

    #include "MemoryManager/MemoryProfiler.h"

namespace rhi
{
uint32 stat_DIP = DAVA::InvalidIndex;
uint32 stat_DP = DAVA::InvalidIndex;
uint32 stat_DTL = DAVA::InvalidIndex;
uint32 stat_DTS = DAVA::InvalidIndex;
uint32 stat_DLL = DAVA::InvalidIndex;
uint32 stat_SET_PS = DAVA::InvalidIndex;
uint32 stat_SET_SS = DAVA::InvalidIndex;
uint32 stat_SET_TEX = DAVA::InvalidIndex;
uint32 stat_SET_CB = DAVA::InvalidIndex;
uint32 stat_SET_VB = DAVA::InvalidIndex;
uint32 stat_SET_IB = DAVA::InvalidIndex;

static Dispatch _Impl = {};
static RenderDeviceCaps renderDeviceCaps = {};

void SetDispatchTable(const Dispatch& dispatch)
{
    _Impl = dispatch;
}

bool
ApiIsSupported(Api api)
{
    bool supported = false;

    switch (api)
    {
    case RHI_DX9:
    {
            #if defined(__DAVAENGINE_WIN32__)
        supported = true;
            #endif
    }
    break;

    case RHI_DX11:
    {
            #if defined(__DAVAENGINE_WIN32__)
        supported = true;
            #endif
    }
    break;

    case RHI_METAL:
    {
            #if defined(__DAVAENGINE_IPHONE__) && TARGET_IPHONE_SIMULATOR != 1
        supported = rhi_MetalIsSupported();
            #endif
    }
    break;

    case RHI_GLES2:
        supported = true;
        break;
    }

    return supported;
}

void Initialize(Api api, const InitParam& param)
{
    switch (api)
    {
#if defined(__DAVAENGINE_WIN32__)
    case RHI_DX9:
        dx9_Initialize(param);
        break;
#endif

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_WIN_UAP__)
    case RHI_DX11:
        dx11_Initialize(param);
        break;
#endif
            
#if !defined(__DAVAENGINE_WIN_UAP__)
    case RHI_GLES2:
        gles2_Initialize(param);
        break;
#endif

#if defined(__DAVAENGINE_IPHONE__)
#if !(TARGET_IPHONE_SIMULATOR == 1)
    case RHI_METAL:
        metal_Initialize(param);
        break;
#endif //#if !(TARGET_IPHONE_SIMULATOR==1)
#endif

    default:
    {
        // error 'unsupported' here
    }
    }
}

void Reset(const ResetParam& param)
{
    (*_Impl.impl_Reset)(param);
}

bool NeedRestoreResources()
{
    return (*_Impl.impl_NeedRestoreResources)();
}

void Uninitialize()
{
    (*_Impl.impl_Uninitialize)();
}

void PresentImpl(Handle sync)
{
    (*_Impl.impl_Present)(sync);
}

Api HostApi()
{
    return (*_Impl.impl_HostApi)();
}

bool TextureFormatSupported(TextureFormat format, ProgType progType)
{
    return (*_Impl.impl_TextureFormatSupported)(format, progType);
}

const RenderDeviceCaps& DeviceCaps()
{
    return renderDeviceCaps;
}

void SuspendRendering()
{
    (*_Impl.impl_SuspendRendering)();
}

void ResumeRendering()
{
    (*_Impl.impl_ResumeRendering)();
}

void InvalidateCache()
{
    if (_Impl.impl_InvalidateCache)
        (*_Impl.impl_InvalidateCache)();
}

//////////////////////////////////////////////////////////////////////////

namespace VertexBuffer
{
Handle
Create(const Descriptor& desc)
{
#if !defined(DAVA_MEMORY_PROFILING_ENABLE)
    return (*_Impl.impl_VertexBuffer_Create)(desc);
#else
    Handle handle = (*_Impl.impl_VertexBuffer_Create)(desc);
    if (handle != rhi::InvalidHandle)
    {
        DAVA_MEMORY_PROFILER_GPU_ALLOC(handle, desc.size, DAVA::ALLOC_GPU_RDO_VERTEX);
    }
    return handle;
#endif
}

void Delete(Handle vb)
{
    if (vb != rhi::InvalidHandle)
    {
        #if defined(DAVA_MEMORY_PROFILING_ENABLE)
        DAVA_MEMORY_PROFILER_GPU_DEALLOC(vb, DAVA::ALLOC_GPU_RDO_VERTEX);        
        #endif
        (*_Impl.impl_VertexBuffer_Delete)(vb);
    }
}

bool Update(Handle vb, const void* data, uint32 offset, uint32 size)
{
    return (*_Impl.impl_VertexBuffer_Update)(vb, data, offset, size);
}

void* Map(Handle vb, uint32 offset, uint32 size)
{
    DAVA_MEMORY_PROFILER_ALLOC_SCOPE(DAVA::ALLOC_POOL_RHI_VERTEX_MAP);
    return (*_Impl.impl_VertexBuffer_Map)(vb, offset, size);
}

void Unmap(Handle vb)
{
    return (*_Impl.impl_VertexBuffer_Unmap)(vb);
}

bool NeedRestore(Handle vb)
{
    return (*_Impl.impl_VertexBuffer_NeedRestore)(vb);
}

} // namespace VertexBuffer

//////////////////////////////////////////////////////////////////////////

namespace IndexBuffer
{
Handle
Create(const Descriptor& desc)
{
#if !defined(DAVA_MEMORY_PROFILING_ENABLE)
    return (*_Impl.impl_IndexBuffer_Create)(desc);
#else
    Handle handle = (*_Impl.impl_IndexBuffer_Create)(desc);
    if (handle != rhi::InvalidHandle)
    {
        DAVA_MEMORY_PROFILER_GPU_ALLOC(handle, desc.size, DAVA::ALLOC_GPU_RDO_INDEX);
    }
    return handle;
#endif
}

void Delete(Handle ib)
{
    if (ib != InvalidHandle)
    {
        #if defined(DAVA_MEMORY_PROFILING_ENABLE)
        DAVA_MEMORY_PROFILER_GPU_DEALLOC(ib, DAVA::ALLOC_GPU_RDO_INDEX);        
        #endif
        (*_Impl.impl_IndexBuffer_Delete)(ib);
    }
}

bool Update(Handle vb, const void* data, uint32 offset, uint32 size)
{
    return (*_Impl.impl_IndexBuffer_Update)(vb, data, offset, size);
}

void* Map(Handle vb, uint32 offset, uint32 size)
{
    DAVA_MEMORY_PROFILER_ALLOC_SCOPE(DAVA::ALLOC_POOL_RHI_INDEX_MAP);
    return (*_Impl.impl_IndexBuffer_Map)(vb, offset, size);
}

void Unmap(Handle vb)
{
    return (*_Impl.impl_IndexBuffer_Unmap)(vb);
}

bool NeedRestore(Handle ib)
{
    return (*_Impl.impl_IndexBuffer_NeedRestore)(ib);
}

} // namespace IndexBuffer

////////////////////////////////////////////////////////////////////////////////

namespace QueryBuffer
{
Handle
Create(uint32 maxObjectCount)
{
    return (*_Impl.impl_QueryBuffer_Create)(maxObjectCount);
}

void Reset(Handle buf)
{
    (*_Impl.impl_QueryBuffer_Reset)(buf);
}

void Delete(Handle buf)
{
    (*_Impl.impl_QueryBuffer_Delete)(buf);
}

bool BufferIsReady(Handle buf)
{
    return (*_Impl.impl_QueryBuffer_IsReady)(buf);
}

bool IsReady(Handle buf, uint32 objectIndex)
{
    return (*_Impl.impl_QueryBuffer_ObjectIsReady)(buf, objectIndex);
}

int Value(Handle buf, uint32 objectIndex)
{
    return (*_Impl.impl_QueryBuffer_Value)(buf, objectIndex);
}
}

////////////////////////////////////////////////////////////////////////////////
namespace PerfQuerySet
{
Handle Create(uint32 maxTimestampCount)
{
    return (*_Impl.impl_PerfQuerySet_Create)(maxTimestampCount);
}
void Reset(Handle set)
{
    (*_Impl.impl_PerfQuerySet_Reset)(set);
}
void SetCurrent(Handle set)
{
    (*_Impl.impl_PerfQuerySet_SetCurrent)(set);
}
void Delete(Handle set)
{
    (*_Impl.impl_PerfQuerySet_Delete)(set);
}

void GetStatus(Handle set, bool* isReady, bool* isValid)
{
    (*_Impl.impl_PerfQuerySet_GetStatus)(set, isReady, isValid);
}
bool GetFreq(Handle set, uint64* freq)
{
    return (*_Impl.impl_PerfQuerySet_GetFreq)(set, freq);
}
bool GetTimestamp(Handle set, uint32 timestampIndex, uint64* timestamp)
{
    return (*_Impl.impl_PerfQuerySet_GetTimestamp)(set, timestampIndex, timestamp);
}
bool GetFrameTimestamps(Handle set, uint64* t0, uint64* t1)
{
    return (*_Impl.impl_PerfQuerySet_GetFrameTimestamps)(set, t0, t1);
}
}
////////////////////////////////////////////////////////////////////////////////

namespace Texture
{
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
uint32 TextureSizeForProfiling(Handle handle, const Texture::Descriptor& desc)
{
    uint32 size = 0;
    uint32 nfaces = desc.type == TEXTURE_TYPE_CUBE ? 6 : 1;
    for (uint32 curFace = 0; curFace < nfaces; ++curFace)
    {
        for (uint32 curLevel = 0; curLevel < desc.levelCount; ++curLevel)
        {
            if (desc.initialData[curFace * desc.levelCount + curLevel] != nullptr)
            {
                Size2i sz = TextureExtents(Size2i(desc.width, desc.height), curLevel);
                uint32 n = TextureSize(desc.format, sz.dx, sz.dy);
                size += n;
            }
        }
    }
    return size;
}
#endif

Handle
Create(const Texture::Descriptor& desc)
{
#if !defined(DAVA_MEMORY_PROFILING_ENABLE)
    return (*_Impl.impl_Texture_Create)(desc);
#else
    Handle handle = (*_Impl.impl_Texture_Create)(desc);
    if (handle != rhi::InvalidHandle)
    {
        uint32 size = TextureSizeForProfiling(handle, desc);
        DAVA_MEMORY_PROFILER_GPU_ALLOC(handle, size, DAVA::ALLOC_GPU_TEXTURE);
    }
    return handle;
#endif
}

void Delete(Handle tex)
{
    if (tex != InvalidHandle)
    {
        #if defined(DAVA_MEMORY_PROFILING_ENABLE)
        DAVA_MEMORY_PROFILER_GPU_DEALLOC(tex, DAVA::ALLOC_GPU_TEXTURE);    
        #endif
        (*_Impl.impl_Texture_Delete)(tex);
    }
}

void* Map(Handle tex, unsigned level, TextureFace face)
{
    DAVA_MEMORY_PROFILER_ALLOC_SCOPE(DAVA::ALLOC_POOL_RHI_TEXTURE_MAP);
    return (*_Impl.impl_Texture_Map)(tex, level, face);
}

void Unmap(Handle tex)
{
    return (*_Impl.impl_Texture_Unmap)(tex);
}

void Update(Handle tex, const void* data, uint32 level, TextureFace face)
{
    return (*_Impl.impl_Texture_Update)(tex, data, level, face);
}

bool NeedRestore(Handle tex)
{
    return (*_Impl.impl_Texture_NeedRestore)(tex);
}
};

////////////////////////////////////////////////////////////////////////////////

namespace PipelineState
{
Handle
Create(const Descriptor& desc)
{
    return (*_Impl.impl_PipelineState_Create)(desc);
}

void Delete(Handle ps)
{
    return (*_Impl.impl_PipelineState_Delete)(ps);
}

Handle
CreateVertexConstBuffer(Handle ps, uint32 bufIndex)
{
    return (*_Impl.impl_PipelineState_CreateVertexConstBuffer)(ps, bufIndex);
}

Handle
CreateFragmentConstBuffer(Handle ps, uint32 bufIndex)
{
    return (*_Impl.impl_PipelineState_CreateFragmentConstBuffer)(ps, bufIndex);
}

uint32
VertexConstBufferCount(Handle ps)
{
    return (*_Impl.impl_PipelineState_VertexConstBufferCount)(ps);
}

uint32
VertexConstCount(Handle ps, uint32 bufIndex)
{
    return (*_Impl.impl_PipelineState_VertexConstCount)(ps, bufIndex);
}

bool GetVertexConstInfo(Handle ps, uint32 bufIndex, uint32 maxCount, ProgConstInfo* info)
{
    return (*_Impl.impl_PipelineState_GetVertexConstInfo)(ps, bufIndex, maxCount, info);
}

uint32
FragmentConstBufferCount(Handle ps)
{
    return (*_Impl.impl_PipelineState_FragmentConstBufferCount)(ps);
}

uint32
FragmentConstCount(Handle ps, uint32 bufIndex)
{
    return (*_Impl.impl_PipelineState_FragmentConstCount)(ps, bufIndex);
}

bool GetFragmentConstInfo(Handle ps, uint32 bufIndex, uint32 maxCount, ProgConstInfo* info)
{
    return (*_Impl.impl_PipelineState_GetFragmentConstInfo)(ps, bufIndex, maxCount, info);
}

} // namespace PipelineState

//////////////////////////////////////////////////////////////////////////

namespace ConstBuffer
{
uint32
ConstCount(Handle cb)
{
    return (*_Impl.impl_ConstBuffer_ConstCount)(cb);
}

bool SetConst(Handle cb, uint32 constIndex, uint32 constCount, const float* data)
{
    return (*_Impl.impl_ConstBuffer_SetConst)(cb, constIndex, constCount, data);
}

bool SetConst(Handle cb, uint32 constIndex, uint32 constSubIndex, const float* data, uint32 dataCount)
{
    return (*_Impl.impl_ConstBuffer_SetConst1fv)(cb, constIndex, constSubIndex, data, dataCount);
}

void Delete(Handle cb)
{
    if (cb != InvalidHandle)
        (*_Impl.impl_ConstBuffer_Delete)(cb);
}

} // namespace ConstBuffer

//////////////////////////////////////////////////////////////////////////

namespace DepthStencilState
{
Handle
Create(const Descriptor& desc)
{
    return (*_Impl.impl_DepthStencilState_Create)(desc);
}

void Delete(Handle state)
{
    (*_Impl.impl_DepthStencilState_Delete)(state);
}
}

//////////////////////////////////////////////////////////////////////////

namespace SamplerState
{
Handle
Create(const Descriptor& desc)
{
    return (*_Impl.impl_SamplerState_Create)(desc);
}

void Delete(Handle state)
{
    (*_Impl.impl_SamplerState_Delete)(state);
}
}

//////////////////////////////////////////////////////////////////////////

namespace RenderPass
{
Handle
Allocate(const RenderPassConfig& passDesc, uint32 cmdBufCount, Handle* cmdBuf)
{
    return (*_Impl.impl_Renderpass_Allocate)(passDesc, cmdBufCount, cmdBuf);
}

void Begin(Handle pass)
{
    return (*_Impl.impl_Renderpass_Begin)(pass);
}

void End(Handle pass)
{
    return (*_Impl.impl_Renderpass_End)(pass);
}
}

//////////////////////////////////////////////////////////////////////////

namespace SyncObject
{
Handle
Create()
{
    return (*_Impl.impl_SyncObject_Create)();
}

void Delete(Handle obj)
{
    (*_Impl.impl_SyncObject_Delete)(obj);
}

bool IsSygnaled(Handle obj)
{
    return (*_Impl.impl_SyncObject_IsSignaled)(obj);
}
}

//////////////////////////////////////////////////////////////////////////

namespace CommandBuffer
{
void Begin(Handle cmdBuf)
{
    (*_Impl.impl_CommandBuffer_Begin)(cmdBuf);
}

void End(Handle cmdBuf, Handle syncObject)
{
    (*_Impl.impl_CommandBuffer_End)(cmdBuf, syncObject);
}

void SetPipelineState(Handle cmdBuf, Handle ps, uint32 layout)
{
    (*_Impl.impl_CommandBuffer_SetPipelineState)(cmdBuf, ps, layout);
}

void SetCullMode(Handle cmdBuf, CullMode mode)
{
    (*_Impl.impl_CommandBuffer_SetCullMode)(cmdBuf, mode);
}

void SetScissorRect(Handle cmdBuf, ScissorRect rect)
{
    (*_Impl.impl_CommandBuffer_SetScissorRect)(cmdBuf, rect);
}

void SetViewport(Handle cmdBuf, Viewport vp)
{
    (*_Impl.impl_CommandBuffer_SetViewport)(cmdBuf, vp);
}

void SetFillMode(Handle cmdBuf, FillMode mode)
{
    (*_Impl.impl_CommandBuffer_SetFillMode)(cmdBuf, mode);
}

void SetVertexData(Handle cmdBuf, Handle vb, uint32 streamIndex)
{
    (*_Impl.impl_CommandBuffer_SetVertexData)(cmdBuf, vb, streamIndex);
}

void SetVertexConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    (*_Impl.impl_CommandBuffer_SetVertexConstBuffer)(cmdBuf, bufIndex, buffer);
}

void SetVertexTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    (*_Impl.impl_CommandBuffer_SetVertexTexture)(cmdBuf, unitIndex, tex);
}

void SetIndices(Handle cmdBuf, Handle ib)
{
    (*_Impl.impl_CommandBuffer_SetIndices)(cmdBuf, ib);
}

void SetQueryBuffer(Handle cmdBuf, Handle queryBuf)
{
    (*_Impl.impl_CommandBuffer_SetQueryBuffer)(cmdBuf, queryBuf);
}

void SetQueryIndex(Handle cmdBuf, uint32 index)
{
    (*_Impl.impl_CommandBuffer_SetQueryIndex)(cmdBuf, index);
}
void IssueTimestampQuery(Handle cmdBuf, Handle pqset, uint32 timestampIndex)
{
    (*_Impl.impl_CommandBuffer_IssueTimestampQuery)(cmdBuf, pqset, timestampIndex);
}

void SetFragmentConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buf)
{
    (*_Impl.impl_CommandBuffer_SetFragmentConstBuffer)(cmdBuf, bufIndex, buf);
}

void SetFragmentTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    (*_Impl.impl_CommandBuffer_SetFragmentTexture)(cmdBuf, unitIndex, tex);
}

void SetDepthStencilState(Handle cmdBuf, Handle depthStencilState)
{
    (*_Impl.impl_CommandBuffer_SetDepthStencilState)(cmdBuf, depthStencilState);
}

void SetSamplerState(Handle cmdBuf, const Handle samplerState)
{
    (*_Impl.impl_CommandBuffer_SetSamplerState)(cmdBuf, samplerState);
}

void DrawPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count)
{
    (*_Impl.impl_CommandBuffer_DrawPrimitive)(cmdBuf, type, count);
}

void DrawIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 primCount, uint32 vertexCount, uint32 firstVertex, uint32 startIndex)
{
    (*_Impl.impl_CommandBuffer_DrawIndexedPrimitive)(cmdBuf, type, primCount, vertexCount, firstVertex, startIndex);
}

void DrawInstancedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 instCount, uint32 count)
{
    (*_Impl.impl_CommandBuffer_DrawInstancedPrimitive)(cmdBuf, type, instCount, count);
}

void DrawInstancedIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 instCount, uint32 primCount, uint32 vertexCount, uint32 firstVertex, uint32 startIndex, uint32 baseInstance)
{
    (*_Impl.impl_CommandBuffer_DrawInstancedIndexedPrimitive)(cmdBuf, type, instCount, primCount, vertexCount, firstVertex, startIndex, baseInstance);
}

void SetMarker(Handle cmdBuf, const char* text)
{
    (*_Impl.impl_CommandBuffer_SetMarker)(cmdBuf, text);
}

} // namespace CommandBuffer

//------------------------------------------------------------------------------

uint32
TextureStride(TextureFormat format, Size2i size, uint32 level)
{
    uint32 stride = 0;
    uint32 width = TextureExtents(size, level).dx;

    switch (format)
    {
    case TEXTURE_FORMAT_R8G8B8A8:
    {
        stride = width * sizeof(uint32);
    }
    break;

    case TEXTURE_FORMAT_R8G8B8:
    {
        stride = width * 3 * sizeof(uint8);
    }
    break;

    case TEXTURE_FORMAT_R4G4B4A4:
    case TEXTURE_FORMAT_R5G5B5A1:
    case TEXTURE_FORMAT_R5G6B5:
    case TEXTURE_FORMAT_R16:
    case TEXTURE_FORMAT_D16:
    {
        stride = width * sizeof(uint16);
    }
    break;

    case TEXTURE_FORMAT_R8:
    {
        stride = width * sizeof(uint8);
    }
    break;

    case TEXTURE_FORMAT_D24S8:
    {
        stride = width * sizeof(uint32);
    }
    break;

    case TEXTURE_FORMAT_DXT1:
    {
        stride = 8 * std::max(1u, (width + 3) / 4);
    }
    break;

    case TEXTURE_FORMAT_DXT3:
    case TEXTURE_FORMAT_DXT5:
    {
        stride = 16 * std::max(1u, (width + 3) / 4);
    }
    break;

    default:
    {
    }
    }

    return stride;
}

//------------------------------------------------------------------------------

Size2i
TextureExtents(Size2i size, uint32 level)
{
    Size2i sz(size.dx >> level, size.dy >> level);

    if (sz.dx == 0)
        sz.dx = 1;
    if (sz.dy == 0)
        sz.dy = 1;

    return sz;
}

//------------------------------------------------------------------------------

uint32
TextureSize(TextureFormat format, uint32 width, uint32 height, uint32 level)
{
    Size2i ext = TextureExtents(Size2i(width, height), level);
    uint32 sz = 0;

    switch (format)
    {
    case TEXTURE_FORMAT_R8G8B8A8:
    case TEXTURE_FORMAT_R8G8B8X8:
        sz = ext.dx * ext.dy * sizeof(uint32);
        break;

    case TEXTURE_FORMAT_R8G8B8:
        sz = ext.dx * ext.dy * 3 * sizeof(uint8);
        break;

    case TEXTURE_FORMAT_R5G5B5A1:
    case TEXTURE_FORMAT_R5G6B5:
        sz = ext.dx * ext.dy * sizeof(uint16);
        break;

    case TEXTURE_FORMAT_R4G4B4A4:
        sz = ext.dx * ext.dy * sizeof(uint16);
        break;

    case TEXTURE_FORMAT_A16R16G16B16:
        sz = ext.dx * ext.dy * sizeof(uint16);
        break;

    case TEXTURE_FORMAT_A32R32G32B32:
        sz = ext.dx * ext.dy * sizeof(float32);
        break;

    case TEXTURE_FORMAT_R8:
        sz = ext.dx * ext.dy * sizeof(uint8);
        break;

    case TEXTURE_FORMAT_R16:
        sz = ext.dx * ext.dy * sizeof(uint16);
        break;

    case TEXTURE_FORMAT_DXT1:
    {
        int ww = ext.dx >> 2;
        int hh = ext.dy >> 2;

        if (!ww)
            ww = 1;
        if (!hh)
            hh = 1;

        sz = (ww * hh) << 3;
    }
    break;

    case TEXTURE_FORMAT_DXT3:
    case TEXTURE_FORMAT_DXT5:
    {
        int ww = ext.dx >> 2;
        int hh = ext.dy >> 2;

        if (!ww)
            ww = 1;
        if (!hh)
            hh = 1;

        sz = (ww * hh) << 4;
    }
    break;

    case TEXTURE_FORMAT_PVRTC_4BPP_RGBA:
    {
        uint32 block_h = 8;
        uint32 block_w = 8;

        sz = ((height + block_h - 1) / block_h) * ((width + block_w - 1) / block_w) * (sizeof(uint64) * 4);
    }
    break;

    case TEXTURE_FORMAT_PVRTC_2BPP_RGBA:
    {
        uint32 block_h = 16;
        uint32 block_w = 8;

        sz = ((height + block_h - 1) / block_h) * ((width + block_w - 1) / block_w) * (sizeof(uint64) * 4);
    }
    break;

    case TEXTURE_FORMAT_PVRTC2_4BPP_RGB:
    case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA:
    {
        uint32 block_h = 4;
        uint32 block_w = 4;

        sz = ((height + block_h - 1) / block_h) * ((width + block_w - 1) / block_w) * sizeof(uint64);
    }
    break;

    case TEXTURE_FORMAT_PVRTC2_2BPP_RGB:
    case TEXTURE_FORMAT_PVRTC2_2BPP_RGBA:
    {
        uint32 block_h = 4;
        uint32 block_w = 8;

        sz = ((height + block_h - 1) / block_h) * ((width + block_w - 1) / block_w) * sizeof(uint64);
    }
    break;

    case TEXTURE_FORMAT_ATC_RGB:
        sz = ((ext.dx + 3) / 4) * ((ext.dy + 3) / 4) * 8;
        break;

    case TEXTURE_FORMAT_ATC_RGBA_EXPLICIT:
    case TEXTURE_FORMAT_ATC_RGBA_INTERPOLATED:
        sz = ((ext.dx + 3) / 4) * ((ext.dy + 3) / 4) * 16;
        break;

    case TEXTURE_FORMAT_ETC1:
    case TEXTURE_FORMAT_ETC2_R8G8B8:
    {
        int ww = ext.dx >> 2;
        int hh = ext.dy >> 2;

        if (!ww)
            ww = 1;
        if (!hh)
            hh = 1;

        sz = (ww * hh) << 3;
    }
    break;

    case TEXTURE_FORMAT_ETC2_R8G8B8A8:
    case TEXTURE_FORMAT_ETC2_R8G8B8A1:
    {
        int ww = ext.dx >> 2;
        int hh = ext.dy >> 2;

        if (!ww)
            ww = 1;
        if (!hh)
            hh = 1;

        sz = (ww * hh) << 4;
    }
    break;

    case TEXTURE_FORMAT_EAC_R11_UNSIGNED:
    case TEXTURE_FORMAT_EAC_R11_SIGNED:
    {
        int ww = ext.dx >> 2;
        int hh = ext.dy >> 2;

        if (!ww)
            ww = 1;
        if (!hh)
            hh = 1;

        sz = (ww * hh) << 3;
    }
    break;

    case TEXTURE_FORMAT_EAC_R11G11_UNSIGNED:
    case TEXTURE_FORMAT_EAC_R11G11_SIGNED:
    {
        int ww = ext.dx >> 2;
        int hh = ext.dy >> 2;

        if (!ww)
            ww = 1;
        if (!hh)
            hh = 1;

        sz = (ww * hh) << 4;
    }
    break;

    case TEXTURE_FORMAT_D16:
        sz = ext.dx * ext.dy * sizeof(uint16);
        break;

    case TEXTURE_FORMAT_D24S8:
        sz = ext.dx * ext.dy * sizeof(uint32);
        break;

    case TEXTURE_FORMAT_R32F:
        sz = ext.dx * ext.dy * sizeof(float32);
        break;

    case TEXTURE_FORMAT_RG32F:
        sz = ext.dx * ext.dy * sizeof(float32) * 2;
        break;

    case TEXTURE_FORMAT_RGBA32F:
        sz = ext.dx * ext.dy * sizeof(float32) * 4;
        break;

    default:
        break;
    }

    return sz;
}

//------------------------------------------------------------------------------

uint32
NativeColorRGBA(float red, float green, float blue, float alpha)
{
    uint32 color = 0;
    int r = int(red * 255.0f);
    int g = int(green * 255.0f);
    int b = int(blue * 255.0f);
    int a = int(alpha * 255.0f);

    DVASSERT((r >= 0) && (r <= 0xff) && (g >= 0) && (g <= 0xff) && (b >= 0) && (b <= 0xff) && (a >= 0) && (a <= 0xff));

    switch (HostApi())
    {
    case RHI_DX9:
        color = static_cast<uint32>((((a)&0xFF) << 24) | (((r)&0xFF) << 16) | (((g)&0xFF) << 8) | ((b)&0xFF));
        break;

    case RHI_DX11:
        color = static_cast<uint32>((((a)&0xFF) << 24) | (((b)&0xFF) << 16) | (((g)&0xFF) << 8) | ((r)&0xFF));
        //color = ((uint32)((((a)& 0xFF) << 24) | (((r)& 0xFF) << 16) | (((g)& 0xFF) << 8) | ((b)& 0xFF))); for some reason it was here in case of non-uap. seems work ok without it. wait here for someone with "strange" videocard to complain
        break;

    case RHI_GLES2:
        color = static_cast<uint32>((((a)&0xFF) << 24) | (((b)&0xFF) << 16) | (((g)&0xFF) << 8) | ((r)&0xFF));
        break;

    case RHI_METAL:
        color = static_cast<uint32>((((a)&0xFF) << 24) | (((b)&0xFF) << 16) | (((g)&0xFF) << 8) | ((r)&0xFF));
        break;
    }

    return color;
}

namespace MutableDeviceCaps
{
RenderDeviceCaps& Get()
{
    return renderDeviceCaps;
}
}

} //namespace rhi

//------------------------------------------------------------------------------

static DAVA::Spinlock _TraceSync;
static char _TraceBuf[4096];

void Trace(const char* format, ...)
{
#if 0
    _TraceSync.Lock();

    va_list  arglist;

    va_start( arglist, format );
    #if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_WIN_UAP__)
    _vsnprintf( _TraceBuf, countof(_TraceBuf), format, arglist );
    #else
    vsnprintf( _TraceBuf, countof(_TraceBuf), format, arglist );
    #endif
    va_end( arglist );
    
    #if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_WIN_UAP__)
    ::OutputDebugStringA( _TraceBuf );
    #else
    puts( _TraceBuf );
    #endif
    
    _TraceSync.Unlock();
#endif
}
