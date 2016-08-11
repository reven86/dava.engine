#include "../Common/rhi_Private.h"
#include "../Common/rhi_Pool.h"
#include "rhi_Metal.h"

#include "../rhi_Type.h"
#include "../Common/dbg_StatSet.h"

#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
using DAVA::Logger;
#include "Core/Core.h"
#include "Debug/Profiler.h"

#include "_metal.h"

#if 0
    #define MTL_TRACE(...) DAVA::Logger::Info(__VA_ARGS__)
    #define MTL_CP DAVA::Logger::Info("%s : %i", __FILE__, __LINE__);
#else
    #define MTL_TRACE(...)
    #define MTL_CP 
#endif


#if !(TARGET_IPHONE_SIMULATOR == 1)
namespace rhi
{

#if !RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
enum CommandMetalType
{
    MTL__BEGIN,
    MTL__END,

    MTL__SET_VERTEX_DATA,
    MTL__SET_INDICES,
    MTL__SET_QUERY_BUFFER,
    MTL__SET_QUERY_INDEX,
    MTL__ISSUE_TIMESTAMP_QUERY,

    MTL__SET_PIPELINE_STATE,
    MTL__SET_CULL_MODE,
    MTL__SET_SCISSOR_RECT,
    MTL__SET_VIEWPORT,
    MTL__SET_FILLMODE,
    MTL__SET_VERTEX_PROG_CONST_BUFFER,
    MTL__SET_FRAGMENT_PROG_CONST_BUFFER,
    MTL__SET_FRAGMENT_TEXTURE,
    MTL__SET_VERTEX_TEXTURE,

    MTL__SET_DEPTHSTENCIL_STATE,
    MTL__SET_SAMPLER_STATE,

    MTL__DRAW_PRIMITIVE,
    MTL__DRAW_INDEXED_PRIMITIVE,
    MTL__DRAW_INSTANCED_PRIMITIVE,
    MTL__DRAW_INSTANCED_INDEXED_PRIMITIVE,

    MTL__DEBUG_MARKER,

