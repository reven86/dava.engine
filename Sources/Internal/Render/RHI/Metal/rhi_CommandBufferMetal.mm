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
struct
RenderPassMetal_t
{
    MTLRenderPassDescriptor* desc;
    id<MTLCommandBuffer> buf;
    id<MTLParallelRenderCommandEncoder> encoder;
    id<MTLCommandBuffer> blit_buf;
    id<MTLBlitCommandEncoder> blit_encoder;
    std::vector<Handle> cmdBuf;
    int priority;
    uint32 do_present:1;
    uint32 finished : 1;
};

struct
CommandBufferMetal_t
{
    id<MTLRenderCommandEncoder> encoder;
    id<MTLCommandBuffer> buf;

    id<MTLTexture> rt;
    bool ds_used;
    Handle cur_ib;
    unsigned cur_vstream_count;
    Handle cur_vb[MAX_VERTEX_STREAM_COUNT];
    uint32 cur_stride;

    void _ApplyVertexData( unsigned firstVertex=0 );
};

struct
FrameMetal_t
{
    std::vector<Handle> pass;
    id<CAMetalDrawable> drawable;
};

void CommandBufferMetal_t::_ApplyVertexData( unsigned firstVertex )
{
   for (unsigned s = 0; s != cur_vstream_count; ++s)
    {
        unsigned base = 0;
        id<MTLBuffer> vb = VertexBufferMetal::GetBuffer(cur_vb[s], &base);
        unsigned off = (s==0) ? firstVertex*cur_stride : 0;

        [encoder setVertexBuffer:vb offset:base + off atIndex:s];
    }
}

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
static id<MTLTexture> _ScreenshotTexture = nil;

static std::vector<FrameMetal_t> _Metal_Frame;
static bool _Metal_NewFramePending = true;

//------------------------------------------------------------------------------

