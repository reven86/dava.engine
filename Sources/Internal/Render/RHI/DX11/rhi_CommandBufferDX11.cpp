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

    #include "../Common/rhi_Pool.h"
    #include "rhi_DX11.h"

    #include "../rhi_Type.h"
    #include "../Common/rhi_RingBuffer.h"
    #include "../Common/dbg_StatSet.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
using DAVA::Logger;
    #include "Core/Core.h"
    #include "Debug/Profiler.h"
    #include "Concurrency/Thread.h"
    #include "Concurrency/Semaphore.h"

    #include "_dx11.h"
namespace rhi
{
extern void _InitDX11();
}
    #include <vector>

namespace rhi
{
//==============================================================================

#if !RHI_DX11__USE_DEFERRED_CONTEXTS
enum CommandDX11Type
{
    DX11__BEGIN,
    DX11__END,

    DX11__SET_VERTEX_DATA,
    DX11__SET_INDICES,
    DX11__SET_QUERY_BUFFER,
    DX11__SET_QUERY_INDEX,
    DX11__ISSUE_TIMESTAMP_QUERY,

    DX11__SET_PIPELINE_STATE,
    DX11__SET_CULL_MODE,
    DX11__SET_SCISSOR_RECT,
    DX11__SET_VIEWPORT,
    DX11__SET_FILLMODE,
    DX11__SET_VERTEX_PROG_CONST_BUFFER,
    DX11__SET_FRAGMENT_PROG_CONST_BUFFER,
    DX11__SET_FRAGMENT_TEXTURE,
    DX11__SET_VERTEX_TEXTURE,

    DX11__SET_DEPTHSTENCIL_STATE,
    DX11__SET_SAMPLER_STATE,

    DX11__DRAW_PRIMITIVE,
    DX11__DRAW_INDEXED_PRIMITIVE,

    DX11__DEBUG_MARKER,