    MTL__NOP
};

struct
CommandMTL
{
    uint8 type;
    uint8 size;

    CommandMTL(uint8 t, uint8 sz)
        : type(t)
        , size(sz)
    {
    }
};

template <class T, CommandMetalType t>
struct
CommandMetalImpl
: public CommandMTL
{
    CommandMetalImpl()
        : CommandMTL(t, sizeof(T))
    {
    }
};

struct
CommandMTL_Begin : public CommandMetalImpl<CommandMTL_Begin, MTL__BEGIN>
{
};

struct
CommandMTL_End : public CommandMetalImpl<CommandMTL_End, MTL__END>
{
    Handle syncObject;
    bool doCommit;
};

struct
CommandMTL_SetVertexData : public CommandMetalImpl<CommandMTL_SetVertexData, MTL__SET_VERTEX_DATA>
{
    uint16 streamIndex;
    Handle vb;
};

struct
CommandMTL_SetIndices : public CommandMetalImpl<CommandMTL_SetIndices, MTL__SET_INDICES>
{
    Handle ib;
};

struct
CommandMTL_SetQueryBuffer : public CommandMetalImpl<CommandMTL_SetQueryBuffer, MTL__SET_QUERY_BUFFER>
{
    Handle queryBuf;
};

struct
CommandMTL_SetQueryIndex : public CommandMetalImpl<CommandMTL_SetQueryIndex, MTL__SET_QUERY_INDEX>
{
    uint32 objectIndex;
};

struct
CommandMTL_SetPipelineState : public CommandMetalImpl<CommandMTL_SetPipelineState, MTL__SET_PIPELINE_STATE>
{
    Handle ps;
    uint32 vdeclUID;
};

struct
CommandMTL_SetDepthStencilState : public CommandMetalImpl<CommandMTL_SetDepthStencilState, MTL__SET_DEPTHSTENCIL_STATE>
{
    Handle depthStencilState;
};

struct
CommandMTL_SetSamplerState : public CommandMetalImpl<CommandMTL_SetSamplerState, MTL__SET_SAMPLER_STATE>
{
    Handle samplerState;
};

struct
CommandMTL_SetCullMode : public CommandMetalImpl<CommandMTL_SetCullMode, MTL__SET_CULL_MODE>
{
    uint8 mode;
};

struct
CommandMTL_SetScissorRect : public CommandMetalImpl<CommandMTL_SetScissorRect, MTL__SET_SCISSOR_RECT>
{
    uint16 x, y, w, h;
};

struct
CommandMTL_SetViewport : public CommandMetalImpl<CommandMTL_SetViewport, MTL__SET_VIEWPORT>
{
    uint16 x, y, w, h;
};

struct
CommandMTL_SetFillMode : public CommandMetalImpl<CommandMTL_SetFillMode, MTL__SET_FILLMODE>
{
    uint8 mode;
};

struct
CommandMTL_SetVertexProgConstBuffer : public CommandMetalImpl<CommandMTL_SetVertexProgConstBuffer, MTL__SET_VERTEX_PROG_CONST_BUFFER>
{
    uint16 bufIndex;
    Handle buffer;
    uint32 inst_offset;
};

struct
CommandMTL_SetFragmentProgConstBuffer : public CommandMetalImpl<CommandMTL_SetFragmentProgConstBuffer, MTL__SET_FRAGMENT_PROG_CONST_BUFFER>
{
    uint16 bufIndex;
    Handle buffer;
    uint32 inst_offset;
};

struct
CommandMTL_SetFragmentTexture : public CommandMetalImpl<CommandMTL_SetFragmentTexture, MTL__SET_FRAGMENT_TEXTURE>
{
    uint16 unitIndex;
    Handle tex;
};

struct
CommandMTL_SetVertexTexture : public CommandMetalImpl<CommandMTL_SetVertexTexture, MTL__SET_VERTEX_TEXTURE>
{
    uint16 unitIndex;
    Handle tex;
};

struct
CommandMTL_DrawPrimitive : public CommandMetalImpl<CommandMTL_DrawPrimitive, MTL__DRAW_PRIMITIVE>
{
    uint8 type;
    uint32 vertexCount;
    uint32 baseVertex;
};

struct
CommandMTL_DrawIndexedPrimitive : public CommandMetalImpl<CommandMTL_DrawIndexedPrimitive, MTL__DRAW_INDEXED_PRIMITIVE>
{
    uint8 type;
    uint32 indexCount;
    uint32 vertexCount;
    uint32 baseVertex;
    uint32 startIndex;
};

struct
CommandMTL_DrawInstancedPrimitive : public CommandMetalImpl<CommandMTL_DrawInstancedPrimitive, MTL__DRAW_INSTANCED_PRIMITIVE>
{
    uint8 type;
    uint32 vertexCount;
    uint32 baseVertex;
    uint32 instCount;
};

struct
CommandMTL_DrawInstancedIndexedPrimitive : public CommandMetalImpl<CommandMTL_DrawInstancedIndexedPrimitive, MTL__DRAW_INSTANCED_INDEXED_PRIMITIVE>
{
    uint8 type;
    uint32 indexCount;
    uint32 vertexCount;
    uint32 baseVertex;
    uint32 startIndex;
    uint32 instCount;
    uint32 baseInst;
};

struct
CommandMTL_SetMarker : public CommandMetalImpl<CommandMTL_SetMarker, MTL__DEBUG_MARKER>
{
};

struct
CommandMTL_IssueTimestamptQuery : public CommandMetalImpl<CommandMTL_IssueTimestamptQuery, MTL__SET_QUERY_BUFFER>
{
    Handle querySet;
    uint32 timestampIndex;
};



#endif

struct
RenderPassMetal_t
{
    RenderPassConfig cfg;
    MTLRenderPassDescriptor* desc;
    id<MTLCommandBuffer> buf;
    id<MTLParallelRenderCommandEncoder> encoder;
    std::vector<Handle> cmdBuf;
    int priority;
    uint32 do_present : 1;
    uint32 finished : 1;

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
#else
    bool Initialize();
#endif
};

struct
CommandBufferMetal_t
{
    id<MTLRenderCommandEncoder> encoder;
    id<MTLCommandBuffer> buf;

    id<MTLTexture> rt;
    MTLPixelFormat color_fmt;
    Handle cur_ib;
    unsigned cur_vstream_count;
    Handle cur_vb[MAX_VERTEX_STREAM_COUNT];
    uint32 cur_stride;
    bool ds_used;
#if RHI_METAL__COMMIT_COMMAND_BUFFER_ON_END
    bool do_commit_on_end;
#endif

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
#else
    template <class T>
    T* allocCmd()
    {
        if (curUsedSize + sizeof(T) >= cmdDataSize)
        {
            cmdDataSize += 4 * 1024; // CRAP: hardcoded grow-size
            cmdData = (uint8*)::realloc(cmdData, cmdDataSize);
        }

        uint8* p = cmdData + curUsedSize;
        curUsedSize += sizeof(T);
        return new ((T*)p) T();
    }
    uint8* cmdData;
    uint32 cmdDataSize;
    uint32 curUsedSize;
#endif

    CommandBufferMetal_t();
    void _ApplyVertexData(unsigned firstVertex = 0);
    #if !RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    void Execute();
    #endif
};

struct
FrameMetal_t
{
    std::vector<Handle> pass;
    id<CAMetalDrawable> drawable;
};

struct
SyncObjectMetal_t
{
    uint32 is_signaled : 1;
};

typedef ResourcePool<CommandBufferMetal_t, RESOURCE_COMMAND_BUFFER, CommandBuffer::Descriptor, false> CommandBufferPool;
typedef ResourcePool<RenderPassMetal_t, RESOURCE_RENDER_PASS, RenderPassConfig, false> RenderPassPool;
typedef ResourcePool<SyncObjectMetal_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false> SyncObjectPool;

RHI_IMPL_POOL(CommandBufferMetal_t, RESOURCE_COMMAND_BUFFER, CommandBuffer::Descriptor, false);
RHI_IMPL_POOL(RenderPassMetal_t, RESOURCE_RENDER_PASS, RenderPassConfig, false);
RHI_IMPL_POOL(SyncObjectMetal_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false);

static bool _Metal_NextDrawablePending = false;
static bool _Metal_PresentDrawablePending = false;

static std::vector<FrameMetal_t> _Metal_Frame;
static bool _Metal_NewFramePending = true;

CommandBufferMetal_t::CommandBufferMetal_t()
#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
#else
    : cmdData(nullptr)
    , cmdDataSize(0)
    , curUsedSize(0)
#endif
{
}

void CommandBufferMetal_t::_ApplyVertexData(unsigned firstVertex)
{
    for (unsigned s = 0; s != cur_vstream_count; ++s)
    {
        unsigned base = 0;
        id<MTLBuffer> vb = VertexBufferMetal::GetBuffer(cur_vb[s], &base);
        unsigned off = (s == 0) ? firstVertex * cur_stride : 0;

        [encoder setVertexBuffer:vb offset:base + off atIndex:s];
    }
}

#if !RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
void
CommandBufferMetal_t::Execute()
{
    for (const uint8 *c = cmdData, *c_end = cmdData + curUsedSize; c != c_end;)
    {
        const CommandMTL* cmd = (const CommandMTL*)c;

        switch (CommandMetalType(cmd->type))
        {
        case MTL__BEGIN:
        {
            cur_vstream_count = 0;
            for (unsigned s = 0; s != countof(cur_vb); ++s)
                cur_vb[s] = InvalidHandle;

            [encoder setDepthStencilState:_Metal_DefDepthState];
        }
        break;

        case MTL__END:
        {
            [encoder endEncoding];

            Handle syncObject = ((CommandMTL_End*)cmd)->syncObject;

            if (syncObject != InvalidHandle)
            {
                [buf addCompletedHandler:^(id<MTLCommandBuffer> cmdb) {
                  SyncObjectMetal_t* sync = SyncObjectPool::Get(syncObject);

                  sync->is_signaled = true;
                }];
            }

            #if RHI_METAL__COMMIT_COMMAND_BUFFER_ON_END
            if (((CommandMTL_End*)cmd)->doCommit)
            {
                [buf commit];
            }
            #endif
        }
        break;

        case MTL__SET_PIPELINE_STATE:
        {
            Handle ps = ((CommandMTL_SetPipelineState*)cmd)->ps;
            unsigned layoutUID = ((CommandMTL_SetPipelineState*)cmd)->vdeclUID;

            cur_stride = PipelineStateMetal::SetToRHI(ps, layoutUID, color_fmt, ds_used, encoder);
            cur_vstream_count = PipelineStateMetal::VertexStreamCount(ps);
            StatSet::IncStat(stat_SET_PS, 1);
        }
        break;

        case MTL__SET_CULL_MODE:
        {
            switch (CullMode(((CommandMTL_SetCullMode*)cmd)->mode))
            {
            case CULL_NONE:
                [encoder setCullMode:MTLCullModeNone];
                break;

            case CULL_CCW:
                [encoder setFrontFacingWinding:MTLWindingClockwise];
                [encoder setCullMode:MTLCullModeBack];
                break;

            case CULL_CW:
                [encoder setFrontFacingWinding:MTLWindingClockwise];
                [encoder setCullMode:MTLCullModeFront];
                break;
            }
        }
        break;

        case MTL__SET_SCISSOR_RECT:
        {
            int x = ((CommandMTL_SetScissorRect*)cmd)->x;
            int y = ((CommandMTL_SetScissorRect*)cmd)->y;
            int w = ((CommandMTL_SetScissorRect*)cmd)->w;
            int h = ((CommandMTL_SetScissorRect*)cmd)->h;
            MTLScissorRect rc;

            if (!(x == 0 && y == 0 && w == 0 && h == 0))
            {
                unsigned max_x = (rt) ? unsigned(rt.width) : unsigned(_Metal_DefFrameBuf.width);
                unsigned max_y = (rt) ? unsigned(rt.height) : unsigned(_Metal_DefFrameBuf.height);

                rc.x = x;
                rc.y = y;
                rc.width = (x + w > max_x) ? (max_x - rc.x) : w;
                rc.height = (y + h > max_y) ? (max_y - rc.y) : h;

                if (rc.width == 0)
                {
                    rc.width = 1;
                    if (rc.x > 0)
                        --rc.x;
                }

                if (rc.height == 0)
                {
                    rc.height = 1;
                    if (rc.y > 0)
                        --rc.y;
                }
            }
            else
            {
                rc.x = 0;
                rc.y = 0;
                if (rt)
                {
                    rc.width = rt.width;
                    rc.height = rt.height;
                }
                else
                {
                    rc.width = _Metal_DefFrameBuf.width;
                    rc.height = _Metal_DefFrameBuf.height;
                }
            }

            [encoder setScissorRect:rc];
        }
        break;

        case MTL__SET_VIEWPORT:
        {
            MTLViewport vp;
            int x = ((CommandMTL_SetViewport*)cmd)->x;
            int y = ((CommandMTL_SetViewport*)cmd)->y;
            int w = ((CommandMTL_SetViewport*)cmd)->w;
            int h = ((CommandMTL_SetViewport*)cmd)->h;

            if (!(x == 0 && y == 0 && w == 0 && h == 0))
            {
                vp.originX = x;
                vp.originY = y;
                vp.width = w;
                vp.height = h;
                vp.znear = 0.0;
                vp.zfar = 1.0;
            }
            else
            {
                vp.originX = 0;
                vp.originY = 0;
                vp.width = rt.width;
                vp.height = rt.height;
                vp.znear = 0.0;
                vp.zfar = 1.0;
            }

            [encoder setViewport:vp];
        }
        break;

        case MTL__SET_FILLMODE:
        {
            [encoder setTriangleFillMode:(FillMode(((CommandMTL_SetFillMode*)cmd)->mode) == FILLMODE_WIREFRAME) ? MTLTriangleFillModeLines : MTLTriangleFillModeFill];
        }
        break;

        case MTL__SET_VERTEX_DATA:
        {
            Handle vb = ((CommandMTL_SetVertexData*)cmd)->vb;
            unsigned streamIndex = ((CommandMTL_SetVertexData*)cmd)->streamIndex;

            cur_vb[streamIndex] = vb;
            StatSet::IncStat(stat_SET_VB, 1);
        }
        break;

        case MTL__SET_VERTEX_PROG_CONST_BUFFER:
        {
            Handle buffer = ((CommandMTL_SetVertexProgConstBuffer*)cmd)->buffer;
            unsigned index = ((CommandMTL_SetVertexProgConstBuffer*)cmd)->bufIndex;
            unsigned inst_offset = ((CommandMTL_SetVertexProgConstBuffer*)cmd)->inst_offset;

            ConstBufferMetal::SetToRHI(buffer, index, inst_offset, encoder);
        }
        break;

        case MTL__SET_VERTEX_TEXTURE:
        {
            Handle tex = ((CommandMTL_SetVertexTexture*)cmd)->tex;
            unsigned unitIndex = ((CommandMTL_SetVertexTexture*)cmd)->unitIndex;

            TextureMetal::SetToRHIVertex(tex, unitIndex, encoder);
            StatSet::IncStat(stat_SET_TEX, 1);
        }
        break;

        case MTL__SET_INDICES:
        {
            cur_ib = ((CommandMTL_SetIndices*)cmd)->ib;
            StatSet::IncStat(stat_SET_IB, 1);
        }
        break;

        case MTL__SET_QUERY_INDEX:
        {
            unsigned index = ((CommandMTL_SetQueryIndex*)cmd)->objectIndex;

            if (index != DAVA::InvalidIndex)
            {
                [encoder setVisibilityResultMode:MTLVisibilityResultModeBoolean offset:index * QueryBUfferElemeentAlign];
            }
            else
            {
                [encoder setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:0];
            }
        }
        break;

        case MTL__SET_QUERY_BUFFER:
            break; // do NOTHING

        case MTL__SET_FRAGMENT_PROG_CONST_BUFFER:
        {
            Handle buffer = ((CommandMTL_SetFragmentProgConstBuffer*)cmd)->buffer;
            unsigned index = ((CommandMTL_SetFragmentProgConstBuffer*)cmd)->bufIndex;
            unsigned inst_offset = ((CommandMTL_SetFragmentProgConstBuffer*)cmd)->inst_offset;

            ConstBufferMetal::SetToRHI(buffer, index, inst_offset, encoder);
        }
        break;

        case MTL__SET_FRAGMENT_TEXTURE:
        {
            Handle tex = ((CommandMTL_SetFragmentTexture*)cmd)->tex;
            unsigned unitIndex = ((CommandMTL_SetFragmentTexture*)cmd)->unitIndex;

            TextureMetal::SetToRHIFragment(tex, unitIndex, encoder);
            StatSet::IncStat(stat_SET_TEX, 1);
        }
        break;

        case MTL__SET_DEPTHSTENCIL_STATE:
        {
            DepthStencilStateMetal::SetToRHI(((CommandMTL_SetDepthStencilState*)cmd)->depthStencilState, encoder);
        }
        break;

        case MTL__SET_SAMPLER_STATE:
        {
            SamplerStateMetal::SetToRHI(((CommandMTL_SetSamplerState*)cmd)->samplerState, encoder);
            StatSet::IncStat(stat_SET_SS, 1);
        }
        break;

        case MTL__DRAW_PRIMITIVE:
        {
            MTLPrimitiveType ptype = MTLPrimitiveType(((CommandMTL_DrawPrimitive*)cmd)->type);
            unsigned vertexCount = ((CommandMTL_DrawPrimitive*)cmd)->vertexCount;
            unsigned baseVertex = ((CommandMTL_DrawPrimitive*)cmd)->baseVertex;

            _ApplyVertexData();
            [encoder drawPrimitives:ptype vertexStart:0 vertexCount:vertexCount];
        }
        break;

        case MTL__DRAW_INDEXED_PRIMITIVE:
        {
            MTLPrimitiveType ptype = MTLPrimitiveType(((CommandMTL_DrawIndexedPrimitive*)cmd)->type);
            unsigned vertexCount = ((CommandMTL_DrawIndexedPrimitive*)cmd)->vertexCount;
            unsigned baseVertex = ((CommandMTL_DrawIndexedPrimitive*)cmd)->baseVertex;
            unsigned startIndex = ((CommandMTL_DrawIndexedPrimitive*)cmd)->startIndex;

            unsigned i_cnt = 0;
            unsigned ib_base = 0;
            id<MTLBuffer> ib = IndexBufferMetal::GetBuffer(cur_ib, &ib_base);
            MTLIndexType i_type = IndexBufferMetal::GetType(cur_ib);
            unsigned i_off = (i_type == MTLIndexTypeUInt16) ? startIndex * sizeof(uint16) : startIndex * sizeof(uint32);

            _ApplyVertexData(baseVertex);
            [encoder drawIndexedPrimitives:ptype indexCount:vertexCount indexType:i_type indexBuffer:ib indexBufferOffset:ib_base + i_off];
        }
        break;

        case MTL__DRAW_INSTANCED_PRIMITIVE:
        {
            MTLPrimitiveType ptype = MTLPrimitiveType(((CommandMTL_DrawIndexedPrimitive*)cmd)->type);
            unsigned vertexCount = ((CommandMTL_DrawInstancedPrimitive*)cmd)->vertexCount;
            unsigned baseVertex = ((CommandMTL_DrawInstancedPrimitive*)cmd)->baseVertex;
            unsigned instCount = ((CommandMTL_DrawInstancedPrimitive*)cmd)->instCount;

            _ApplyVertexData();
            [encoder drawPrimitives:ptype vertexStart:0 vertexCount:vertexCount instanceCount:instCount];
        }
        break;

        case MTL__DRAW_INSTANCED_INDEXED_PRIMITIVE:
        {
            MTLPrimitiveType ptype = MTLPrimitiveType(((CommandMTL_DrawIndexedPrimitive*)cmd)->type);
            unsigned vertexCount = ((CommandMTL_DrawInstancedIndexedPrimitive*)cmd)->vertexCount;
            unsigned baseVertex = ((CommandMTL_DrawInstancedIndexedPrimitive*)cmd)->baseVertex;
            unsigned indexCount = ((CommandMTL_DrawInstancedIndexedPrimitive*)cmd)->indexCount;
            unsigned startIndex = ((CommandMTL_DrawInstancedIndexedPrimitive*)cmd)->startIndex;
            unsigned instCount = ((CommandMTL_DrawInstancedIndexedPrimitive*)cmd)->instCount;
            unsigned baseInst = ((CommandMTL_DrawInstancedIndexedPrimitive*)cmd)->baseInst;

            unsigned i_cnt = 0;
            unsigned ib_base = 0;
            id<MTLBuffer> ib = IndexBufferMetal::GetBuffer(cur_ib, &ib_base);
            MTLIndexType i_type = IndexBufferMetal::GetType(cur_ib);
            unsigned i_off = (i_type == MTLIndexTypeUInt16) ? startIndex * sizeof(uint16) : startIndex * sizeof(uint32);

            _ApplyVertexData(baseVertex);
            [encoder drawIndexedPrimitives:ptype indexCount:vertexCount indexType:i_type indexBuffer:ib indexBufferOffset:ib_base + i_off instanceCount:instCount];
        }
        break;
        }

        if (cmd->type == MTL__END)
            break;
        c += cmd->size;
    }
}
#endif

//------------------------------------------------------------------------------

#if !RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

bool
RenderPassMetal_t::Initialize()
{
    bool need_drawable = cfg.colorBuffer[0].texture == InvalidHandle;

    if (need_drawable && !_Metal_Frame.back().drawable)
    {
        @autoreleasepool
        {
            _Metal_Frame.back().drawable = [_Metal_Layer nextDrawable];
            [_Metal_Frame.back().drawable retain];
            //            MTL_TRACE(" next.drawable= %p %i %s", (void*)(f.drawable), [f.drawable retainCount], NSStringFromClass([f.drawable class]).UTF8String);
            _Metal_DefFrameBuf = _Metal_Frame.back().drawable.texture;
        }
    }

    if (need_drawable && !_Metal_Frame.back().drawable)
    {
        _Metal_Frame.clear();

        for (unsigned i = 0; i != cmdBuf.size(); ++i)
        {
            CommandBufferPool::Free(cmdBuf[i]);
        }
        cmdBuf.clear();

        MTL_TRACE("-rp.init failed (no drawable)");
        _Metal_NewFramePending = true;
        return false;
    }

    bool ds_used = false;

    desc = [MTLRenderPassDescriptor renderPassDescriptor];

    if (cfg.colorBuffer[0].texture == InvalidHandle)
        desc.colorAttachments[0].texture = _Metal_Frame.back().drawable.texture;
    else
        TextureMetal::SetAsRenderTarget(cfg.colorBuffer[0].texture, desc);

    switch (cfg.colorBuffer[0].loadAction)
    {
    case LOADACTION_CLEAR:
        desc.colorAttachments[0].loadAction = MTLLoadActionClear;
        break;
    case LOADACTION_LOAD:
        desc.colorAttachments[0].loadAction = MTLLoadActionLoad;
        break;
    default:
        desc.colorAttachments[0].loadAction = MTLLoadActionDontCare;
    }

    desc.colorAttachments[0].storeAction = MTLStoreActionStore;
    desc.colorAttachments[0].clearColor = MTLClearColorMake(cfg.colorBuffer[0].clearColor[0], cfg.colorBuffer[0].clearColor[1], cfg.colorBuffer[0].clearColor[2], cfg.colorBuffer[0].clearColor[3]);

    if (cfg.depthStencilBuffer.texture == rhi::DefaultDepthBuffer)
    {
        desc.depthAttachment.texture = _Metal_DefDepthBuf;
        desc.stencilAttachment.texture = _Metal_DefStencilBuf;
        ds_used = true;
    }
    else if (cfg.depthStencilBuffer.texture != rhi::InvalidHandle)
    {
        TextureMetal::SetAsDepthStencil(cfg.depthStencilBuffer.texture, desc);
        ds_used = true;
    }

    if (ds_used)
    {
        desc.depthAttachment.loadAction = (cfg.depthStencilBuffer.loadAction == LOADACTION_CLEAR) ? MTLLoadActionClear : MTLLoadActionDontCare;
        desc.depthAttachment.storeAction = (cfg.depthStencilBuffer.storeAction == STOREACTION_STORE) ? MTLStoreActionStore : MTLStoreActionDontCare;
        desc.depthAttachment.clearDepth = cfg.depthStencilBuffer.clearDepth;

        desc.stencilAttachment.loadAction = (cfg.depthStencilBuffer.loadAction == LOADACTION_CLEAR) ? MTLLoadActionClear : MTLLoadActionDontCare;
        desc.stencilAttachment.storeAction = (cfg.depthStencilBuffer.storeAction == STOREACTION_STORE) ? MTLStoreActionStore : MTLStoreActionDontCare;
        desc.stencilAttachment.clearStencil = cfg.depthStencilBuffer.clearStencil;
    }

    if (cfg.queryBuffer != InvalidHandle)
    {
        desc.visibilityResultBuffer = QueryBufferMetal::GetBuffer(cfg.queryBuffer);
    }

    do_present = cfg.colorBuffer[0].texture == InvalidHandle;

    id<MTLCommandBuffer> pbuf = nil;

    if (cmdBuf.size() == 1)
    {
        CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf[0]);

        encoder = nil;
        //pass->buf = [_Metal_DefCmdQueue commandBufferWithUnretainedReferences];
        buf = [_Metal_DefCmdQueue commandBuffer];
        [buf retain];

        cb->encoder = [buf renderCommandEncoderWithDescriptor:desc];
        [cb->encoder retain];

        cb->buf = buf;
        cb->rt = desc.colorAttachments[0].texture;
        cb->color_fmt = desc.colorAttachments[0].texture.pixelFormat;
        cb->ds_used = ds_used;
        cb->cur_ib = InvalidHandle;
        cb->cur_vstream_count = 0;
        for (unsigned s = 0; s != countof(cb->cur_vb); ++s)
            cb->cur_vb[s] = InvalidHandle;

        #if RHI_METAL__COMMIT_COMMAND_BUFFER_ON_END
        cb->do_commit_on_end = !do_present;
        #endif

        if (do_present)
            pbuf = buf;
    }
    else
    {
        //pass->buf = [_Metal_DefCmdQueue commandBufferWithUnretainedReferences];
        buf = [_Metal_DefCmdQueue commandBuffer];
        [buf retain];
        encoder = [buf parallelRenderCommandEncoderWithDescriptor:desc];
        [encoder retain];

        for (unsigned i = 0; i != cmdBuf.size(); ++i)
        {
            CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf[i]);

            cb->encoder = [encoder renderCommandEncoder];
            [cb->encoder retain];
            cb->buf = buf;
            cb->rt = desc.colorAttachments[0].texture;
            cb->color_fmt = desc.colorAttachments[0].texture.pixelFormat;
            cb->ds_used = ds_used;
            cb->cur_ib = InvalidHandle;
            cb->cur_vstream_count = 0;
            for (unsigned s = 0; s != countof(cb->cur_vb); ++s)
                cb->cur_vb[s] = InvalidHandle;

            #if RHI_METAL__COMMIT_COMMAND_BUFFER_ON_END
            cb->do_commit_on_end = !do_present;
            #endif

            if (i == 0 && do_present)
                pbuf = cb->buf;
        }
    }

    return true;
}