static Handle
metal_RenderPass_Allocate(const RenderPassConfig& passConf, uint32 cmdBufCount, Handle* cmdBuf)
{
    bool suspended = false;

    _Metal_ScreenshotCallbackSync.Lock();
    suspended = _Metal_Suspended;
    _Metal_ScreenshotCallbackSync.Unlock();

    if (suspended)
    {
        for (unsigned i = 0; i != cmdBufCount; ++i)
            cmdBuf[i] = InvalidHandle;

        MTL_TRACE("-rp.alloc InvalidHande (suspended)");
        return InvalidHandle;
    }

    DVASSERT(cmdBufCount);

    if (_Metal_NewFramePending)
    {
        MTL_TRACE("--- next-frame");
        FrameMetal_t f;

        f.drawable = [_Metal_Layer nextDrawable];
        [f.drawable retain];
        MTL_TRACE(" next.drawable= %p %i %s", (void*)(f.drawable), [f.drawable retainCount], NSStringFromClass([f.drawable class]).UTF8String);
        [f.drawable retain]; // this is workaround, see Present()
        _Metal_DefFrameBuf = f.drawable.texture;

        _Metal_Frame.push_back(f);
        _Metal_NewFramePending = false;
    }

    if (!_Metal_Frame.back().drawable)
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
        cb->ds_used = ds_used;
        cb->cur_ib = InvalidHandle;
        cb->cur_vstream_count = 0;
        for (unsigned s = 0; s != countof(cb->cur_vb); ++s)
            cb->cur_vb[s] = InvalidHandle;

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
            cb->ds_used = ds_used;
            cb->cur_ib = InvalidHandle;
            cb->cur_vstream_count = 0;
            for (unsigned s = 0; s != countof(cb->cur_vb); ++s)
                cb->cur_vb[s] = InvalidHandle;

            pass->cmdBuf[i] = cb_h;
            cmdBuf[i] = cb_h;

            if (i == 0 && pass->do_present)
                pbuf = cb->buf;
        }
    }

    //    pass->blit_buf = [_Metal_DefCmdQueue commandBufferWithUnretainedReferences];
    pass->blit_buf = [_Metal_DefCmdQueue commandBuffer];
    [pass->blit_buf retain];
    pass->blit_encoder = [pass->blit_buf blitCommandEncoder];
    [pass->blit_encoder retain];

    return pass_h;
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

    if (pass->cmdBuf.size() > 1)
    {
        [pass->encoder endEncoding];
    }
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

    [cb->encoder setDepthStencilState:_Metal_DefDepthState];
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_End(Handle cmdBuf, Handle syncObject)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

    [cb->encoder endEncoding];

    if (syncObject != InvalidHandle)
    {
        [cb->buf addCompletedHandler:^(id<MTLCommandBuffer> cmdb) {
          SyncObjectMetal_t* sync = SyncObjectPool::Get(syncObject);

          sync->is_signaled = true;
        }];
    }
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetPipelineState(Handle cmdBuf, Handle ps, uint32 layoutUID)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

    cb->cur_stride = PipelineStateMetal::SetToRHI(ps, layoutUID, cb->ds_used, cb->encoder);
    cb->cur_vstream_count = PipelineStateMetal::VertexStreamCount(ps);
    StatSet::IncStat(stat_SET_PS, 1);
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetCullMode(Handle cmdBuf, CullMode mode)
{
    id<MTLRenderCommandEncoder> encoder = CommandBufferPool::Get(cmdBuf)->encoder;

    switch (mode)
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

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetScissorRect(Handle cmdBuf, ScissorRect rect)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);
    id<MTLRenderCommandEncoder> encoder = cb->encoder;
    MTLScissorRect rc;

    if (!(rect.x == 0 && rect.y == 0 && rect.width == 0 && rect.height == 0))
    {
        unsigned max_x = (cb->rt)  ? unsigned(cb->rt.width-1)  :  unsigned(_Metal_DefFrameBuf.width-1);
        unsigned max_y = (cb->rt)  ? unsigned(cb->rt.height-1)  :  unsigned(_Metal_DefFrameBuf.height-1);

        rc.x = rect.x;
        rc.y = rect.y;
        rc.width = (rect.x+rect.width > max_x) ? (max_x-rc.x) : rect.width;
        rc.height = (rect.y+rect.height > max_y) ? (max_y-rc.y) : rect.height;

        if( rc.width == 0 )
        {
            rc.width = 1;
            if( rc.x > 0 )
                --rc.x;
        }

        if( rc.height == 0 )
        {
            rc.height = 1;
            if( rc.y > 0 )
                --rc.y;
        }
    }
    else
    {
        rc.x = 0;
        rc.y = 0;
        if( cb->rt )
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

    [encoder setScissorRect:rc];
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetViewport(Handle cmdBuf, Viewport viewport)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);
    id<MTLRenderCommandEncoder> encoder = cb->encoder;
    MTLViewport vp;

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

    [encoder setViewport:vp];
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetFillMode(Handle cmdBuf, FillMode mode)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);
    id<MTLRenderCommandEncoder> encoder = cb->encoder;

    [encoder setTriangleFillMode:(mode == FILLMODE_WIREFRAME) ? MTLTriangleFillModeLines : MTLTriangleFillModeFill];
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetVertexData(Handle cmdBuf, Handle vb, uint32 streamIndex)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

    cb->cur_vb[streamIndex] = vb;

    StatSet::IncStat(stat_SET_VB, 1);
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetVertexConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);

    ConstBufferMetal::SetToRHI(buffer, bufIndex, cb->encoder);
    StatSet::IncStat(stat_SET_CB, 1);
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetVertexTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

    TextureMetal::SetToRHIVertex(tex, unitIndex, cb->encoder);
    StatSet::IncStat(stat_SET_TEX, 1);
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetIndices(Handle cmdBuf, Handle ib)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

    cb->cur_ib = ib;

    StatSet::IncStat(stat_SET_IB, 1);
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetQueryIndex(Handle cmdBuf, uint32 objectIndex)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

    if (objectIndex != DAVA::InvalidIndex)
    {
        [cb->encoder setVisibilityResultMode:MTLVisibilityResultModeBoolean offset:objectIndex * QueryBUfferElemeentAlign];
    }
    else
    {
        [cb->encoder setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:0];
    }
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

    ConstBufferMetal::SetToRHI(buffer, bufIndex, cb->encoder);
    StatSet::IncStat(stat_SET_CB, 1);
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetFragmentTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

    TextureMetal::SetToRHIFragment(tex, unitIndex, cb->encoder);
    StatSet::IncStat(stat_SET_TEX, 1);
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetDepthStencilState(Handle cmdBuf, Handle depthStencilState)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

    DepthStencilStateMetal::SetToRHI(depthStencilState, cb->encoder);
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetSamplerState(Handle cmdBuf, const Handle samplerState)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);

    SamplerStateMetal::SetToRHI(samplerState, cb->encoder);
    StatSet::IncStat(stat_SET_SS, 1);
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
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_DrawIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count, uint32 /*vertexCount*/, uint32 firstVertex, uint32 startIndex)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);
    MTLPrimitiveType ptype = MTLPrimitiveTypeTriangle;
    unsigned i_cnt = 0;
    unsigned ib_base = 0;
    id<MTLBuffer> ib = IndexBufferMetal::GetBuffer(cb->cur_ib, &ib_base);
    MTLIndexType i_type = IndexBufferMetal::GetType(cb->cur_ib);
    unsigned i_off = (i_type == MTLIndexTypeUInt16) ? startIndex * sizeof(uint16) : startIndex * sizeof(uint32);

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

    cb->_ApplyVertexData( firstVertex );
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
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_DrawInstancedIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 inst_count, uint32 prim_count, uint32 /*vertexCount*/, uint32 firstVertex, uint32 startIndex, uint32 baseInst)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);
    MTLPrimitiveType ptype = MTLPrimitiveTypeTriangle;
    unsigned i_cnt = 0;
    unsigned ib_base = 0;
    id<MTLBuffer> ib = IndexBufferMetal::GetBuffer(cb->cur_ib, &ib_base);
    MTLIndexType i_type = IndexBufferMetal::GetType(cb->cur_ib);
    unsigned i_off = (i_type == MTLIndexTypeUInt16) ? startIndex * sizeof(uint16) : startIndex * sizeof(uint32);

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

    cb->_ApplyVertexData( firstVertex );
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
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetMarker(Handle cmdBuf, const char* text)
{
    CommandBufferMetal_t* cb = CommandBufferPool::Get(cmdBuf);
    NSString* txt = [[NSString alloc] initWithUTF8String:text];

    [cb->encoder insertDebugSignpost:txt];

    [txt release];
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
    SyncObjectMetal_t* sync = SyncObjectPool::Get(obj);

    if (sync)
        signaled = sync->is_signaled;

    return signaled;
}