    DX11__NOP
};

struct
CommandDX11
{
    uint8 type;
    uint8 size;

    CommandDX11(uint8 t, uint8 sz)
        : type(t)
        , size(sz)
    {
    }
};

template <class T, CommandDX11Type t>
struct
CommandDX11Impl
: public CommandDX11
{
    CommandDX11Impl()
        : CommandDX11(t, sizeof(T))
    {
    }
};

struct
CommandDX11_Begin : public CommandDX11Impl<CommandDX11_Begin, DX11__BEGIN>
{
};

struct
CommandDX11_End : public CommandDX11Impl<CommandDX11_End, DX11__END>
{
    Handle syncObject;
};

struct
CommandDX11_SetVertexData : public CommandDX11Impl<CommandDX11_SetVertexData, DX11__SET_VERTEX_DATA>
{
    uint16 streamIndex;
    Handle vb;
};

struct
CommandDX11_SetIndices : public CommandDX11Impl<CommandDX11_SetIndices, DX11__SET_INDICES>
{
    Handle ib;
};

struct
CommandDX11_SetQueryBuffer : public CommandDX11Impl<CommandDX11_SetQueryBuffer, DX11__SET_QUERY_BUFFER>
{
    Handle queryBuf;
};

struct
CommandDX11_SetQueryIndex : public CommandDX11Impl<CommandDX11_SetQueryIndex, DX11__SET_QUERY_INDEX>
{
    uint32 objectIndex;
};

struct
CommandDX11_SetPipelineState : public CommandDX11Impl<CommandDX11_SetPipelineState, DX11__SET_PIPELINE_STATE>
{
    Handle ps;
    uint32 vdeclUID;
};

struct
CommandDX11_SetDepthStencilState : public CommandDX11Impl<CommandDX11_SetDepthStencilState, DX11__SET_DEPTHSTENCIL_STATE>
{
    Handle depthStencilState;
};

struct
CommandDX11_SetSamplerState : public CommandDX11Impl<CommandDX11_SetSamplerState, DX11__SET_SAMPLER_STATE>
{
    Handle samplerState;
};

struct
CommandDX11_SetCullMode : public CommandDX11Impl<CommandDX11_SetCullMode, DX11__SET_CULL_MODE>
{
    uint8 mode;
};

struct
CommandDX11_SetScissorRect : public CommandDX11Impl<CommandDX11_SetScissorRect, DX11__SET_SCISSOR_RECT>
{
    uint16 x, y, w, h;
};

struct
CommandDX11_SetViewport : public CommandDX11Impl<CommandDX11_SetViewport, DX11__SET_VIEWPORT>
{
    uint16 x, y, w, h;
};

struct
CommandDX11_SetFillMode : public CommandDX11Impl<CommandDX11_SetFillMode, DX11__SET_FILLMODE>
{
    uint8 mode;
};

struct
CommandDX11_SetVertexProgConstBuffer : public CommandDX11Impl<CommandDX11_SetVertexProgConstBuffer, DX11__SET_VERTEX_PROG_CONST_BUFFER>
{
    uint16 bufIndex;
    Handle buffer;
    const void* inst;
};

struct
CommandDX11_SetFragmentProgConstBuffer : public CommandDX11Impl<CommandDX11_SetFragmentProgConstBuffer, DX11__SET_FRAGMENT_PROG_CONST_BUFFER>
{
    uint16 bufIndex;
    Handle buffer;
    const void* inst;
};

struct
CommandDX11_SetFragmentTexture : public CommandDX11Impl<CommandDX11_SetFragmentTexture, DX11__SET_FRAGMENT_TEXTURE>
{
    uint16 unitIndex;
    Handle tex;
};

struct
CommandDX11_SetVertexTexture : public CommandDX11Impl<CommandDX11_SetVertexTexture, DX11__SET_VERTEX_TEXTURE>
{
    uint16 unitIndex;
    Handle tex;
};

struct
CommandDX11_DrawPrimitive : public CommandDX11Impl<CommandDX11_DrawPrimitive, DX11__DRAW_PRIMITIVE>
{
    uint8 topo;
    uint32 vertexCount;
    uint32 baseVertex;
};

struct
CommandDX11_DrawIndexedPrimitive : public CommandDX11Impl<CommandDX11_DrawIndexedPrimitive, DX11__DRAW_INDEXED_PRIMITIVE>
{
    uint8 topo;
    uint32 indexCount;
    uint32 vertexCount;
    uint32 baseVertex;
    uint32 startIndex;
};

struct
CommandDX11_SetMarker : public CommandDX11Impl<CommandDX11_SetMarker, DX11__DEBUG_MARKER>
{
};

struct
CommandDX11_IssueTimestamptQuery : public CommandDX11Impl<CommandDX11_IssueTimestamptQuery, DX11__SET_QUERY_BUFFER>
{
    Handle querySet;
    uint32 timestampIndex;
};


#endif

//==============================================================================

struct
RasterizerParamDX11
{
    uint32 cullMode : 3;
    uint32 scissorEnabled : 1;
    uint32 wireframe : 1;

    bool operator==(const RasterizerParamDX11& b) const
    {
        return this->cullMode == b.cullMode && this->scissorEnabled == b.scissorEnabled && this->wireframe == b.wireframe;
    }
};

struct
RasterizerStateDX11
{
    RasterizerParamDX11 param;
    ID3D11RasterizerState* state;
};

//==============================================================================

class
CommandBufferDX11_t
{
public:
    CommandBufferDX11_t();
    ~CommandBufferDX11_t();

    void Begin(ID3D11DeviceContext* context);
    void Execute();

    RenderPassConfig passCfg;
    uint32 isFirstInPass : 1;
    uint32 isLastInPass : 1;

    D3D11_PRIMITIVE_TOPOLOGY cur_topo;
    Handle cur_ib;
    Handle cur_vb;
    Handle cur_vb_stride;
    Handle cur_pipelinestate;
    uint32 cur_stride;
    Handle cur_query_buf;
    uint32 cur_query_i;
    D3D11_VIEWPORT def_viewport;
    RasterizerParamDX11 rs_param;
    ID3D11RasterizerState* cur_rs;


#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ID3D11DeviceContext* context;
    ID3DUserDefinedAnnotation* contextAnnotation;
    ID3D11CommandList* commandList;
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

    Handle sync;
};

class
RenderPassDX11_t
{
public:
    std::vector<Handle> cmdBuf;
    int priority;
    uint32 perfQueryIndex0;
    uint32 perfQueryIndex1;
};

struct
SyncObjectDX11_t
{
    uint32 frame;
    uint32 is_signaled : 1;
    uint32 is_used : 1;
};

typedef ResourcePool<CommandBufferDX11_t, RESOURCE_COMMAND_BUFFER, CommandBuffer::Descriptor, false> CommandBufferPool;
typedef ResourcePool<RenderPassDX11_t, RESOURCE_RENDER_PASS, RenderPassConfig, false> RenderPassPool;
typedef ResourcePool<SyncObjectDX11_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false> SyncObjectPool;

RHI_IMPL_POOL(CommandBufferDX11_t, RESOURCE_COMMAND_BUFFER, CommandBuffer::Descriptor, false);
RHI_IMPL_POOL(RenderPassDX11_t, RESOURCE_RENDER_PASS, RenderPassConfig, false);
RHI_IMPL_POOL(SyncObjectDX11_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false);

struct
FrameDX11
{
    unsigned number;
    Handle sync;
    Handle perfQuerySet;
    std::vector<Handle> pass;
    uint32 readyToExecute : 1;

    ID3D11CommandList* cmdList;
};

static std::vector<FrameDX11> _Frame;
static bool _FrameStarted = false;
static unsigned _FrameNumber = 1;
//static DAVA::Spinlock       _FrameSync;
static DAVA::Mutex _FrameSync;

static void _ExecuteQueuedCommandsDX11();

static DAVA::Thread* _DX11_RenderThread = nullptr;
static unsigned _DX11_RenderThreadFrameCount = 0;
static bool _DX11_RenderThreadExitPending = false;
static DAVA::Spinlock _DX11_RenderThreadExitSync;
static DAVA::Semaphore _DX11_RenderThreadStartedSync(0);

static DX11Command* _DX11_PendingImmediateCmd = nullptr;
static uint32 _DX11_PendingImmediateCmdCount = 0;
static DAVA::Mutex _DX11_PendingImmediateCmdSync;

static bool _DX11_PerfQuerySetPending = false;

//------------------------------------------------------------------------------

static std::vector<RasterizerStateDX11> _RasterizerStateDX11;

static ID3D11RasterizerState*
_GetRasterizerState(RasterizerParamDX11 param)
{
    ID3D11RasterizerState* state = nullptr;

    for (std::vector<RasterizerStateDX11>::iterator s = _RasterizerStateDX11.begin(), s_end = _RasterizerStateDX11.end(); s != s_end; ++s)
    {
        if (s->param == param)
        {
            state = s->state;
            break;
        }
    }

    if (!state)
    {
        D3D11_RASTERIZER_DESC desc;
        HRESULT hr;

        desc.FillMode = (param.wireframe) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
        desc.FrontCounterClockwise = FALSE;

        switch (CullMode(param.cullMode))
        {
        case CULL_NONE:
            desc.CullMode = D3D11_CULL_NONE;
            break;
        case CULL_CCW:
            desc.CullMode = D3D11_CULL_BACK;
            break;
        case CULL_CW:
            desc.CullMode = D3D11_CULL_FRONT;
            break;
        }

        desc.DepthBias = 0;
        desc.DepthBiasClamp = 0;
        desc.SlopeScaledDepthBias = 0.0f;
        desc.DepthClipEnable = TRUE;
        desc.ScissorEnable = param.scissorEnabled;
        desc.MultisampleEnable = FALSE;
        desc.AntialiasedLineEnable = FALSE;

        hr = _D3D11_Device->CreateRasterizerState(&desc, &state);
        if (SUCCEEDED(hr))
        {
            RasterizerStateDX11 s;

            s.param = param;
            s.state = state;
            _RasterizerStateDX11.push_back(s);
        }
        else
        {
            state = nullptr;
        }
    }

    return state;
}

//------------------------------------------------------------------------------

static Handle
dx11_RenderPass_Allocate(const RenderPassConfig& passDesc, uint32 cmdBufCount, Handle* cmdBuf)
{
    DVASSERT(cmdBufCount);

    Handle handle = RenderPassPool::Alloc();
    RenderPassDX11_t* pass = RenderPassPool::Get(handle);

    pass->cmdBuf.resize(cmdBufCount);
    pass->priority = passDesc.priority;
    pass->perfQueryIndex0 = passDesc.PerfQueryIndex0;
    pass->perfQueryIndex1 = passDesc.PerfQueryIndex1;

    for (unsigned i = 0; i != cmdBufCount; ++i)
    {
        Handle h = CommandBufferPool::Alloc();
        CommandBufferDX11_t* cb = CommandBufferPool::Get(h);

        #if RHI_DX11__USE_DEFERRED_CONTEXTS
        cb->commandList = nullptr;
        #endif
        cb->passCfg = passDesc;
        cb->isFirstInPass = i == 0;
        cb->isLastInPass = i == cmdBufCount - 1;

        #if RHI_DX11__USE_DEFERRED_CONTEXTS
        if (!cb->context)
        {
            HRESULT hr = _D3D11_Device->CreateDeferredContext(0, &(cb->context));

            if (SUCCEEDED(hr))
            {
                hr = cb->context->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)(&(cb->contextAnnotation)));
            }
        }
        #endif

        pass->cmdBuf[i] = h;
        cmdBuf[i] = h;
    }

    return handle;
}

//------------------------------------------------------------------------------

static void
dx11_RenderPass_Begin(Handle pass)
{
    _FrameSync.Lock();

    if (!_FrameStarted)
    {
        _Frame.push_back(FrameDX11());
        _Frame.back().number = _FrameNumber;
        _Frame.back().sync = rhi::InvalidHandle;
        _Frame.back().perfQuerySet = PerfQuerySetDX11::Current();
        _Frame.back().readyToExecute = false;
        _Frame.back().cmdList = nullptr;

        Trace("\n\n-------------------------------\nframe %u started\n", _FrameNumber);
        _FrameStarted = true;
        ++_FrameNumber;
#if !RHI_DX11__USE_DEFERRED_CONTEXTS
        ConstBufferDX11::InvalidateAllInstances();
#endif
    }

    _Frame.back().pass.push_back(pass);

    _FrameSync.Unlock();
}

//------------------------------------------------------------------------------

static void
dx11_RenderPass_End(Handle pass)
{
}

//------------------------------------------------------------------------------

namespace RenderPassDX11
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_Renderpass_Allocate = &dx11_RenderPass_Allocate;
    dispatch->impl_Renderpass_Begin = &dx11_RenderPass_Begin;
    dispatch->impl_Renderpass_End = &dx11_RenderPass_End;
}
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_Begin(Handle cmdBuf)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);
#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ID3D11DeviceContext* context = cb->context;
    cb->Begin(context);