#endif

//------------------------------------------------------------------------------

static Handle
metal_RenderPass_Allocate(const RenderPassConfig& passConf, uint32 cmdBufCount, Handle* cmdBuf)
{
    if (_Metal_Suspended.GetRelaxed())
    {
        for (unsigned i = 0; i != cmdBufCount; ++i)
            cmdBuf[i] = InvalidHandle;

        MTL_TRACE("-rp.alloc InvalidHande (suspended)");
        return InvalidHandle;
    }

    DVASSERT(cmdBufCount);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    if (_Metal_NewFramePending)
    {
        MTL_TRACE("--- next-frame");
        FrameMetal_t f;

        f.drawable = nil;
        _Metal_Frame.push_back(f);
        _Metal_NewFramePending = false;
    }

    bool need_drawable = passConf.colorBuffer[0].texture == InvalidHandle && !_Metal_Frame.back().drawable;

    if (need_drawable)
    {
        @autoreleasepool
        {
            _Metal_Frame.back().drawable = [_Metal_Layer nextDrawable];
            [_Metal_Frame.back().drawable retain];
            _Metal_DefFrameBuf = _Metal_Frame.back().drawable.texture;
            MTL_TRACE(" next.drawable= %p %i %s", (void*)(_Metal_Frame.back().drawable), [_Metal_Frame.back().drawable retainCount], NSStringFromClass([_Metal_Frame.back().drawable class]).UTF8String);
        }
    }

    if (need_drawable && !_Metal_Frame.back().drawable)
    {
        _Metal_Frame.clear();

        for (unsigned i = 0; i != cmdBufCount; ++i)
            cmdBuf[i] = InvalidHandle;

        MTL_TRACE("-rp.alloc InvalidHande (no drawable)");
        _Metal_NewFramePending = true;
        return InvalidHandle;
    }

    Handle pass_h = RenderPassPool::Alloc();
    MTL_TRACE("-rp.alloc %u (%u)", RHI_HANDLE_INDEX(pass_h), cmdBufCount);
    RenderPassMetal_t* pass = RenderPassPool::Get(pass_h);
    bool ds_used = false;
    pass->desc = [MTLRenderPassDescriptor renderPassDescriptor];

    if (passConf.colorBuffer[0].texture == InvalidHandle)
        pass->desc.colorAttachments[0].texture = _Metal_Frame.back().drawable.texture;
    else
        TextureMetal::SetAsRenderTarget(passConf.colorBuffer[0].texture, pass->desc);

    switch (passConf.colorBuffer[0].loadAction)
    {
    case LOADACTION_CLEAR:
        pass->desc.colorAttachments[0].loadAction = MTLLoadActionClear;
        break;
    case LOADACTION_LOAD:
        pass->desc.colorAttachments[0].loadAction = MTLLoadActionLoad;
        break;
    default:
        pass->desc.colorAttachments[0].loadAction = MTLLoadActionDontCare;
    }

    pass->desc.colorAttachments[0].storeAction = MTLStoreActionStore;
    pass->desc.colorAttachments[0].clearColor = MTLClearColorMake(passConf.colorBuffer[0].clearColor[0], passConf.colorBuffer[0].clearColor[1], passConf.colorBuffer[0].clearColor[2], passConf.colorBuffer[0].clearColor[3]);

    if (passConf.depthStencilBuffer.texture == rhi::DefaultDepthBuffer)
    {
        pass->desc.depthAttachment.texture = _Metal_DefDepthBuf;
        pass->desc.stencilAttachment.texture = _Metal_DefStencilBuf;
        ds_used = true;
    }
    else if (passConf.depthStencilBuffer.texture != rhi::InvalidHandle)
    {
        TextureMetal::SetAsDepthStencil(passConf.depthStencilBuffer.texture, pass->desc);
        ds_used = true;
    }

    if (ds_used)
    {
        pass->desc.depthAttachment.loadAction = (passConf.depthStencilBuffer.loadAction == LOADACTION_CLEAR) ? MTLLoadActionClear : MTLLoadActionDontCare;
        pass->desc.depthAttachment.storeAction = (passConf.depthStencilBuffer.storeAction == STOREACTION_STORE) ? MTLStoreActionStore : MTLStoreActionDontCare;
        pass->desc.depthAttachment.clearDepth = passConf.depthStencilBuffer.clearDepth;

        pass->desc.stencilAttachment.loadAction = (passConf.depthStencilBuffer.loadAction == LOADACTION_CLEAR) ? MTLLoadActionClear : MTLLoadActionDontCare;
        pass->desc.stencilAttachment.storeAction = (passConf.depthStencilBuffer.storeAction == STOREACTION_STORE) ? MTLStoreActionStore : MTLStoreActionDontCare;
        pass->desc.stencilAttachment.clearStencil = passConf.depthStencilBuffer.clearStencil;
    }

    if (passConf.queryBuffer != InvalidHandle)
    {
        pass->desc.visibilityResultBuffer = QueryBufferMetal::GetBuffer(passConf.queryBuffer);
    }

    pass->cmdBuf.resize(cmdBufCount);
    pass->priority = passConf.priority;
    pass->do_present = passConf.colorBuffer[0].texture == InvalidHandle;

    id<MTLCommandBuffer> pbuf = nil;

    if (cmdBufCount == 1)
    {
        Handle cb_h = CommandBufferPool::Alloc();
        CommandBufferMetal_t* cb = CommandBufferPool::Get(cb_h);

        pass->encoder = nil;
        //pass->buf = [_Metal_DefCmdQueue commandBufferWithUnretainedReferences];
        pass->buf = [_Metal_DefCmdQueue commandBuffer];
        [pass->buf retain];

        cb->encoder = [pass->buf renderCommandEncoderWithDescriptor:pass->desc];
        [cb->encoder retain];

        cb->buf = pass->buf;
        cb->rt = pass->desc.colorAttachments[0].texture;
        cb->color_fmt = pass->desc.colorAttachments[0].texture.pixelFormat;
        cb->ds_used = ds_used;
        cb->cur_ib = InvalidHandle;
        cb->cur_vstream_count = 0;
        for (unsigned s = 0; s != countof(cb->cur_vb); ++s)
            cb->cur_vb[s] = InvalidHandle;

        #if RHI_METAL__COMMIT_COMMAND_BUFFER_ON_END
        cb->do_commit_on_end = !pass->do_present;
        #endif

        pass->cmdBuf[0] = cb_h;
        cmdBuf[0] = cb_h;

        if (pass->do_present)
            pbuf = pass->buf;
    }
    else
    {
        //pass->buf = [_Metal_DefCmdQueue commandBufferWithUnretainedReferences];
        pass->buf = [_Metal_DefCmdQueue commandBuffer];
        [pass->buf retain];
        pass->encoder = [pass->buf parallelRenderCommandEncoderWithDescriptor:pass->desc];
        [pass->encoder retain];

        for (unsigned i = 0; i != cmdBufCount; ++i)
        {
            Handle cb_h = CommandBufferPool::Alloc();
            CommandBufferMetal_t* cb = CommandBufferPool::Get(cb_h);

            cb->encoder = [pass->encoder renderCommandEncoder];
            [cb->encoder retain];
            cb->buf = pass->buf;
            cb->rt = pass->desc.colorAttachments[0].texture;
            cb->color_fmt = pass->desc.colorAttachments[0].texture.pixelFormat;
            cb->ds_used = ds_used;
            cb->cur_ib = InvalidHandle;
            cb->cur_vstream_count = 0;
            for (unsigned s = 0; s != countof(cb->cur_vb); ++s)
                cb->cur_vb[s] = InvalidHandle;
            
            #if RHI_METAL__COMMIT_COMMAND_BUFFER_ON_END
            cb->do_commit_on_end = !pass->do_present;
            #endif

            pass->cmdBuf[i] = cb_h;
            cmdBuf[i] = cb_h;

            if (i == 0 && pass->do_present)
                pbuf = cb->buf;
        }
    }

    return pass_h;
    
#else

    if (_Metal_NewFramePending)
    {
        FrameMetal_t f;

        f.drawable = nil;
        _Metal_Frame.push_back(f);
        _Metal_NewFramePending = false;
    }

    Handle pass_h = RenderPassPool::Alloc();
    MTL_TRACE("-rp.alloc %u (%u)", RHI_HANDLE_INDEX(pass_h), cmdBufCount);
    RenderPassMetal_t* pass = RenderPassPool::Get(pass_h);

    pass->cfg = passConf;
    pass->priority = passConf.priority;

    pass->cmdBuf.resize(cmdBufCount);
    for (unsigned i = 0; i != cmdBufCount; ++i)
    {
        Handle cb_h = CommandBufferPool::Alloc();
        CommandBufferMetal_t* cb = CommandBufferPool::Get(cb_h);

        cb->ds_used = passConf.depthStencilBuffer.texture != rhi::InvalidHandle;

        pass->cmdBuf[i] = cb_h;
        cmdBuf[i] = cb_h;
    }

    return pass_h;

#endif
}