//------------------------------------------------------------------------------

static void
metal_Present(Handle syncObject)
{
    static unsigned frame_n = 0;
    MTL_TRACE("--present %u", ++frame_n);
    SCOPED_NAMED_TIMING("rhi.draw-present");

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

    bool do_discard = false;

    _Metal_ScreenshotCallbackSync.Lock();
    if (_Metal_Suspended)
        do_discard = true;
    _Metal_ScreenshotCallbackSync.Unlock();

    if ([_Metal_Frame.back().drawable retainCount] < 3)
        do_discard = true; // this is workaroud against drawable/cmd.buf being de-allocated in certain cases

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

            [rp->blit_encoder endEncoding];

            [rp->buf release];
            rp->buf = nil;
            [rp->encoder release];
            rp->encoder = nil;

            [rp->blit_encoder release];
            rp->blit_encoder = nil;
            [rp->blit_buf release];
            rp->blit_buf = nil;

            rp->cmdBuf.clear();
        }

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
        
        if( pass->do_present )
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

        _Metal_ScreenshotCallbackSync.Lock();
        {
            if (_Metal_PendingScreenshotCallback)
            {
                if (!_ScreenshotTexture)
                {
                    MTLPixelFormat pf = MTLPixelFormatBGRA8Unorm;
                    MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pf width:_Metal_DefFrameBuf.width height:_Metal_DefFrameBuf.height mipmapped:NO];

                    desc.textureType = MTLTextureType2D;
                    desc.mipmapLevelCount = 1;
                    desc.sampleCount = 1;

                    _ScreenshotTexture = [_Metal_Device newTextureWithDescriptor:desc];
                }

                if (!_Metal_ScreenshotData)
                {
                    MTLOrigin org;
                    MTLSize sz;
                    org.x = 0;
                    org.y = 0;
                    org.z = 0;
                    sz.width = _Metal_DefFrameBuf.width;
                    sz.height = _Metal_DefFrameBuf.height;
                    sz.depth = 1;
                    [pass.back()->blit_encoder copyFromTexture:back_buf sourceSlice:0 sourceLevel:0 sourceOrigin:org sourceSize:sz toTexture:_ScreenshotTexture destinationSlice:0 destinationLevel:0 destinationOrigin:org];
                }
            }
        }
        _Metal_ScreenshotCallbackSync.Unlock();

        [last_cb->buf addCompletedHandler:^(id<MTLCommandBuffer> cmdb)
                                          {
                                            if (syncObject != InvalidHandle)
                                            {
                                                SyncObjectMetal_t* sync = SyncObjectPool::Get(syncObject);

                                                sync->is_signaled = true;
                                            }

                                          }];

        _Metal_ScreenshotCallbackSync.Lock();
        if (_Metal_PendingScreenshotCallback)
        {
            [pass.back()
             ->blit_buf addCompletedHandler:^(id<MTLCommandBuffer> cmdb)
                                            {
                                              _Metal_ScreenshotCallbackSync.Lock();
                                              if (_Metal_PendingScreenshotCallback)
                                              {
                                                  uint32 stride = _Metal_DefFrameBuf.width * sizeof(uint32);
                                                  uint32 sz = stride * _Metal_DefFrameBuf.height;
                                                  _Metal_ScreenshotData = ::malloc(sz);
                                                  MTLRegion rgn;

                                                  rgn.origin.x = 0;
                                                  rgn.origin.y = 0;
                                                  rgn.origin.z = 0;
                                                  rgn.size.width = _Metal_DefFrameBuf.width;
                                                  rgn.size.height = _Metal_DefFrameBuf.height;
                                                  rgn.size.depth = 1;

                                                  [_ScreenshotTexture getBytes:_Metal_ScreenshotData bytesPerRow:stride bytesPerImage:sz fromRegion:rgn mipmapLevel:0 slice:0];
                                                  for (uint8 *p = (uint8 *)_Metal_ScreenshotData, *p_end = (uint8 *)_Metal_ScreenshotData + _Metal_DefFrameBuf.width * _Metal_DefFrameBuf.height * 4; p != p_end; p += 4)
                                                  {
                                                      uint8 tmp = p[0];
                                                      p[0] = p[2];
                                                      p[2] = tmp;
                                                      p[3] = 0xFF;
                                                  }

                                                  (*_Metal_PendingScreenshotCallback)(_Metal_DefFrameBuf.width, _Metal_DefFrameBuf.height, _Metal_ScreenshotData);
                                                  ::free(_Metal_ScreenshotData);
                                                  _Metal_PendingScreenshotCallback = nullptr;
                                                  _Metal_ScreenshotData = nullptr;
                                                  [_ScreenshotTexture setPurgeableState:MTLPurgeableStateEmpty];
                                                  _ScreenshotTexture = nil;
                                              }

                                              _Metal_ScreenshotCallbackSync.Unlock();
                                            }];
        };
        _Metal_ScreenshotCallbackSync.Unlock();
    }

    for (std::vector<RenderPassMetal_t *>::iterator p = pass.begin(), p_end = pass.end(); p != p_end; ++p)
    {
        RenderPassMetal_t* rp = *p;

        MTL_TRACE("  .commit %u   %p", (p - pass.begin()), (void*)(rp->buf));
        [rp->buf commit];
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

        rp->desc.colorAttachments[0].texture = nil;
        rp->desc.depthAttachment.texture = nil;
        rp->desc = nullptr;

        [rp->blit_encoder endEncoding];
        [rp->blit_buf commit];

        [rp->buf release];
        rp->buf = nil;
        [rp->encoder release];
        rp->encoder = nil;

        [rp->blit_encoder release];
        rp->blit_encoder = nil;
        [rp->blit_buf release];
        rp->blit_buf = nil;

        rp->cmdBuf.clear();
    }

    for (unsigned i = 0; i != _Metal_Frame.back().pass.size(); ++i)
        RenderPassPool::Free(_Metal_Frame.back().pass[i]);

    ConstBufferMetal::InvalidateAllInstances();
    ConstBufferMetal::ResetRingBuffer();

    [_Metal_Frame.back().drawable release]; // this additional 'release' is due to workaround
    [_Metal_Frame.back().drawable release];
    _Metal_Frame.back().drawable = nil;
    _Metal_NewFramePending = true;
    _Metal_Frame.clear();
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