#else
    cb->curUsedSize = 0;
    CommandDX11_Begin* cmd = cb->allocCmd<CommandDX11_Begin>();
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_End(Handle cmdBuf, Handle syncObject)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    cb->context->OMSetRenderTargets(0, NULL, NULL);
    cb->context->FinishCommandList(TRUE, &(cb->commandList));
    cb->sync = syncObject;
#else
    CommandDX11_End* cmd = cb->allocCmd<CommandDX11_End>();
    cmd->syncObject = syncObject;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetPipelineState(Handle cmdBuf, Handle ps, uint32 vdeclUID)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);
#if RHI_DX11__USE_DEFERRED_CONTEXTS
    const VertexLayout* vdecl = (vdeclUID == VertexLayout::InvalidUID) ? nullptr : VertexLayout::Get(vdeclUID);

    cb->cur_pipelinestate = ps;
    cb->cur_vb_stride = (vdecl) ? vdecl->Stride() : 0;

    PipelineStateDX11::SetToRHI(ps, vdeclUID, cb->context);
    StatSet::IncStat(stat_SET_PS, 1);
#else
    CommandDX11_SetPipelineState* cmd = cb->allocCmd<CommandDX11_SetPipelineState>();
    cmd->ps = ps;
    cmd->vdeclUID = vdeclUID;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetCullMode(Handle cmdBuf, CullMode mode)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    cb->rs_param.cullMode = mode;
    cb->cur_rs = nullptr;
#else
    CommandDX11_SetCullMode* cmd = cb->allocCmd<CommandDX11_SetCullMode>();
    cmd->mode = mode;
#endif
}

//------------------------------------------------------------------------------

void dx11_CommandBuffer_SetScissorRect(Handle cmdBuf, ScissorRect rect)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);
    int x = rect.x;
    int y = rect.y;
    int w = rect.width;
    int h = rect.height;

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    if (!(x == 0 && y == 0 && w == 0 && h == 0))
    {
        D3D11_RECT rect = { x, y, x + w, y + h };

        cb->rs_param.scissorEnabled = true;
        cb->cur_rs = nullptr;

        cb->context->RSSetScissorRects(1, &rect);
    }
    else
    {
        cb->rs_param.scissorEnabled = false;
        cb->cur_rs = nullptr;
    }
#else
    CommandDX11_SetScissorRect* cmd = cb->allocCmd<CommandDX11_SetScissorRect>();
    cmd->x = x;
    cmd->y = y;
    cmd->w = w;
    cmd->h = h;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetViewport(Handle cmdBuf, Viewport vp)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);
    int x = vp.x;
    int y = vp.y;
    int w = vp.width;
    int h = vp.height;

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    if (!(x == 0 && y == 0 && w == 0 && h == 0))
    {
        D3D11_VIEWPORT vp;

        vp.TopLeftX = float(x);
        vp.TopLeftY = float(y);
        vp.Width = float(w);
        vp.Height = float(h);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;

        cb->context->RSSetViewports(1, &vp);
    }
    else
    {
        cb->context->RSSetViewports(1, &(cb->def_viewport));
    }
#else
    CommandDX11_SetViewport* cmd = cb->allocCmd<CommandDX11_SetViewport>();
    cmd->x = x;
    cmd->y = y;
    cmd->w = w;
    cmd->h = h;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetFillMode(Handle cmdBuf, FillMode mode)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    cb->rs_param.wireframe = (mode == FILLMODE_WIREFRAME);
#else
    CommandDX11_SetFillMode* cmd = cb->allocCmd<CommandDX11_SetFillMode>();
    cmd->mode = mode;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetVertexData(Handle cmdBuf, Handle vb, uint32 streamIndex)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    cb->cur_vb = vb;
    cb->cur_vb_stride = (cb->cur_vb_stride) ? cb->cur_vb_stride : PipelineStateDX11::VertexLayoutStride(cb->cur_pipelinestate);
#else
    CommandDX11_SetVertexData* cmd = cb->allocCmd<CommandDX11_SetVertexData>();
    cmd->vb = vb;
    cmd->streamIndex = streamIndex;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetVertexConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ConstBufferDX11::SetToRHI(buffer, cb->context);

    StatSet::IncStat(stat_SET_CB, 1);
#else
    CommandDX11_SetVertexProgConstBuffer* cmd = cb->allocCmd<CommandDX11_SetVertexProgConstBuffer>();
    cmd->bufIndex = bufIndex;
    cmd->buffer = buffer;
    cmd->inst = ConstBufferDX11::Instance(buffer);
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetVertexTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    TextureDX11::SetToRHIVertex(tex, unitIndex, cb->context);
    StatSet::IncStat(stat_SET_TEX, 1);
#else
    CommandDX11_SetVertexTexture* cmd = cb->allocCmd<CommandDX11_SetVertexTexture>();
    cmd->unitIndex = unitIndex;
    cmd->tex = tex;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetIndices(Handle cmdBuf, Handle ib)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    cb->cur_ib = ib;
#else
    CommandDX11_SetIndices* cmd = cb->allocCmd<CommandDX11_SetIndices>();
    cmd->ib = ib;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetQueryIndex(Handle cmdBuf, uint32 objectIndex)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    cb->cur_query_i = objectIndex;
#else
    CommandDX11_SetQueryIndex* cmd = cb->allocCmd<CommandDX11_SetQueryIndex>();
    cmd->objectIndex = objectIndex;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetQueryBuffer(Handle cmdBuf, Handle queryBuf)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

    DVASSERT(cb->cur_query_buf == InvalidHandle);
#if RHI_DX11__USE_DEFERRED_CONTEXTS
    cb->cur_query_buf = queryBuf;
#else
    CommandDX11_SetQueryBuffer* cmd = cb->allocCmd<CommandDX11_SetQueryBuffer>();
    cmd->queryBuf = queryBuf;
#endif
}
static void
dx11_CommandBuffer_IssueTimestampQuery(Handle cmdBuf, Handle pqset, uint32 timestampIndex)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    PerfQuerySetDX11::IssueTimestampQuery(pqset, timestampIndex, cb->context);
#else
    CommandDX11_IssueTimestamptQuery* cmd = cb->allocCmd<CommandDX11_IssueTimestamptQuery>();
    cmd->querySet = pqset;
    cmd->timestampIndex = timestampIndex;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetFragmentConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ConstBufferDX11::SetToRHI(buffer, cb->context);
    StatSet::IncStat(stat_SET_CB, 1);
#else
    CommandDX11_SetFragmentProgConstBuffer* cmd = cb->allocCmd<CommandDX11_SetFragmentProgConstBuffer>();
    cmd->bufIndex = bufIndex;
    cmd->buffer = buffer;
    cmd->inst = ConstBufferDX11::Instance(buffer);
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetFragmentTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    TextureDX11::SetToRHIFragment(tex, unitIndex, cb->context);
    StatSet::IncStat(stat_SET_TEX, 1);
#else
    CommandDX11_SetFragmentTexture* cmd = cb->allocCmd<CommandDX11_SetFragmentTexture>();
    cmd->unitIndex = unitIndex;
    cmd->tex = tex;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetDepthStencilState(Handle cmdBuf, Handle depthStencilState)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    DepthStencilStateDX11::SetToRHI(depthStencilState, cb->context);