static void
metal_RenderPass_Begin(Handle pass_h)
{
    MTL_TRACE(" -rp.begin %u", RHI_HANDLE_INDEX(pass_h));
    RenderPassMetal_t* pass = RenderPassPool::Get(pass_h);

    pass->finished = false;
    _Metal_Frame.back().pass.push_back(pass_h);
    MTL_TRACE("  drawable %p %i", (void*)(_Metal_Frame.back().drawable), [_Metal_Frame.back().drawable retainCount]);
}

static void
metal_RenderPass_End(Handle pass_h)
{
    MTL_TRACE(" -rp.end %u", RHI_HANDLE_INDEX(pass_h));
    RenderPassMetal_t* pass = RenderPassPool::Get(pass_h);

    DVASSERT(!pass->finished);
    pass->finished = true;

    #if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    if (pass->cmdBuf.size() > 1)
    {
        [pass->encoder endEncoding];
    }
    #endif
    MTL_TRACE("  drawable %p %i", (void*)(_Metal_Frame.back().drawable), [_Metal_Frame.back().drawable retainCount]);
}

namespace RenderPassMetal
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_Renderpass_Allocate = &metal_RenderPass_Allocate;
    dispatch->impl_Renderpass_Begin = &metal_RenderPass_Begin;
    dispatch->impl_Renderpass_End = &metal_RenderPass_End;
}
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_Begin(Handle cmdBuf)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

    cb->cur_vstream_count = 0;
    for (unsigned s = 0; s != countof(cb->cur_vb); ++s)
        cb->cur_vb[s] = InvalidHandle;

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    [cb->encoder setDepthStencilState:_Metal_DefDepthState];
#else
    cb->curUsedSize = 0;
    CommandMTL_Begin* cmd = cb->allocCmd<CommandMTL_Begin>();
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_End(Handle cmdBuf, Handle syncObject)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    [cb->encoder endEncoding];

    if (syncObject != InvalidHandle)
    {
        [cb->buf addCompletedHandler:^(id<MTLCommandBuffer> cmdb) {
          SyncObjectMetal_t* sync = SyncObjectPool::Get(syncObject);

          sync->is_signaled = true;
        }];
    }
    
    #if RHI_METAL__COMMIT_COMMAND_BUFFER_ON_END
    if (cb->do_commit_on_end)
        [cb->buf commit];
    #endif
    
#else
    CommandMTL_End* cmd = cb->allocCmd<CommandMTL_End>();
    cmd->syncObject = syncObject;
    #if RHI_METAL__COMMIT_COMMAND_BUFFER_ON_END
    cmd->doCommit = cb->do_commit_on_end;
    #endif
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetPipelineState(Handle cmdBuf, Handle ps, uint32 layoutUID)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    cb->cur_stride = PipelineStateMetal::SetToRHI(ps, layoutUID, cb->color_fmt, cb->ds_used, cb->encoder);
    cb->cur_vstream_count = PipelineStateMetal::VertexStreamCount(ps);
    StatSet::IncStat(stat_SET_PS, 1);
#else
    CommandMTL_SetPipelineState* cmd = cb->allocCmd<CommandMTL_SetPipelineState>();
    cmd->ps = ps;
    cmd->vdeclUID = layoutUID;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetCullMode(Handle cmdBuf, CullMode mode)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    switch (mode)
    {
    case CULL_NONE:
        [cb->encoder setCullMode:MTLCullModeNone];
        break;

    case CULL_CCW:
        [cb->encoder setFrontFacingWinding:MTLWindingClockwise];
        [cb->encoder setCullMode:MTLCullModeBack];
        break;

    case CULL_CW:
        [cb->encoder setFrontFacingWinding:MTLWindingClockwise];
        [cb->encoder setCullMode:MTLCullModeFront];
        break;
    }