#else
    CommandDX11_SetDepthStencilState* cmd = cb->allocCmd<CommandDX11_SetDepthStencilState>();
    cmd->depthStencilState = depthStencilState;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetSamplerState(Handle cmdBuf, const Handle samplerState)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    SamplerStateDX11::SetToRHI(samplerState, cb->context);
    StatSet::IncStat(stat_SET_SS, 1);
#else
    CommandDX11_SetSamplerState* cmd = cb->allocCmd<CommandDX11_SetSamplerState>();
    cmd->samplerState = samplerState;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_DrawPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);
    unsigned vertexCount = 0;
    D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    switch (type)
    {
    case PRIMITIVE_TRIANGLELIST:
        vertexCount = count * 3;
        topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        vertexCount = 2 + count;
        topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        break;

    case PRIMITIVE_LINELIST:
        vertexCount = count * 2;
        topo = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        break;
    }

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ID3D11DeviceContext* ctx = cb->context;
    INT baseVertex = 0;

    if (topo != cb->cur_topo)
    {
        ctx->IASetPrimitiveTopology(topo);
        cb->cur_topo = topo;
    }

    if (!cb->cur_rs)
    {
        cb->cur_rs = _GetRasterizerState(cb->rs_param);
        ctx->RSSetState(cb->cur_rs);
    }

    VertexBufferDX11::SetToRHI(cb->cur_vb, 0, 0, cb->cur_vb_stride, ctx);

    StatSet::IncStat(stat_SET_VB, 1);

    if (cb->cur_query_i != InvalidIndex)
        QueryBufferDX11::BeginQuery(cb->cur_query_buf, cb->cur_query_i, ctx);

    ctx->Draw(vertexCount, baseVertex);

    if (cb->cur_query_i != InvalidIndex)
        QueryBufferDX11::EndQuery(cb->cur_query_buf, cb->cur_query_i, ctx);

    StatSet::IncStat(stat_DIP, 1);
    switch (topo)
    {
    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
        StatSet::IncStat(stat_DTL, 1);
        break;
    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
        StatSet::IncStat(stat_DTS, 1);
        break;
    case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
        StatSet::IncStat(stat_DLL, 1);
        break;
    default:
        break;
    }
#else
    CommandDX11_DrawPrimitive* cmd = cb->allocCmd<CommandDX11_DrawPrimitive>();
    cmd->topo = topo;
    cmd->vertexCount = vertexCount;
    cmd->baseVertex = 0;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_DrawIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count, uint32 vertexCount, uint32 firstVertex, uint32 startIndex)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);
    unsigned indexCount = 0;
    D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    switch (type)
    {
    case PRIMITIVE_TRIANGLELIST:
        indexCount = count * 3;
        topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        indexCount = 2 + count;
        topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        break;

    case PRIMITIVE_LINELIST:
        indexCount = count * 2;
        topo = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        break;
    }

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ID3D11DeviceContext* ctx = cb->context;

    if (topo != cb->cur_topo)
    {
        ctx->IASetPrimitiveTopology(topo);
        cb->cur_topo = topo;
    }

    if (!cb->cur_rs)
    {
        cb->cur_rs = _GetRasterizerState(cb->rs_param);
        ctx->RSSetState(cb->cur_rs);
    }

    IndexBufferDX11::SetToRHI(cb->cur_ib, 0, ctx);
    StatSet::IncStat(stat_SET_IB, 1);

    VertexBufferDX11::SetToRHI(cb->cur_vb, 0, 0, cb->cur_vb_stride, ctx);
    StatSet::IncStat(stat_SET_VB, 1);

    if (cb->cur_query_i != InvalidIndex)
        QueryBufferDX11::BeginQuery(cb->cur_query_buf, cb->cur_query_i, ctx);

    ctx->DrawIndexed(indexCount, startIndex, firstVertex);

    if (cb->cur_query_i != InvalidIndex)
        QueryBufferDX11::BeginQuery(cb->cur_query_buf, cb->cur_query_i, ctx);

    StatSet::IncStat(stat_DIP, 1);
    switch (topo)
    {
    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
        StatSet::IncStat(stat_DTL, 1);
        break;
    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
        StatSet::IncStat(stat_DTS, 1);
        break;
    case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
        StatSet::IncStat(stat_DLL, 1);
        break;
    default:
        break;
    }
#else
    CommandDX11_DrawIndexedPrimitive* cmd = cb->allocCmd<CommandDX11_DrawIndexedPrimitive>();
    cmd->topo = topo;
    cmd->indexCount = indexCount;
    cmd->vertexCount = vertexCount;
    cmd->baseVertex = firstVertex;
    cmd->startIndex = startIndex;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetMarker(Handle cmdBuf, const char* text)
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);
    wchar_t txt[128];

    ::MultiByteToWideChar(CP_ACP, 0, text, -1, txt, countof(txt));

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    if (cb->contextAnnotation)
    {
        cb->contextAnnotation->SetMarker(txt);
    }
    //    else
    //    {
    //        ::D3DPERF_SetMarker( D3DCOLOR_ARGB(0xFF,0x40,0x40,0x80), txt );
    //    }
#else
#endif
}

//------------------------------------------------------------------------------

static Handle
dx11_SyncObject_Create()
{
    Handle handle = SyncObjectPool::Alloc();
    SyncObjectDX11_t* sync = SyncObjectPool::Get(handle);

    sync->is_signaled = false;
    sync->is_used = false;

    return handle;
}

//------------------------------------------------------------------------------

static void
dx11_SyncObject_Delete(Handle obj)
{
    SyncObjectPool::Free(obj);
}

//------------------------------------------------------------------------------

static bool
dx11_SyncObject_IsSignaled(Handle obj)
{
    bool signaled = false;
    SyncObjectDX11_t* sync = SyncObjectPool::Get(obj);

    if (sync)
        signaled = sync->is_signaled;

    return signaled;
}

//------------------------------------------------------------------------------