#else
    CommandMTL_SetCullMode* cmd = cb->allocCmd<CommandMTL_SetCullMode>();
    cmd->mode = mode;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetScissorRect(Handle cmdBuf, ScissorRect rect)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);
    MTLScissorRect rc;

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    if (!(rect.x == 0 && rect.y == 0 && rect.width == 0 && rect.height == 0))
    {
        unsigned max_x = (cb->rt) ? unsigned(cb->rt.width) : unsigned(_Metal_DefFrameBuf.width);
        unsigned max_y = (cb->rt) ? unsigned(cb->rt.height) : unsigned(_Metal_DefFrameBuf.height);

        rc.x = rect.x;
        rc.y = rect.y;
        rc.width = (rect.x + rect.width > max_x) ? (max_x - rc.x) : rect.width;
        rc.height = (rect.y + rect.height > max_y) ? (max_y - rc.y) : rect.height;

        if (rc.width == 0)
        {
            rc.width = 1;
            if (rc.x > 0)
                --rc.x;
        }

        if (rc.height == 0)
        {
            rc.height = 1;
            if (rc.y > 0)
                --rc.y;
        }
    }
    else
    {
        rc.x = 0;
        rc.y = 0;
        if (cb->rt)
        {
            rc.width = cb->rt.width;
            rc.height = cb->rt.height;
        }
        else
        {
            rc.width = _Metal_DefFrameBuf.width;
            rc.height = _Metal_DefFrameBuf.height;
        }
    }

    [cb->encoder setScissorRect:rc];

#else

    int x = rect.x;
    int y = rect.y;
    int w = rect.width;
    int h = rect.height;
    CommandMTL_SetScissorRect* cmd = cb->allocCmd<CommandMTL_SetScissorRect>();
    cmd->x = x;
    cmd->y = y;
    cmd->w = w;
    cmd->h = h;
    
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetViewport(Handle cmdBuf, Viewport viewport)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);
    MTLViewport vp;

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    if (!(viewport.x == 0 && viewport.y == 0 && viewport.width == 0 && viewport.height == 0))
    {
        vp.originX = viewport.x;
        vp.originY = viewport.y;
        vp.width = viewport.width;
        vp.height = viewport.height;
        vp.znear = 0.0;
        vp.zfar = 1.0;
    }
    else
    {
        vp.originX = 0;
        vp.originY = 0;
        vp.width = cb->rt.width;
        vp.height = cb->rt.height;
        vp.znear = 0.0;
        vp.zfar = 1.0;
    }

    [cb->encoder setViewport:vp];

#else

    int x = viewport.x;
    int y = viewport.y;
    int w = viewport.width;
    int h = viewport.height;
    CommandMTL_SetViewport* cmd = cb->allocCmd<CommandMTL_SetViewport>();
    cmd->x = x;
    cmd->y = y;
    cmd->w = w;
    cmd->h = h;
    

#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetFillMode(Handle cmdBuf, FillMode mode)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    [cb->encoder setTriangleFillMode:(mode == FILLMODE_WIREFRAME) ? MTLTriangleFillModeLines : MTLTriangleFillModeFill];
#else
    CommandMTL_SetFillMode* cmd = cb->allocCmd<CommandMTL_SetFillMode>();
    cmd->mode = mode;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetVertexData(Handle cmdBuf, Handle vb, uint32 streamIndex)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    cb->cur_vb[streamIndex] = vb;
    StatSet::IncStat(stat_SET_VB, 1);
#else
    CommandMTL_SetVertexData* cmd = cb->allocCmd<CommandMTL_SetVertexData>();
    cmd->vb = vb;
    cmd->streamIndex = streamIndex;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetVertexConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    ConstBufferMetal::SetToRHI(buffer, bufIndex, cb->encoder);
    StatSet::IncStat(stat_SET_CB, 1);
#else
    CommandMTL_SetVertexProgConstBuffer* cmd = cb->allocCmd<CommandMTL_SetVertexProgConstBuffer>();
    cmd->bufIndex = bufIndex;
    cmd->buffer = buffer;
    cmd->inst_offset = ConstBufferMetal::Instance(buffer);
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetVertexTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    TextureMetal::SetToRHIVertex(tex, unitIndex, cb->encoder);
    StatSet::IncStat(stat_SET_TEX, 1);
#else
    CommandMTL_SetVertexTexture* cmd = cb->allocCmd<CommandMTL_SetVertexTexture>();
    cmd->unitIndex = unitIndex;
    cmd->tex = tex;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetIndices(Handle cmdBuf, Handle ib)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    cb->cur_ib = ib;
    StatSet::IncStat(stat_SET_IB, 1);
#else
    CommandMTL_SetIndices* cmd = cb->allocCmd<CommandMTL_SetIndices>();
    cmd->ib = ib;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetQueryIndex(Handle cmdBuf, uint32 objectIndex)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    if (objectIndex != DAVA::InvalidIndex)
    {
        [cb->encoder setVisibilityResultMode:MTLVisibilityResultModeBoolean offset:objectIndex * QueryBUfferElemeentAlign];
    }
    else
    {
        [cb->encoder setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:0];
    }
#else
    CommandMTL_SetQueryIndex* cmd = cb->allocCmd<CommandMTL_SetQueryIndex>();
    cmd->objectIndex = objectIndex;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetQueryBuffer(Handle /*cmdBuf*/, Handle /*queryBuf*/)
{
    // do NOTHING, since query-buffer specified in render-pass
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetFragmentConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    ConstBufferMetal::SetToRHI(buffer, bufIndex, cb->encoder);
    StatSet::IncStat(stat_SET_CB, 1);
#else
    CommandMTL_SetFragmentProgConstBuffer* cmd = cb->allocCmd<CommandMTL_SetFragmentProgConstBuffer>();
    cmd->bufIndex = bufIndex;
    cmd->buffer = buffer;
    cmd->inst_offset = ConstBufferMetal::Instance(buffer);
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetFragmentTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    TextureMetal::SetToRHIFragment(tex, unitIndex, cb->encoder);
    StatSet::IncStat(stat_SET_TEX, 1);
#else
    CommandMTL_SetFragmentTexture* cmd = cb->allocCmd<CommandMTL_SetFragmentTexture>();
    cmd->unitIndex = unitIndex;
    cmd->tex = tex;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetDepthStencilState(Handle cmdBuf, Handle depthStencilState)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    DepthStencilStateMetal::SetToRHI(depthStencilState, cb->encoder);
#else
    CommandMTL_SetDepthStencilState* cmd = cb->allocCmd<CommandMTL_SetDepthStencilState>();
    cmd->depthStencilState = depthStencilState;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetSamplerState(Handle cmdBuf, const Handle samplerState)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    SamplerStateMetal::SetToRHI(samplerState, cb->encoder);
    StatSet::IncStat(stat_SET_SS, 1);
#else
    CommandMTL_SetSamplerState* cmd = cb->allocCmd<CommandMTL_SetSamplerState>();
    cmd->samplerState = samplerState;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_DrawPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);
    MTLPrimitiveType ptype = MTLPrimitiveTypeTriangle;
    unsigned v_cnt = 0;

    switch (type)
    {
    case PRIMITIVE_TRIANGLELIST:
        ptype = MTLPrimitiveTypeTriangle;
        v_cnt = count * 3;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        ptype = MTLPrimitiveTypeTriangleStrip;
        v_cnt = 2 + count;
        break;

    case PRIMITIVE_LINELIST:
        ptype = MTLPrimitiveTypeLine;
        v_cnt = count * 2;
        break;
    }

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    cb->_ApplyVertexData();
    [cb->encoder drawPrimitives:ptype vertexStart:0 vertexCount:v_cnt];

    StatSet::IncStat(stat_DP, 1);
    switch (ptype)
    {
    case MTLPrimitiveTypeTriangle:
        StatSet::IncStat(stat_DTL, 1);
        break;
    case MTLPrimitiveTypeTriangleStrip:
        StatSet::IncStat(stat_DTS, 1);
        break;
    case MTLPrimitiveTypeLine:
        StatSet::IncStat(stat_DLL, 1);
        break;
    default:
        break;
    }

#else

    CommandMTL_DrawPrimitive* cmd = cb->allocCmd<CommandMTL_DrawPrimitive>();

    cmd->type = ptype;
    cmd->vertexCount = v_cnt;
    cmd->baseVertex = 0;

#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_DrawIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count, uint32 /*vertexCount*/, uint32 firstVertex, uint32 startIndex)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);
    MTLPrimitiveType ptype = MTLPrimitiveTypeTriangle;
    unsigned i_cnt = 0;

    switch (type)
    {
    case PRIMITIVE_TRIANGLELIST:
        ptype = MTLPrimitiveTypeTriangle;
        i_cnt = count * 3;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        ptype = MTLPrimitiveTypeTriangleStrip;
        i_cnt = 2 + count;
        break;

    case PRIMITIVE_LINELIST:
        ptype = MTLPrimitiveTypeLine;
        i_cnt = count * 2;
        break;
    }

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    unsigned ib_base = 0;
    id<MTLBuffer> ib = IndexBufferMetal::GetBuffer(cb->cur_ib, &ib_base);
    MTLIndexType i_type = IndexBufferMetal::GetType(cb->cur_ib);
    unsigned i_off = (i_type == MTLIndexTypeUInt16) ? startIndex * sizeof(uint16) : startIndex * sizeof(uint32);

    cb->_ApplyVertexData(firstVertex);
    [cb->encoder drawIndexedPrimitives:ptype indexCount:i_cnt indexType:i_type indexBuffer:ib indexBufferOffset:ib_base + i_off];

    StatSet::IncStat(stat_DIP, 1);
    switch (ptype)
    {
    case MTLPrimitiveTypeTriangle:
        StatSet::IncStat(stat_DTL, 1);
        break;
    case MTLPrimitiveTypeTriangleStrip:
        StatSet::IncStat(stat_DTS, 1);
        break;
    case MTLPrimitiveTypeLine:
        StatSet::IncStat(stat_DLL, 1);
        break;
    default:
        break;
    }

#else

    CommandMTL_DrawIndexedPrimitive* cmd = cb->allocCmd<CommandMTL_DrawIndexedPrimitive>();

    cmd->type = ptype;
    cmd->vertexCount = i_cnt;
    cmd->baseVertex = firstVertex;
    cmd->startIndex = startIndex;

#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_DrawInstancedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 inst_count, uint32 prim_count)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);
    MTLPrimitiveType ptype = MTLPrimitiveTypeTriangle;
    unsigned v_cnt = 0;

    switch (type)
    {
    case PRIMITIVE_TRIANGLELIST:
        ptype = MTLPrimitiveTypeTriangle;
        v_cnt = prim_count * 3;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        ptype = MTLPrimitiveTypeTriangleStrip;
        v_cnt = 2 + prim_count;
        break;

    case PRIMITIVE_LINELIST:
        ptype = MTLPrimitiveTypeLine;
        v_cnt = prim_count * 2;
        break;
    }

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    cb->_ApplyVertexData();
    [cb->encoder drawPrimitives:ptype vertexStart:0 vertexCount:v_cnt instanceCount:inst_count];

    StatSet::IncStat(stat_DP, 1);
    switch (ptype)
    {
    case MTLPrimitiveTypeTriangle:
        StatSet::IncStat(stat_DTL, 1);
        break;
    case MTLPrimitiveTypeTriangleStrip:
        StatSet::IncStat(stat_DTS, 1);
        break;
    case MTLPrimitiveTypeLine:
        StatSet::IncStat(stat_DLL, 1);
        break;
    default:
        break;
    }

#else

    CommandMTL_DrawInstancedPrimitive* cmd = cb->allocCmd<CommandMTL_DrawInstancedPrimitive>();

    cmd->type = ptype;
    cmd->instCount = inst_count;
    cmd->vertexCount = v_cnt;
    cmd->baseVertex = 0;

#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_DrawInstancedIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 instCount, uint32 prim_count, uint32 /*vertexCount*/, uint32 firstVertex, uint32 startIndex, uint32 baseInst)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);
    MTLPrimitiveType ptype = MTLPrimitiveTypeTriangle;
    unsigned i_cnt = 0;

    switch (type)
    {
    case PRIMITIVE_TRIANGLELIST:
        ptype = MTLPrimitiveTypeTriangle;
        i_cnt = prim_count * 3;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        ptype = MTLPrimitiveTypeTriangleStrip;
        i_cnt = 2 + prim_count;
        break;

    case PRIMITIVE_LINELIST:
        ptype = MTLPrimitiveTypeLine;
        i_cnt = prim_count * 2;
        break;
    }

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    unsigned ib_base = 0;
    id<MTLBuffer> ib = IndexBufferMetal::GetBuffer(cb->cur_ib, &ib_base);
    MTLIndexType i_type = IndexBufferMetal::GetType(cb->cur_ib);
    unsigned i_off = (i_type == MTLIndexTypeUInt16) ? startIndex * sizeof(uint16) : startIndex * sizeof(uint32);

    cb->_ApplyVertexData(firstVertex);
    [cb->encoder drawIndexedPrimitives:ptype indexCount:i_cnt indexType:i_type indexBuffer:ib indexBufferOffset:ib_base + i_off instanceCount:instCount];

    StatSet::IncStat(stat_DIP, 1);
    switch (ptype)
    {
    case MTLPrimitiveTypeTriangle:
        StatSet::IncStat(stat_DTL, 1);
        break;
    case MTLPrimitiveTypeTriangleStrip:
        StatSet::IncStat(stat_DTS, 1);
        break;
    case MTLPrimitiveTypeLine:
        StatSet::IncStat(stat_DLL, 1);
        break;
    default:
        break;
    }
#else
    CommandMTL_DrawInstancedIndexedPrimitive* cmd = cb->allocCmd<CommandMTL_DrawInstancedIndexedPrimitive>();

    cmd->type = ptype;
    cmd->vertexCount = i_cnt;
    cmd->baseVertex = firstVertex;
    cmd->startIndex = startIndex;
    cmd->instCount = instCount;
    cmd->baseInst = baseInst;

#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetMarker(Handle cmdBuf, const char* text)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);
    
#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    NSString* txt = [[NSString alloc] initWithUTF8String:text];

    [cb->encoder insertDebugSignpost:txt];
    [txt release];
#else
#endif
}

//------------------------------------------------------------------------------

static Handle
metal_SyncObject_Create()
{
    Handle handle = SyncObjectPool::Alloc();
    SyncObjectMetal_t* sync = SyncObjectPool::Get(handle);

    sync->is_signaled = false;

    return handle;
}

//------------------------------------------------------------------------------

static void
metal_SyncObject_Delete(Handle obj)
{
    SyncObjectPool::Free(obj);
}

//------------------------------------------------------------------------------

static bool
metal_SyncObject_IsSignaled(Handle obj)
{
    bool signaled = false;

    if (SyncObjectPool::IsAlive(obj))
    {
        SyncObjectMetal_t* sync = SyncObjectPool::Get(obj);

        if (sync)
            signaled = sync->is_signaled;
    }
    else
    {
        signaled = true;
    }

    return signaled;
}

//------------------------------------------------------------------------------