static void
_ExecuteQueuedCommandsDX11()
{
    Trace("rhi-dx11.exec-queued-cmd\n");

    TRACE_BEGIN_EVENT(22, "rhi", "_ExecuteQueuedCommandsDX11");

    if (_DX11_InitParam.FrameCommandExecutionSync)
        _DX11_InitParam.FrameCommandExecutionSync->Lock();

    std::vector<RenderPassDX11_t*> pass;
    std::vector<Handle> pass_h;
    ID3D11CommandList* cmdList = nullptr;
    Handle perfQuerySet = InvalidHandle;
    unsigned frame_n = 0;
    bool do_exec = true;

    _FrameSync.Lock();
    if (_Frame.size())
    {
        for (std::vector<Handle>::iterator p = _Frame.begin()->pass.begin(), p_end = _Frame.begin()->pass.end(); p != p_end; ++p)
        {
            RenderPassDX11_t* pp = RenderPassPool::Get(*p);
            bool do_add = true;

            for (unsigned i = 0; i != pass.size(); ++i)
            {
                if (pp->priority > pass[i]->priority)
                {
                    pass.insert(pass.begin() + i, 1, pp);
                    do_add = false;
                    break;
                }
            }

            if (do_add)
                pass.push_back(pp);
        }

        pass_h = _Frame.begin()->pass;
        frame_n = _Frame.begin()->number;
        cmdList = _Frame.begin()->cmdList;
        perfQuerySet = _Frame.begin()->perfQuerySet;
    }
    else
    {
        do_exec = false;
    }

    if (_Frame.size() && _Frame.begin()->sync != InvalidHandle)
    {
        SyncObjectDX11_t* sync = SyncObjectPool::Get(_Frame.begin()->sync);

        sync->frame = frame_n;
        sync->is_signaled = false;
        sync->is_used = true;
    }

    _FrameSync.Unlock();

    if (do_exec)
    {
        Trace("\n\n-------------------------------\nexecuting frame %u\n", frame_n);

        if (perfQuerySet != InvalidHandle)
        {
            if (_DX11_PerfQuerySetPending)
            {
                bool ready = false;
                bool valid = false;

                PerfQuerySetDX11::ObtainResults(perfQuerySet);
                PerfQuerySet::GetStatus(perfQuerySet, &ready, &valid);

                if (ready)
                {
                    _DX11_PerfQuerySetPending = false;
                    perfQuerySet = InvalidHandle;
                }
            }
            else
            {
                PerfQuerySet::Reset(perfQuerySet);
                PerfQuerySetDX11::BeginFreqMeasurment(perfQuerySet, _D3D11_ImmediateContext);
            }
        }

        if (perfQuerySet != InvalidHandle && !_DX11_PerfQuerySetPending)
            PerfQuerySetDX11::IssueFrameBeginQuery(perfQuerySet, _D3D11_ImmediateContext);
        
        #if RHI_DX11__USE_DEFERRED_CONTEXTS
        _D3D11_ImmediateContext->ExecuteCommandList(cmdList, FALSE);
        cmdList->Release();
        cmdList = nullptr;
        #endif

        for (std::vector<RenderPassDX11_t *>::iterator p = pass.begin(), p_end = pass.end(); p != p_end; ++p)
        {
            RenderPassDX11_t* pp = *p;

            if (perfQuerySet != InvalidHandle && !_DX11_PerfQuerySetPending && pp->perfQueryIndex0 != InvalidIndex)
                PerfQuerySetDX11::IssueTimestampQuery(perfQuerySet, pp->perfQueryIndex0, _D3D11_ImmediateContext);

            for (unsigned b = 0; b != pp->cmdBuf.size(); ++b)
            {
                Handle cb_h = pp->cmdBuf[b];
                CommandBufferDX11_t* cb = CommandBufferPool::Get(cb_h);

                cb->Execute();

                if (cb->sync != InvalidHandle)
                {
                    SyncObjectDX11_t* sync = SyncObjectPool::Get(cb->sync);

                    sync->frame = frame_n;
                    sync->is_signaled = false;
                }

                CommandBufferPool::Free(cb_h);
            }

            if (perfQuerySet != InvalidHandle && !_DX11_PerfQuerySetPending && pp->perfQueryIndex1 != InvalidIndex)
                PerfQuerySetDX11::IssueTimestampQuery(perfQuerySet, pp->perfQueryIndex1, _D3D11_ImmediateContext);
        }

        _FrameSync.Lock();
        {
            Trace("\n\n-------------------------------\nframe %u executed(submitted to GPU)\n", frame_n);
            _Frame.erase(_Frame.begin());

            for (std::vector<Handle>::iterator p = pass_h.begin(), p_end = pass_h.end(); p != p_end; ++p)
                RenderPassPool::Free(*p);
        }
        _FrameSync.Unlock();

        // do present

        _D3D11_SwapChain->Present(1, 0);

        if (perfQuerySet != InvalidHandle && !_DX11_PerfQuerySetPending)
            PerfQuerySetDX11::IssueFrameEndQuery(perfQuerySet, _D3D11_ImmediateContext);

        if (perfQuerySet != InvalidHandle && !_DX11_PerfQuerySetPending)
            PerfQuerySetDX11::EndFreqMeasurment(perfQuerySet, _D3D11_ImmediateContext);

        if (perfQuerySet != InvalidHandle && !_DX11_PerfQuerySetPending)
            _DX11_PerfQuerySetPending = true;

        // update sync-objects

        for (SyncObjectPool::Iterator s = SyncObjectPool::Begin(), s_end = SyncObjectPool::End(); s != s_end; ++s)
        {
            if (s->is_used && (frame_n - s->frame >= 2))
                s->is_signaled = true;
        }
    }

    if (_DX11_InitParam.FrameCommandExecutionSync)
        _DX11_InitParam.FrameCommandExecutionSync->Unlock();

    TRACE_END_EVENT(22, "rhi", "_ExecuteQueuedCommandsDX11");
}

//------------------------------------------------------------------------------

static void
_ExecDX11(DX11Command* command, uint32 cmdCount)
{
#if 1
    #define CHECK_HR(hr) \
    if (FAILED(hr)) \
        Logger::Error("%s", D3D11ErrorText(hr));
#else
    CHECK_HR(hr)
#endif

    for (DX11Command *cmd = command, *cmdEnd = command + cmdCount; cmd != cmdEnd; ++cmd)
    {
        const uint64* arg = cmd->arg;

        Trace("exec %i\n", int(cmd->func));
        switch (cmd->func)
        {
        case DX11Command::NOP:
            break;

        case DX11Command::MAP:
            cmd->retval = _D3D11_ImmediateContext->Map((ID3D11Resource*)(arg[0]), UINT(arg[1]), D3D11_MAP(arg[2]), UINT(arg[3]), (D3D11_MAPPED_SUBRESOURCE*)(arg[4]));
            break;

        case DX11Command::UNMAP:
            _D3D11_ImmediateContext->Unmap((ID3D11Resource*)(arg[0]), UINT(arg[1]));
            break;

        case DX11Command::UPDATE_SUBRESOURCE:
            _D3D11_ImmediateContext->UpdateSubresource((ID3D11Resource*)(arg[0]), UINT(arg[1]), (const D3D11_BOX*)(arg[2]), (const void*)(arg[3]), UINT(arg[4]), UINT(arg[5]));
            break;

        default:
            DVASSERT(!"unknown DX11-cmd");
        }
    }

    #undef CHECK_HR
}

//------------------------------------------------------------------------------

void ExecDX11(DX11Command* command, uint32 cmdCount, bool force_immediate)
{
    //TRACE_BEGIN_EVENT((force_immediate)?22:11,"rhi","ExecDX11");
    if (force_immediate || !_DX11_RenderThreadFrameCount)
    {
        _ExecDX11(command, cmdCount);
    }
    else
    {
        bool scheduled = false;
        bool executed = false;

        // CRAP: busy-wait
        do
        {
            _DX11_PendingImmediateCmdSync.Lock();
            if (!_DX11_PendingImmediateCmd)
            {
                _DX11_PendingImmediateCmd = command;
                _DX11_PendingImmediateCmdCount = cmdCount;
                scheduled = true;
            }
            _DX11_PendingImmediateCmdSync.Unlock();
        } while (!scheduled);

        // CRAP: busy-wait
        do
        {
            _DX11_PendingImmediateCmdSync.Lock();
            if (!_DX11_PendingImmediateCmd)
            {
                executed = true;
            }
            _DX11_PendingImmediateCmdSync.Unlock();
        } while (!executed);
    }
    //TRACE_END_EVENT((force_immediate)?22:11,"rhi","ExecDX11");
}

//------------------------------------------------------------------------------

static void
_RenderFuncDX11(DAVA::BaseObject* obj, void*, void*)
{
    _InitDX11();

    _DX11_RenderThreadStartedSync.Post();
    Logger::Info("RHI render-thread started");

    while (true)
    {
        bool do_wait = true;
        bool do_exit = false;

        TRACE_BEGIN_EVENT(22, "rhi", "wait-to-exec");
        do
        {
            // CRAP: busy-wait
            _DX11_RenderThreadExitSync.Lock();
            do_exit = _DX11_RenderThreadExitPending;
            _DX11_RenderThreadExitSync.Unlock();

            if (do_exit)
                break;

            //            TRACE_BEGIN_EVENT(22,"rhi","imm.cmd processing");
            _DX11_PendingImmediateCmdSync.Lock();
            if (_DX11_PendingImmediateCmd)
            {
                Trace("exec imm cmd (%u)\n", _DX11_PendingImmediateCmdCount);
                _ExecDX11(_DX11_PendingImmediateCmd, _DX11_PendingImmediateCmdCount);
                _DX11_PendingImmediateCmd = nullptr;
                _DX11_PendingImmediateCmdCount = 0;
                Trace("exec-imm-cmd done\n");
            }
            _DX11_PendingImmediateCmdSync.Unlock();
            //            TRACE_END_EVENT(22,"rhi","imm.cmd processing");

            _FrameSync.Lock();
            do_wait = !(_Frame.size() && _Frame.begin()->readyToExecute);
            _FrameSync.Unlock();
        } while (do_wait);
        TRACE_END_EVENT(22, "rhi", "wait-to-exec");

        if (do_exit)
            break;

        _ExecuteQueuedCommandsDX11();
    }

    Logger::Info("RHI render-thread stopped");
}

void InitializeRenderThreadDX11(uint32 frameCount)
{
    _DX11_RenderThreadFrameCount = frameCount;

    if (_DX11_RenderThreadFrameCount)
    {
        _DX11_RenderThread = DAVA::Thread::Create(DAVA::Message(&_RenderFuncDX11));
        _DX11_RenderThread->SetName("RHI.dx11-render");
        _DX11_RenderThread->Start();
        _DX11_RenderThread->SetPriority(DAVA::Thread::PRIORITY_HIGH);
        _DX11_RenderThreadStartedSync.Wait();
    }
    else
    {
        _InitDX11();
    }
}

//------------------------------------------------------------------------------

void UninitializeRenderThreadDX11()
{
    if (_DX11_RenderThreadFrameCount)
    {
        _DX11_RenderThreadExitSync.Lock();
        _DX11_RenderThreadExitPending = true;
        _DX11_RenderThreadExitSync.Unlock();

        _DX11_RenderThread->Join();
    }
}

//------------------------------------------------------------------------------

static void
dx11_Present(Handle sync)
{
    if (_DX11_RenderThreadFrameCount)
    {
        Trace("rhi-dx11.present\n");
        TRACE_BEGIN_EVENT(11, "rhi", "dx11_Present");
        _FrameSync.Lock();
        {
            if (_Frame.size())
            {
                #if RHI_DX11__USE_DEFERRED_CONTEXTS
                _D3D11_SecondaryContext->FinishCommandList(TRUE, &(_Frame.back().cmdList));
                #endif

                _Frame.back().readyToExecute = true;
                _Frame.back().sync = sync;
                _FrameStarted = false;
                Trace("\n\n-------------------------------\nframe %u generated\n", _Frame.back().number);
            }
        }
        _FrameSync.Unlock();

        unsigned frame_cnt = 0;

        do
        {
            _FrameSync.Lock();
            frame_cnt = static_cast<unsigned>(_Frame.size());
            //Trace("rhi-gl.present frame-cnt= %u\n",frame_cnt);
            _FrameSync.Unlock();
        } while (frame_cnt >= _DX11_RenderThreadFrameCount);
        TRACE_END_EVENT(11, "rhi", "dx11_Present");
    }
    else
    {
        TRACE_BEGIN_EVENT(22, "rhi", "dx11_Present");
        if (_Frame.size())
        {
            #if RHI_DX11__USE_DEFERRED_CONTEXTS
            _D3D11_SecondaryContext->FinishCommandList(TRUE, &(_Frame.back().cmdList));
            #endif

            _Frame.back().readyToExecute = true;
            _Frame.back().sync = sync;
            _FrameStarted = false;
        }
        else
        {
            #if RHI_DX11__USE_DEFERRED_CONTEXTS
            ID3D11CommandList* cl = nullptr;

            _D3D11_SecondaryContext->FinishCommandList(TRUE, &cl);
            _D3D11_ImmediateContext->ExecuteCommandList(cl, FALSE);
            cl->Release();
            #endif
        }

        _ExecuteQueuedCommandsDX11();
        TRACE_END_EVENT(22, "rhi", "dx11_Present");
    }
}

//------------------------------------------------------------------------------

CommandBufferDX11_t::CommandBufferDX11_t()
#if RHI_DX11__USE_DEFERRED_CONTEXTS
    : context(nullptr)
    , contextAnnotation(nullptr)
    , commandList(nullptr)
#else
    : cmdData(nullptr)
    , cmdDataSize(0)
    , curUsedSize(0)
#endif
{
}

//------------------------------------------------------------------------------

CommandBufferDX11_t::~CommandBufferDX11_t()
{
}

//------------------------------------------------------------------------------