static void
metal_Present(Handle syncObject)
{
    PROFILER_TIMING("rhi::Present");

    static unsigned frame_n = 0;
    MTL_TRACE("--present %u", ++frame_n);

    if (_Metal_Frame.size() == 0)
    {
        if (syncObject != InvalidHandle)
        {
            SyncObjectMetal_t* sync = SyncObjectPool::Get(syncObject);
            sync->is_signaled = true;
        }

        _Metal_NewFramePending = true;
        MTL_TRACE("  no-frames");
        return;
    }

    bool do_discard = TextureMetal::NeedRestoreCount();

    if (_Metal_Suspended.GetRelaxed())
        do_discard = true;

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    if (do_discard)
    {
        MTL_TRACE("  discard-frame %u", ++frame_n);

        for (unsigned i = 0; i != _Metal_Frame.back().pass.size(); ++i)
        {
            RenderPassMetal_t* rp = RenderPassPool::Get(_Metal_Frame.back().pass[i]);

            for (unsigned b = 0; b != rp->cmdBuf.size(); ++b)
            {
                Handle cbh = rp->cmdBuf[b];
                CommandBufferMetal_t* cb = CommandBufferPool::Get(cbh);

                cb->buf = nil;
                [cb->encoder release];
                cb->encoder = nil;
                cb->rt = nil;

                CommandBufferPool::Free(cbh);
            }

            rp->desc = nullptr;

            [rp->buf release];
            rp->buf = nil;
            [rp->encoder release];
            rp->encoder = nil;

            rp->cmdBuf.clear();
        }

        [_Metal_Frame.back().drawable release];
        _Metal_Frame.back().drawable = nil;
        _Metal_NewFramePending = true;
        _Metal_Frame.clear();

        if (syncObject != InvalidHandle)
        {
            SyncObjectMetal_t* sync = SyncObjectPool::Get(syncObject);
            sync->is_signaled = true;
        }

        return;
    }

#else

    if (!do_discard)
    {
        for (unsigned i = 0; i != _Metal_Frame.back().pass.size(); ++i)
        {
            RenderPassMetal_t* rp = RenderPassPool::Get(_Metal_Frame.back().pass[i]);

            if (!rp->Initialize())
            {
                do_discard = true;
                break;
            }
        }
    }

    if (do_discard)
    {
        if (syncObject != InvalidHandle)
        {
            SyncObjectMetal_t* sync = SyncObjectPool::Get(syncObject);
            sync->is_signaled = true;
        }

        return;
    }
    
#endif

    static std::vector<RenderPassMetal_t*> pass;

    // sort cmd-lists by priority

    pass.clear();
    for (unsigned i = 0; i != _Metal_Frame.back().pass.size(); ++i)
    {
        RenderPassMetal_t* rp = RenderPassPool::Get(_Metal_Frame.back().pass[i]);
        bool do_add = true;

        for (std::vector<RenderPassMetal_t *>::iterator p = pass.begin(), p_end = pass.end(); p != p_end; ++p)
        {
            if (rp->priority > (*p)->priority)
            {
                pass.insert(p, 1, rp);
                do_add = false;
                break;
            }
        }

        if (do_add)
            pass.push_back(rp);
    }

    id<MTLCommandBuffer> pbuf;

    for (std::vector<RenderPassMetal_t *>::iterator p = pass.begin(), p_end = pass.end(); p != p_end; ++p)
    {
        RenderPassMetal_t* pass = *p;

        if (pass->do_present)
            pbuf = pass->buf;
    }
    if (pass.size())
    {
        MTL_TRACE("  -mtl.present-drawable %p", (void*)(_Metal_Frame.back().drawable));
        MTL_TRACE("   drawable= %p %i %s", (void*)(_Metal_Frame.back().drawable), [_Metal_Frame.back().drawable retainCount], NSStringFromClass([_Metal_Frame.back().drawable class]).UTF8String);
        [pbuf presentDrawable:_Metal_Frame.back().drawable];

        unsigned f = frame_n;
        [pbuf addCompletedHandler:^(id<MTLCommandBuffer> cb) {
          MTL_TRACE("  .frame %u complete", f);
        }];
    }

    if (pass.size() && pass.back()->cmdBuf.size())
    {
        Handle last_cb_h = pass.back()->cmdBuf.back();
        CommandBufferMetal_t* last_cb = CommandBufferPool::Get(last_cb_h);
        id<MTLTexture> back_buf = _Metal_DefFrameBuf;

        [last_cb->buf addCompletedHandler:^(id<MTLCommandBuffer> cmdb)
                                          {
                                            if (syncObject != InvalidHandle)
                                            {
                                                SyncObjectMetal_t* sync = SyncObjectPool::Get(syncObject);

                                                sync->is_signaled = true;
                                            }

                                          }];
    }

    for (std::vector<RenderPassMetal_t *>::iterator p = pass.begin(), p_end = pass.end(); p != p_end; ++p)
    {
        RenderPassMetal_t* rp = *p;

        #if !RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
        for (unsigned b = 0; b != rp->cmdBuf.size(); ++b)
        {
            Handle cbh = rp->cmdBuf[b];
            CommandBufferMetal_t* cb = CommandBufferPool::Get(cbh);

            cb->Execute();
        }
        #endif
  
        #if !RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
        if (rp->encoder)
            [rp->encoder endEncoding];
        #endif

        for (unsigned b = 0; b != rp->cmdBuf.size(); ++b)
        {
            Handle cbh = rp->cmdBuf[b];
            CommandBufferMetal_t* cb = CommandBufferPool::Get(cbh);

            cb->buf = nil;
            [cb->encoder release];
            cb->encoder = nil;
            cb->rt = nil;

            CommandBufferPool::Free(cbh);
        }

        #if !RHI_METAL__COMMIT_COMMAND_BUFFER_ON_END
        MTL_TRACE("  .commit %u   %p", (p - pass.begin()), (void*)(rp->buf));
        [rp->buf commit];
        #endif

        rp->desc = nullptr;

        [rp->buf release];
        rp->buf = nil;
        [rp->encoder release];
        rp->encoder = nil;

        rp->cmdBuf.clear();
    }

    for (unsigned i = 0; i != _Metal_Frame.back().pass.size(); ++i)
        RenderPassPool::Free(_Metal_Frame.back().pass[i]);

    ConstBufferMetal::InvalidateAllInstances();
    ConstBufferMetal::ResetRingBuffer();

    [_Metal_Frame.back().drawable release];
    _Metal_Frame.back().drawable = nil;
    _Metal_NewFramePending = true;
    _Metal_Frame.clear();

    _Metal_DefFrameBuf = nil;
}

namespace CommandBufferMetal
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_CommandBuffer_Begin = &metal_CommandBuffer_Begin;
    dispatch->impl_CommandBuffer_End = &metal_CommandBuffer_End;
    dispatch->impl_CommandBuffer_SetPipelineState = &metal_CommandBuffer_SetPipelineState;
    dispatch->impl_CommandBuffer_SetCullMode = &metal_CommandBuffer_SetCullMode;
    dispatch->impl_CommandBuffer_SetScissorRect = &metal_CommandBuffer_SetScissorRect;
    dispatch->impl_CommandBuffer_SetViewport = &metal_CommandBuffer_SetViewport;
    dispatch->impl_CommandBuffer_SetFillMode = &metal_CommandBuffer_SetFillMode;
    dispatch->impl_CommandBuffer_SetVertexData = &metal_CommandBuffer_SetVertexData;
    dispatch->impl_CommandBuffer_SetVertexConstBuffer = &metal_CommandBuffer_SetVertexConstBuffer;
    dispatch->impl_CommandBuffer_SetVertexTexture = &metal_CommandBuffer_SetVertexTexture;
    dispatch->impl_CommandBuffer_SetIndices = &metal_CommandBuffer_SetIndices;
    dispatch->impl_CommandBuffer_SetQueryBuffer = &metal_CommandBuffer_SetQueryBuffer;
    dispatch->impl_CommandBuffer_SetQueryIndex = &metal_CommandBuffer_SetQueryIndex;
    dispatch->impl_CommandBuffer_SetFragmentConstBuffer = &metal_CommandBuffer_SetFragmentConstBuffer;
    dispatch->impl_CommandBuffer_SetFragmentTexture = &metal_CommandBuffer_SetFragmentTexture;
    dispatch->impl_CommandBuffer_SetDepthStencilState = &metal_CommandBuffer_SetDepthStencilState;
    dispatch->impl_CommandBuffer_SetSamplerState = &metal_CommandBuffer_SetSamplerState;
    dispatch->impl_CommandBuffer_DrawPrimitive = &metal_CommandBuffer_DrawPrimitive;
    dispatch->impl_CommandBuffer_DrawIndexedPrimitive = &metal_CommandBuffer_DrawIndexedPrimitive;
    dispatch->impl_CommandBuffer_DrawInstancedPrimitive = &metal_CommandBuffer_DrawInstancedPrimitive;
    dispatch->impl_CommandBuffer_DrawInstancedIndexedPrimitive = &metal_CommandBuffer_DrawInstancedIndexedPrimitive;
    dispatch->impl_CommandBuffer_SetMarker = &metal_CommandBuffer_SetMarker;

    dispatch->impl_SyncObject_Create = &metal_SyncObject_Create;
    dispatch->impl_SyncObject_Delete = &metal_SyncObject_Delete;
    dispatch->impl_SyncObject_IsSignaled = &metal_SyncObject_IsSignaled;

    dispatch->impl_Present = &metal_Present;
}
}

} // namespace rhi

#endif //#if !(TARGET_IPHONE_SIMULATOR==1)