void CommandBufferDX11_t::Execute()
{
    SCOPED_FUNCTION_TIMING();

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    TRACE_BEGIN_EVENT(22, "rhi", "CommandBufferDX11_t::Execute");
    context->Release();
    context = nullptr;

    if (contextAnnotation)
    {
        contextAnnotation->Release();
        contextAnnotation = nullptr;
    }

    _D3D11_ImmediateContext->ExecuteCommandList(commandList, FALSE);
    commandList->Release();
    commandList = nullptr;
    TRACE_END_EVENT(22, "rhi", "CommandBufferDX11_t::Execute");
#else

    for (const uint8 *c = cmdData, *c_end = cmdData + curUsedSize; c != c_end;)
    {
        const CommandDX11* cmd = (const CommandDX11*)c;

        switch (CommandDX11Type(cmd->type))
        {
        case DX11__BEGIN:
        {
            Begin(_D3D11_ImmediateContext);
        }
        break;

        case DX11__END:
        {
            sync = ((CommandDX11_End*)cmd)->syncObject;
        }
        break;

        case DX11__SET_VERTEX_DATA:
        {
            Handle vb = ((CommandDX11_SetVertexData*)cmd)->vb;

            cur_vb = vb;
            cur_vb_stride = (cur_vb_stride) ? cur_vb_stride : PipelineStateDX11::VertexLayoutStride(cur_pipelinestate);
        }
        break;

        case DX11__SET_INDICES:
        {
            cur_ib = ((CommandDX11_SetIndices*)cmd)->ib;
        }
        break;

        case DX11__SET_QUERY_BUFFER:
        {
            cur_query_buf = ((CommandDX11_SetQueryBuffer*)cmd)->queryBuf;
        }
        break;

        case DX11__SET_QUERY_INDEX:
        {
            cur_query_i = ((CommandDX11_SetQueryIndex*)cmd)->objectIndex;
        }
        break;

        case DX11__ISSUE_TIMESTAMP_QUERY:
        {
            Handle hset = ((CommandDX11_IssueTimestamptQuery*)cmd)->querySet;
            uint32 timestampIndex = ((CommandDX11_IssueTimestamptQuery*)cmd)->timestampIndex;

            PerfQuerySetDX11::IssueTimestampQuery(hset, timestampIndex, _D3D11_ImmediateContext);
        }
        break;

        case DX11__SET_PIPELINE_STATE:
        {
            Handle ps = ((CommandDX11_SetPipelineState*)cmd)->ps;
            Handle vdeclUID = ((CommandDX11_SetPipelineState*)cmd)->vdeclUID;
            const VertexLayout* vdecl = (vdeclUID == VertexLayout::InvalidUID) ? nullptr : VertexLayout::Get(vdeclUID);

            cur_pipelinestate = ps;
            cur_vb_stride = (vdecl) ? vdecl->Stride() : 0;

            PipelineStateDX11::SetToRHI(ps, vdeclUID, _D3D11_ImmediateContext);
        }
        break;

        case DX11__SET_CULL_MODE:
        {
            rs_param.cullMode = CullMode(((CommandDX11_SetCullMode*)cmd)->mode);
            cur_rs = nullptr;
        }
        break;

        case DX11__SET_SCISSOR_RECT:
        {
            int x = ((CommandDX11_SetScissorRect*)cmd)->x;
            int y = ((CommandDX11_SetScissorRect*)cmd)->y;
            int w = ((CommandDX11_SetScissorRect*)cmd)->w;
            int h = ((CommandDX11_SetScissorRect*)cmd)->h;

            if (!(x == 0 && y == 0 && w == 0 && h == 0))
            {
                D3D11_RECT rect = { x, y, x + w - 1, y + h - 1 };

                rs_param.scissorEnabled = true;
                cur_rs = nullptr;

                _D3D11_ImmediateContext->RSSetScissorRects(1, &rect);
            }
            else
            {
                rs_param.scissorEnabled = false;
                cur_rs = nullptr;
            }
        }
        break;

        case DX11__SET_VIEWPORT:
        {
            int x = ((CommandDX11_SetViewport*)cmd)->x;
            int y = ((CommandDX11_SetViewport*)cmd)->y;
            int w = ((CommandDX11_SetViewport*)cmd)->w;
            int h = ((CommandDX11_SetViewport*)cmd)->h;

            if (!(x == 0 && y == 0 && w == 0 && h == 0))
            {
                D3D11_VIEWPORT vp;

                vp.TopLeftX = float(x);
                vp.TopLeftY = float(y);
                vp.Width = float(w);
                vp.Height = float(h);
                vp.MinDepth = 0.0f;
                vp.MaxDepth = 1.0f;

                _D3D11_ImmediateContext->RSSetViewports(1, &vp);
            }
            else
            {
                _D3D11_ImmediateContext->RSSetViewports(1, &def_viewport);
            }
        }
        break;

        case DX11__SET_FILLMODE:
        {
            rs_param.wireframe = FillMode(((CommandDX11_SetFillMode*)cmd)->mode) == FILLMODE_WIREFRAME;
        }
        break;

        case DX11__SET_VERTEX_PROG_CONST_BUFFER:
        {
            Handle buffer = ((CommandDX11_SetVertexProgConstBuffer*)cmd)->buffer;
            const void* inst = ((CommandDX11_SetVertexProgConstBuffer*)cmd)->inst;

            ConstBufferDX11::SetToRHI(buffer, inst);
        }
        break;

        case DX11__SET_FRAGMENT_PROG_CONST_BUFFER:
        {
            Handle buffer = ((CommandDX11_SetFragmentProgConstBuffer*)cmd)->buffer;
            const void* inst = ((CommandDX11_SetFragmentProgConstBuffer*)cmd)->inst;

            ConstBufferDX11::SetToRHI(buffer, inst);
        }
        break;

        case DX11__SET_FRAGMENT_TEXTURE:
        {
            Handle tex = ((CommandDX11_SetFragmentTexture*)cmd)->tex;
            unsigned unitIndex = ((CommandDX11_SetFragmentTexture*)cmd)->unitIndex;
            TextureDX11::SetToRHIFragment(tex, unitIndex, _D3D11_ImmediateContext);
        }
        break;

        case DX11__SET_VERTEX_TEXTURE:
        {
            Handle tex = ((CommandDX11_SetVertexTexture*)cmd)->tex;
            unsigned unitIndex = ((CommandDX11_SetVertexTexture*)cmd)->unitIndex;
            TextureDX11::SetToRHIVertex(tex, unitIndex, _D3D11_ImmediateContext);
        }
        break;

        case DX11__SET_DEPTHSTENCIL_STATE:
        {
            DepthStencilStateDX11::SetToRHI(((CommandDX11_SetDepthStencilState*)cmd)->depthStencilState, _D3D11_ImmediateContext);
        }
        break;

        case DX11__SET_SAMPLER_STATE:
        {
            SamplerStateDX11::SetToRHI(((CommandDX11_SetSamplerState*)cmd)->samplerState, _D3D11_ImmediateContext);
        }
        break;

        case DX11__DRAW_PRIMITIVE:
        {
            D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY(((CommandDX11_DrawPrimitive*)cmd)->topo);
            unsigned vertexCount = ((CommandDX11_DrawPrimitive*)cmd)->vertexCount;
            unsigned baseVertex = ((CommandDX11_DrawPrimitive*)cmd)->baseVertex;

            if (topo != cur_topo)
            {
                _D3D11_ImmediateContext->IASetPrimitiveTopology(topo);
                cur_topo = topo;
            }

            if (!cur_rs)
            {
                cur_rs = _GetRasterizerState(rs_param);
                _D3D11_ImmediateContext->RSSetState(cur_rs);
            }

            VertexBufferDX11::SetToRHI(cur_vb, 0, 0, cur_vb_stride, _D3D11_ImmediateContext);

            if (cur_query_i != InvalidIndex)
                QueryBufferDX11::BeginQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);

            _D3D11_ImmediateContext->Draw(vertexCount, baseVertex);

            if (cur_query_i != InvalidIndex)
                QueryBufferDX11::EndQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);
        }
        break;

        case DX11__DRAW_INDEXED_PRIMITIVE:
        {
            D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY(((CommandDX11_DrawIndexedPrimitive*)cmd)->topo);
            unsigned vertexCount = ((CommandDX11_DrawIndexedPrimitive*)cmd)->vertexCount;
            unsigned baseVertex = ((CommandDX11_DrawIndexedPrimitive*)cmd)->baseVertex;
            unsigned indexCount = ((CommandDX11_DrawIndexedPrimitive*)cmd)->indexCount;
            unsigned startIndex = ((CommandDX11_DrawIndexedPrimitive*)cmd)->startIndex;

            if (topo != cur_topo)
            {
                _D3D11_ImmediateContext->IASetPrimitiveTopology(topo);
                cur_topo = topo;
            }

            if (!cur_rs)
            {
                cur_rs = _GetRasterizerState(rs_param);
                _D3D11_ImmediateContext->RSSetState(cur_rs);
            }

            IndexBufferDX11::SetToRHI(cur_ib, 0, _D3D11_ImmediateContext);
            VertexBufferDX11::SetToRHI(cur_vb, 0, 0, cur_vb_stride, _D3D11_ImmediateContext);

            if (cur_query_i != InvalidIndex)
                QueryBufferDX11::BeginQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);

            _D3D11_ImmediateContext->DrawIndexed(indexCount, startIndex, baseVertex);

            if (cur_query_i != InvalidIndex)
                QueryBufferDX11::BeginQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);
        }
        break;
        }

        if (cmd->type == DX11__END)
            break;
        c += cmd->size;
    }
#endif
}

void CommandBufferDX11_t::Begin(ID3D11DeviceContext* context)
{
    bool clear_color = isFirstInPass && passCfg.colorBuffer[0].loadAction == LOADACTION_CLEAR;
    bool clear_depth = isFirstInPass && passCfg.depthStencilBuffer.loadAction == LOADACTION_CLEAR;
    ID3D11RenderTargetView* rt[1] = { _D3D11_RenderTargetView };

    cur_topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    cur_ib = InvalidIndex;
    cur_vb = InvalidIndex;
    cur_vb_stride = 0;
    cur_pipelinestate = InvalidHandle;
    cur_stride = 0;
    cur_query_buf = InvalidHandle;
    cur_query_i = InvalidIndex;
    cur_rs = nullptr;

    rs_param.cullMode = CULL_NONE;
    rs_param.scissorEnabled = false;
    rs_param.wireframe = false;

    sync = InvalidHandle;

    def_viewport.TopLeftX = 0;
    def_viewport.TopLeftY = 0;
    def_viewport.MinDepth = 0.0f;
    def_viewport.MaxDepth = 1.0f;

    if (passCfg.colorBuffer[0].texture != rhi::InvalidHandle && passCfg.colorBuffer[0].texture != rhi::DefaultDepthBuffer)
    {
        Size2i sz = TextureDX11::Size(passCfg.colorBuffer[0].texture);

        def_viewport.Width = float(sz.dx);
        def_viewport.Height = float(sz.dy);

        TextureDX11::SetRenderTarget(passCfg.colorBuffer[0].texture, passCfg.depthStencilBuffer.texture, context);
    }
    else
    {
        context->OMSetRenderTargets(1, rt, _D3D11_DepthStencilView);
    }

    ID3D11RenderTargetView* rt_view[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };
    ID3D11DepthStencilView* ds_view = NULL;

    context->OMGetRenderTargets(countof(rt_view), rt_view, &ds_view);

    for (unsigned i = 0; i != countof(rt_view); ++i)
    {
        if (rt_view[i])
        {
            if (i == 0)
            {
                if (passCfg.colorBuffer[0].texture == rhi::InvalidHandle)
                {
                    D3D11_TEXTURE2D_DESC desc;

                    _D3D11_SwapChainBuffer->GetDesc(&desc);

                    def_viewport.Width = float(desc.Width);
                    def_viewport.Height = float(desc.Height);
                }

                context->RSSetViewports(1, &(def_viewport));
            }

            if (clear_color)
                context->ClearRenderTargetView(rt_view[i], passCfg.colorBuffer[0].clearColor);

            rt_view[i]->Release();
        }
    }

    if (ds_view)
    {
        if (clear_depth)
            context->ClearDepthStencilView(ds_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, passCfg.depthStencilBuffer.clearDepth, passCfg.depthStencilBuffer.clearStencil);

        ds_view->Release();
    }

    context->IASetPrimitiveTopology(cur_topo);
}

//------------------------------------------------------------------------------

namespace CommandBufferDX11
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_CommandBuffer_Begin = &dx11_CommandBuffer_Begin;
    dispatch->impl_CommandBuffer_End = &dx11_CommandBuffer_End;
    dispatch->impl_CommandBuffer_SetPipelineState = &dx11_CommandBuffer_SetPipelineState;
    dispatch->impl_CommandBuffer_SetCullMode = &dx11_CommandBuffer_SetCullMode;
    dispatch->impl_CommandBuffer_SetScissorRect = &dx11_CommandBuffer_SetScissorRect;
    dispatch->impl_CommandBuffer_SetViewport = &dx11_CommandBuffer_SetViewport;
    dispatch->impl_CommandBuffer_SetFillMode = &dx11_CommandBuffer_SetFillMode;
    dispatch->impl_CommandBuffer_SetVertexData = &dx11_CommandBuffer_SetVertexData;
    dispatch->impl_CommandBuffer_SetVertexConstBuffer = &dx11_CommandBuffer_SetVertexConstBuffer;
    dispatch->impl_CommandBuffer_SetVertexTexture = &dx11_CommandBuffer_SetVertexTexture;
    dispatch->impl_CommandBuffer_SetIndices = &dx11_CommandBuffer_SetIndices;
    dispatch->impl_CommandBuffer_SetQueryBuffer = &dx11_CommandBuffer_SetQueryBuffer;
    dispatch->impl_CommandBuffer_SetQueryIndex = &dx11_CommandBuffer_SetQueryIndex;
    dispatch->impl_CommandBuffer_SetFragmentConstBuffer = &dx11_CommandBuffer_SetFragmentConstBuffer;
    dispatch->impl_CommandBuffer_SetFragmentTexture = &dx11_CommandBuffer_SetFragmentTexture;
    dispatch->impl_CommandBuffer_SetDepthStencilState = &dx11_CommandBuffer_SetDepthStencilState;
    dispatch->impl_CommandBuffer_SetSamplerState = &dx11_CommandBuffer_SetSamplerState;
    dispatch->impl_CommandBuffer_DrawPrimitive = &dx11_CommandBuffer_DrawPrimitive;
    dispatch->impl_CommandBuffer_DrawIndexedPrimitive = &dx11_CommandBuffer_DrawIndexedPrimitive;
    dispatch->impl_CommandBuffer_SetMarker = &dx11_CommandBuffer_SetMarker;

    dispatch->impl_SyncObject_Create = &dx11_SyncObject_Create;
    dispatch->impl_SyncObject_Delete = &dx11_SyncObject_Delete;
    dispatch->impl_SyncObject_IsSignaled = &dx11_SyncObject_IsSignaled;

    dispatch->impl_Present = &dx11_Present;
}

void DiscardAll()
{
    if (_DX11_InitParam.FrameCommandExecutionSync)
        _DX11_InitParam.FrameCommandExecutionSync->Lock();

    _FrameSync.Lock();
    for (std::vector<FrameDX11>::iterator f = _Frame.begin(), f_end = _Frame.end(); f != f_end; ++f)
    {
        if (f->readyToExecute)
        {
            if (f->sync != InvalidHandle)
            {
                SyncObjectDX11_t* s = SyncObjectPool::Get(f->sync);
                s->is_signaled = true;
                s->is_used = true;
            }

            for (std::vector<Handle>::iterator p = f->pass.begin(), p_end = f->pass.end(); p != p_end; ++p)
            {
                RenderPassDX11_t* pp = RenderPassPool::Get(*p);

                for (std::vector<Handle>::iterator b = pp->cmdBuf.begin(), b_end = pp->cmdBuf.end(); b != b_end; ++b)
                {
                    CommandBufferDX11_t* cb = CommandBufferPool::Get(*b);

                    if (cb->sync != InvalidHandle)
                    {
                        SyncObjectDX11_t* s = SyncObjectPool::Get(cb->sync);
                        s->is_signaled = true;
                        s->is_used = true;
                    }

                    #if RHI_DX11__USE_DEFERRED_CONTEXTS
                    if (cb->commandList)
                        cb->commandList->Release();
                    #endif

                    CommandBufferPool::Free(*b);
                }

                RenderPassPool::Free(*p);
            }
        }
    }
    _Frame.clear();
    _FrameSync.Unlock();

    _D3D11_ImmediateContext->OMSetRenderTargets(0, NULL, NULL);

    if (_DX11_InitParam.FrameCommandExecutionSync)
        _DX11_InitParam.FrameCommandExecutionSync->Unlock();
}
}

//==============================================================================
} // namespace rhi
