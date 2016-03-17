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
    #include "_dx11.h"
    #include "rhi_DX11.h"

	#include "../rhi_Type.h"
    #include "../Common/rhi_RingBuffer.h"
    #include "../Common/dbg_StatSet.h"

    #include "Debug/DVAssert.h"
    #include "Logger/Logger.h"
using DAVA::Logger;
    #include "Core/Core.h"
    #include "Debug/Profiler.h"
    #include "Concurrency/Thread.h"
    #include "Concurrency/Semaphore.h"
    #include "Concurrency/AutoResetEvent.h"

#define LUMIA_1020_DEPTHBUF_WORKAROUND 1

#if LUMIA_1020_DEPTHBUF_WORKAROUND
#include "Platform/DeviceInfo.h"
#endif

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
    DX11__DRAW_INSTANCED_PRIMITIVE,
    DX11__DRAW_INSTANCED_INDEXED_PRIMITIVE,

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
CommandDX11_DrawInstancedPrimitive : public CommandDX11Impl<CommandDX11_DrawInstancedPrimitive, DX11__DRAW_INSTANCED_PRIMITIVE>
{
    uint8 topo;
    uint32 vertexCount;
    uint32 baseVertex;
    uint32 instCount;
};

struct
CommandDX11_DrawInstancedIndexedPrimitive : public CommandDX11Impl<CommandDX11_DrawInstancedIndexedPrimitive, DX11__DRAW_INSTANCED_INDEXED_PRIMITIVE>
{
    uint8 topo;
    uint32 indexCount;
    uint32 vertexCount;
    uint32 baseVertex;
    uint32 startIndex;
    uint32 instCount;
    uint32 baseInst;
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
    void Reset();
    void Execute();

    void _ApplyTopology(PrimitiveType primType, uint32 primCount, unsigned* indexCount);
    void _ApplyVertexData();
    void _ApplyRasterizerState();
    void _ApplyConstBuffers();

    RenderPassConfig passCfg;
    uint32 isFirstInPass : 1;
    uint32 isLastInPass : 1;
    uint32 isComplete : 1;

    D3D11_PRIMITIVE_TOPOLOGY cur_topo;
    Handle cur_vb[MAX_VERTEX_STREAM_COUNT];
    uint32 cur_vb_stride[MAX_VERTEX_STREAM_COUNT];
    uint32 cur_stream_count;
    Handle cur_pipelinestate;
    uint32 cur_stride;
    Handle cur_query_buf;
    uint32 cur_query_i;
    D3D11_VIEWPORT def_viewport;
    RasterizerParamDX11 rs_param;
    ID3D11RasterizerState* cur_rs;

    ID3D11RasterizerState* last_rs;
    Handle last_vb[MAX_VERTEX_STREAM_COUNT];
    uint32 last_vb_stride[MAX_VERTEX_STREAM_COUNT];
    Handle last_ps;
    uint32 last_vdecl;
#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ID3D11DeviceContext* context;
    ID3DUserDefinedAnnotation* contextAnnotation;
    ID3D11CommandList* commandList;

    ID3D11Buffer* vertexConstBuffer[MAX_CONST_BUFFER_COUNT];
    ID3D11Buffer* fragmentConstBuffer[MAX_CONST_BUFFER_COUNT];
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

typedef ResourcePool<CommandBufferDX11_t, RESOURCE_COMMAND_BUFFER, CommandBuffer::Descriptor, false> CommandBufferPoolDX11;
typedef ResourcePool<RenderPassDX11_t, RESOURCE_RENDER_PASS, RenderPassConfig, false> RenderPassPoolDX11;
typedef ResourcePool<SyncObjectDX11_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false> SyncObjectPoolDX11;

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
    uint32 toBeDiscarded : 1;

    ID3D11CommandList* cmdList;
};

static std::vector<FrameDX11> _DX11_Frame;
static bool _DX11_FrameStarted = false;
static unsigned _DX11_FrameNumber = 1;
static bool _DX11_ResetPending = false;
//static DAVA::Spinlock       _FrameSync;
static DAVA::Mutex _DX11_FrameSync;

static DAVA::AutoResetEvent _DX11_FramePreparedEvent(false, 400);
static DAVA::AutoResetEvent _DX11_FrameDoneEvent(false, 400);

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

    Handle handle = RenderPassPoolDX11::Alloc();
    RenderPassDX11_t* pass = RenderPassPoolDX11::Get(handle);

    pass->cmdBuf.resize(cmdBufCount);
    pass->priority = passDesc.priority;
    pass->perfQueryIndex0 = passDesc.PerfQueryIndex0;
    pass->perfQueryIndex1 = passDesc.PerfQueryIndex1;

    for (unsigned i = 0; i != cmdBufCount; ++i)
    {
        Handle h = CommandBufferPoolDX11::Alloc();
        CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(h);

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
    _DX11_FrameSync.Lock();

    if (!_DX11_FrameStarted)
    {
        _DX11_Frame.push_back(FrameDX11());
        _DX11_Frame.back().number = _DX11_FrameNumber;
        _DX11_Frame.back().sync = rhi::InvalidHandle;
        _DX11_Frame.back().perfQuerySet = PerfQuerySetDX11::Current();
        _DX11_Frame.back().readyToExecute = false;
        _DX11_Frame.back().toBeDiscarded = false;
        _DX11_Frame.back().cmdList = nullptr;

        Trace("\n\n-------------------------------\nframe %u started\n", _DX11_FrameNumber);
        _DX11_FrameStarted = true;
        ++_DX11_FrameNumber;
#if !RHI_DX11__USE_DEFERRED_CONTEXTS
        ConstBufferDX11::InvalidateAllInstances();
#endif
    }

    if (_DX11_Frame.size())
        _DX11_Frame.back().pass.push_back(pass);

    _DX11_FrameSync.Unlock();
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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ID3D11DeviceContext* context = cb->context;
    cb->Reset();
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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    cb->context->FinishCommandList(TRUE, &(cb->commandList));
    cb->sync = syncObject;
    cb->isComplete = true;
#else
    CommandDX11_End* cmd = cb->allocCmd<CommandDX11_End>();
    cmd->syncObject = syncObject;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetPipelineState(Handle cmdBuf, Handle ps, uint32 vdeclUID)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
#if RHI_DX11__USE_DEFERRED_CONTEXTS
    const VertexLayout* vdecl = (vdeclUID == VertexLayout::InvalidUID) ? nullptr : VertexLayout::Get(vdeclUID);

    cb->cur_pipelinestate = ps;
    cb->cur_stream_count = PipelineStateDX11::VertexLayoutStreamCount(ps);
    for (unsigned i = 0; i != cb->cur_stream_count; ++i)
        cb->cur_vb_stride[i] = (vdecl) ? vdecl->Stride(i) : 0;

    if (ps != cb->last_ps || vdeclUID != cb->last_vdecl)
    {
        PipelineStateDX11::SetToRHI(ps, vdeclUID, cb->context);
        cb->last_ps = ps;
        cb->last_vdecl = vdeclUID;
        StatSet::IncStat(stat_SET_PS, 1);
    }
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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    cb->cur_vb[streamIndex] = vb;
    if (!cb->cur_vb_stride[streamIndex])
        cb->cur_vb_stride[streamIndex] = PipelineStateDX11::VertexLayoutStride(cb->cur_pipelinestate, streamIndex);
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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ConstBufferDX11::SetToRHI(buffer, cb->context, cb->vertexConstBuffer);
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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    IndexBufferDX11::SetToRHI(ib, 0, cb->context);
#else
    CommandDX11_SetIndices* cmd = cb->allocCmd<CommandDX11_SetIndices>();
    cmd->ib = ib;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetQueryIndex(Handle cmdBuf, uint32 objectIndex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ConstBufferDX11::SetToRHI(buffer, cb->context, cb->fragmentConstBuffer);
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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

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
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    unsigned vertexCount = 0;

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ID3D11DeviceContext* ctx = cb->context;
    INT baseVertex = 0;
    cb->_ApplyTopology(type, count, &vertexCount);
    cb->_ApplyVertexData();
    cb->_ApplyRasterizerState();
    cb->_ApplyConstBuffers();

    if (cb->cur_query_i != DAVA::InvalidIndex)
        QueryBufferDX11::BeginQuery(cb->cur_query_buf, cb->cur_query_i, ctx);

    ctx->Draw(vertexCount, baseVertex);

    if (cb->cur_query_i != DAVA::InvalidIndex)
        QueryBufferDX11::EndQuery(cb->cur_query_buf, cb->cur_query_i, ctx);

    StatSet::IncStat(stat_DIP, 1);
#else
    CommandDX11_DrawPrimitive* cmd = cb->allocCmd<CommandDX11_DrawPrimitive>();
    cb->_ApplyTopology(type, count, &vertexCount);

    cmd->topo = cb->cur_topo;
    cmd->vertexCount = vertexCount;
    cmd->baseVertex = 0;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_DrawIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count, uint32 vertexCount, uint32 firstVertex, uint32 startIndex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    unsigned indexCount = 0;

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ID3D11DeviceContext* ctx = cb->context;
    cb->_ApplyTopology(type, count, &indexCount);
    cb->_ApplyVertexData();
    cb->_ApplyRasterizerState();
    cb->_ApplyConstBuffers();

    if (cb->cur_query_i != DAVA::InvalidIndex)
        QueryBufferDX11::BeginQuery(cb->cur_query_buf, cb->cur_query_i, ctx);

    ctx->DrawIndexed(indexCount, startIndex, firstVertex);

    if (cb->cur_query_i != DAVA::InvalidIndex)
        QueryBufferDX11::BeginQuery(cb->cur_query_buf, cb->cur_query_i, ctx);

    StatSet::IncStat(stat_DIP, 1);
#else
    CommandDX11_DrawIndexedPrimitive* cmd = cb->allocCmd<CommandDX11_DrawIndexedPrimitive>();
    cb->_ApplyTopology(type, count, &indexCount);

    cmd->topo = cb->cur_topo;
    cmd->vertexCount = vertexCount;
    cmd->baseVertex = firstVertex;
    cmd->indexCount = indexCount;
    cmd->startIndex = startIndex;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_DrawInstancedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 instCount, uint32 count)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    unsigned vertexCount = 0;
    INT baseVertex = 0;

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ID3D11DeviceContext* ctx = cb->context;
    cb->_ApplyTopology(type, count, &vertexCount);
    cb->_ApplyVertexData();
    cb->_ApplyRasterizerState();
    cb->_ApplyConstBuffers();

    if (cb->cur_query_i != DAVA::InvalidIndex)
        QueryBufferDX11::BeginQuery(cb->cur_query_buf, cb->cur_query_i, ctx);

    ctx->DrawInstanced(vertexCount, instCount, baseVertex, 0);

    if (cb->cur_query_i != DAVA::InvalidIndex)
        QueryBufferDX11::EndQuery(cb->cur_query_buf, cb->cur_query_i, ctx);

    StatSet::IncStat(stat_DIP, 1);
#else
    CommandDX11_DrawInstancedPrimitive* cmd = cb->allocCmd<CommandDX11_DrawInstancedPrimitive>();
    cb->_ApplyTopology(type, count, &vertexCount);

    cmd->topo = cb->cur_topo;
    cmd->instCount = instCount;
    cmd->vertexCount = vertexCount;
    cmd->baseVertex = 0;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_DrawInstancedIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 instCount, uint32 count, uint32 vertexCount, uint32 firstVertex, uint32 startIndex, uint32 baseInstance)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    unsigned indexCount = 0;

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ID3D11DeviceContext* ctx = cb->context;
    cb->_ApplyTopology(type, count, &indexCount);
    cb->_ApplyVertexData();
    cb->_ApplyRasterizerState();
    cb->_ApplyConstBuffers();

    if (cb->cur_query_i != DAVA::InvalidIndex)
        QueryBufferDX11::BeginQuery(cb->cur_query_buf, cb->cur_query_i, ctx);

    ctx->DrawIndexedInstanced(indexCount, instCount, startIndex, firstVertex, baseInstance);

    if (cb->cur_query_i != DAVA::InvalidIndex)
        QueryBufferDX11::BeginQuery(cb->cur_query_buf, cb->cur_query_i, ctx);

    StatSet::IncStat(stat_DIP, 1);
#else
    CommandDX11_DrawInstancedIndexedPrimitive* cmd = cb->allocCmd<CommandDX11_DrawInstancedIndexedPrimitive>();
    cb->_ApplyTopology(type, count, &indexCount);

    cmd->topo = cb->cur_topo;
    cmd->indexCount = indexCount;
    cmd->instCount = instCount;
    cmd->vertexCount = vertexCount;
    cmd->baseVertex = firstVertex;
    cmd->instCount = instCount;
    cmd->baseInst = baseInstance;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetMarker(Handle cmdBuf, const char* text)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
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
    Handle handle = SyncObjectPoolDX11::Alloc();
    SyncObjectDX11_t* sync = SyncObjectPoolDX11::Get(handle);

    sync->is_signaled = false;
    sync->is_used = false;

    return handle;
}

//------------------------------------------------------------------------------

static void
dx11_SyncObject_Delete(Handle obj)
{
    SyncObjectPoolDX11::Free(obj);
}

//------------------------------------------------------------------------------

static bool
dx11_SyncObject_IsSignaled(Handle obj)
{
    bool signaled = false;
    SyncObjectDX11_t* sync = SyncObjectPoolDX11::Get(obj);

    if (sync)
        signaled = sync->is_signaled;

    return signaled;
}

//------------------------------------------------------------------------------

static void
_ExecuteQueuedCommandsDX11()
{
    Trace("rhi-dx11.exec-queued-cmd\n");

    if (_DX11_InitParam.FrameCommandExecutionSync)
        _DX11_InitParam.FrameCommandExecutionSync->Lock();

    std::vector<RenderPassDX11_t*> pass;
    std::vector<Handle> pass_h;
    ID3D11CommandList* cmdList = nullptr;
    Handle perfQuerySet = InvalidHandle;
    unsigned frame_n = 0;
    bool do_exec = true;
    bool do_discard = false;

    _DX11_FrameSync.Lock();
    if (_DX11_Frame.size())
    {
        for (std::vector<Handle>::iterator p = _DX11_Frame.begin()->pass.begin(), p_end = _DX11_Frame.begin()->pass.end(); p != p_end; ++p)
        {
            RenderPassDX11_t* pp = RenderPassPoolDX11::Get(*p);
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

        pass_h = _DX11_Frame.begin()->pass;
        frame_n = _DX11_Frame.begin()->number;
        cmdList = _DX11_Frame.begin()->cmdList;
        perfQuerySet = _DX11_Frame.begin()->perfQuerySet;
        do_discard = _DX11_Frame.begin()->toBeDiscarded;
    }
    else
    {
        do_exec = false;
    }

    if (_DX11_Frame.size() && _DX11_Frame.begin()->sync != InvalidHandle)
    {
        SyncObjectDX11_t* sync = SyncObjectPoolDX11::Get(_DX11_Frame.begin()->sync);

        sync->frame = frame_n;
        sync->is_signaled = false;
        sync->is_used = true;
    }

    _DX11_FrameSync.Unlock();

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

            if (perfQuerySet != InvalidHandle && !_DX11_PerfQuerySetPending && pp->perfQueryIndex0 != DAVA::InvalidIndex)
                PerfQuerySetDX11::IssueTimestampQuery(perfQuerySet, pp->perfQueryIndex0, _D3D11_ImmediateContext);

            for (unsigned b = 0; b != pp->cmdBuf.size(); ++b)
            {
                Handle cb_h = pp->cmdBuf[b];
                CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cb_h);

                if (!do_discard)
                {
                    TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "cb::exec");
                    cb->Execute();
                    TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "cb::exec");
                }

                if (cb->sync != InvalidHandle)
                {
                    SyncObjectDX11_t* sync = SyncObjectPoolDX11::Get(cb->sync);

                    sync->frame = frame_n;
                    sync->is_signaled = false;
                }

                CommandBufferPoolDX11::Free(cb_h);
            }

            if (perfQuerySet != InvalidHandle && !_DX11_PerfQuerySetPending && pp->perfQueryIndex1 != DAVA::InvalidIndex)
                PerfQuerySetDX11::IssueTimestampQuery(perfQuerySet, pp->perfQueryIndex1, _D3D11_ImmediateContext);
        }

        _DX11_FrameSync.Lock();
        {
            Trace("\n\n-------------------------------\nframe %u executed(submitted to GPU)\n", frame_n);
            _DX11_Frame.erase(_DX11_Frame.begin());

            for (std::vector<Handle>::iterator p = pass_h.begin(), p_end = pass_h.end(); p != p_end; ++p)
                RenderPassPoolDX11::Free(*p);
        }
        _DX11_FrameSync.Unlock();

        // do present

        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "SwapChain::Present");
        _D3D11_SwapChain->Present(1, 0);
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "SwapChain::Present");

        if (perfQuerySet != InvalidHandle && !_DX11_PerfQuerySetPending)
            PerfQuerySetDX11::IssueFrameEndQuery(perfQuerySet, _D3D11_ImmediateContext);

        if (perfQuerySet != InvalidHandle && !_DX11_PerfQuerySetPending)
            PerfQuerySetDX11::EndFreqMeasurment(perfQuerySet, _D3D11_ImmediateContext);

        if (perfQuerySet != InvalidHandle && !_DX11_PerfQuerySetPending)
            _DX11_PerfQuerySetPending = true;

        // update sync-objects

        for (SyncObjectPoolDX11::Iterator s = SyncObjectPoolDX11::Begin(), s_end = SyncObjectPoolDX11::End(); s != s_end; ++s)
        {
            if (s->is_used && (frame_n - s->frame >= 2))
                s->is_signaled = true;
        }
    }

    if (_DX11_InitParam.FrameCommandExecutionSync)
        _DX11_InitParam.FrameCommandExecutionSync->Unlock();

    // take screenshot, if needed

    _D3D11_ScreenshotCallbackSync.Lock();
    if (_D3D11_PendingScreenshotCallback)
    {
        D3D11_TEXTURE2D_DESC desc = { 0 };

        _D3D11_SwapChainBuffer->GetDesc(&desc);

        if (!_D3D11_SwapChainBufferCopy)
        {
            desc.Usage = D3D11_USAGE_STAGING;
            desc.BindFlags = 0;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

            _D3D11_Device->CreateTexture2D(&desc, NULL, &_D3D11_SwapChainBufferCopy);
        }

        if (_D3D11_SwapChainBufferCopy)
        {
            D3D11_MAPPED_SUBRESOURCE res = { 0 };

            _D3D11_ImmediateContext->CopyResource(_D3D11_SwapChainBufferCopy, _D3D11_SwapChainBuffer);
            _D3D11_ImmediateContext->Map(_D3D11_SwapChainBufferCopy, 0, D3D11_MAP_READ, 0, &res);
            if (res.pData)
            {
                for (uint8 *p = (uint8 *)res.pData, *p_end = (uint8 *)res.pData + desc.Width * desc.Height * 4; p != p_end; p += 4)
                {
                    uint8 tmp = p[0];
                    p[0] = p[2];
                    p[2] = tmp;
                }

                (*_D3D11_PendingScreenshotCallback)(desc.Width, desc.Height, res.pData);
                _D3D11_ImmediateContext->Unmap(_D3D11_SwapChainBufferCopy, 0);
                _D3D11_PendingScreenshotCallback = nullptr;
            }
        }
    }
    _D3D11_ScreenshotCallbackSync.Unlock();
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

        case DX11Command::COPY_RESOURCE:
            _D3D11_ImmediateContext->CopyResource((ID3D11Resource*)(arg[0]), (ID3D11Resource*)(arg[1]));
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
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "wait_immediate_cmd");

        while (!scheduled)
        {
            _DX11_PendingImmediateCmdSync.Lock();
            if (!_DX11_PendingImmediateCmd)
            {
                _DX11_PendingImmediateCmd = command;
                _DX11_PendingImmediateCmdCount = cmdCount;
                scheduled = true;
            }
            _DX11_PendingImmediateCmdSync.Unlock();
        }

        // CRAP: busy-wait
        while (!executed)
        {
            _DX11_PendingImmediateCmdSync.Lock();
            if (!_DX11_PendingImmediateCmd)
            {
                executed = true;
            }
            _DX11_PendingImmediateCmdSync.Unlock();

            if (!executed)
            {
                _DX11_FramePreparedEvent.Signal();
            }
        }

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "wait_immediate_cmd");
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

    bool do_exit = false;
    while (!do_exit)
    {
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::render_loop");
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "renderer_wait_core");

        bool do_wait = true;
        while (do_wait)
        {
            // CRAP: busy-wait
            _DX11_RenderThreadExitSync.Lock();
            do_exit = _DX11_RenderThreadExitPending;
            _DX11_RenderThreadExitSync.Unlock();

            if (do_exit)
                break;

            _DX11_PendingImmediateCmdSync.Lock();
            if (_DX11_PendingImmediateCmd)
            {
                Trace("exec imm cmd (%u)\n", _DX11_PendingImmediateCmdCount);
                TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "immediate_cmd");
                _ExecDX11(_DX11_PendingImmediateCmd, _DX11_PendingImmediateCmdCount);
                _DX11_PendingImmediateCmd = nullptr;
                _DX11_PendingImmediateCmdCount = 0;
                Trace("exec-imm-cmd done\n");
                TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "immediate_cmd");
            }
            _DX11_PendingImmediateCmdSync.Unlock();

            _DX11_FrameSync.Lock();
            do_wait = !(_DX11_Frame.size() && _DX11_Frame.begin()->readyToExecute);
            _DX11_FrameSync.Unlock();

            if (do_wait)
            {
                _DX11_FramePreparedEvent.Wait();
            }
        }

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "renderer_wait_core");

        if (!do_exit)
        {
            TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "exec_que_cmds");
            _ExecuteQueuedCommandsDX11();
            TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "exec_que_cmds");
        }

        _DX11_FrameDoneEvent.Signal();

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::render_loop");
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
        _DX11_RenderThread->BindToProcessor(1);
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
        _DX11_FramePreparedEvent.Signal();

        _DX11_RenderThread->Join();
    }
}

//------------------------------------------------------------------------------

static void
dx11_Present(Handle sync)
{
    TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::present");

    if (_DX11_RenderThreadFrameCount)
    {
        Trace("rhi-dx11.present\n");

        _DX11_FrameSync.Lock();
        {
            if (_DX11_Frame.size())
            {
                #if RHI_DX11__USE_DEFERRED_CONTEXTS
                _D3D11_SecondaryContext->FinishCommandList(TRUE, &(_DX11_Frame.back().cmdList));
                #endif

                _DX11_Frame.back().readyToExecute = true;
                _DX11_Frame.back().sync = sync;
                _DX11_FrameStarted = false;
                Trace("\n\n-------------------------------\nframe %u generated\n", _DX11_Frame.back().number);
            }
        }
        _DX11_FrameSync.Unlock();

        _DX11_FramePreparedEvent.Signal();

        unsigned frame_cnt = 0;
        bool reset_pending = false;

        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "core_wait_renderer");
        do
        {
            _DX11_FrameSync.Lock();
            frame_cnt = static_cast<unsigned>(_DX11_Frame.size());
            reset_pending = _DX11_ResetPending;
            //Trace("rhi-gl.present frame-cnt= %u\n",frame_cnt);
            _DX11_FrameSync.Unlock();

            if (frame_cnt >= _DX11_RenderThreadFrameCount || reset_pending)
            {
                _DX11_FrameDoneEvent.Wait();
            }
        }
        while (frame_cnt >= _DX11_RenderThreadFrameCount || reset_pending);
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "core_wait_renderer");
    }
    else
    {
        if (_DX11_Frame.size())
        {
            #if RHI_DX11__USE_DEFERRED_CONTEXTS
            _D3D11_SecondaryContext->FinishCommandList(TRUE, &(_DX11_Frame.back().cmdList));
            #endif

            _DX11_Frame.back().readyToExecute = true;
            _DX11_Frame.back().sync = sync;
            _DX11_FrameStarted = false;
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
    }
    TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::present");
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

void CommandBufferDX11_t::Reset()
{
    cur_topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    for (unsigned i = 0; i != MAX_VERTEX_STREAM_COUNT; ++i)
    {
        cur_vb[i] = InvalidHandle;
        cur_vb_stride[i] = 0;
    }
    cur_pipelinestate = InvalidHandle;
    cur_stride = 0;
    cur_query_buf = InvalidHandle;
    cur_query_i = DAVA::InvalidIndex;
    cur_rs = nullptr;

    rs_param.cullMode = CULL_NONE;
    rs_param.scissorEnabled = false;
    rs_param.wireframe = false;

    last_rs = nullptr;
    for (unsigned i = 0; i != MAX_VERTEX_STREAM_COUNT; ++i)
    {
        last_vb[i] = InvalidHandle;
        last_vb_stride[i] = 0;
    }
    last_ps = InvalidHandle;
    last_vdecl = VertexLayout::InvalidUID;

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    memset(vertexConstBuffer, 0, sizeof(vertexConstBuffer));
    memset(fragmentConstBuffer, 0, sizeof(fragmentConstBuffer));

    context->IASetPrimitiveTopology(cur_topo);
#endif

    isComplete = false;
}

//------------------------------------------------------------------------------

void CommandBufferDX11_t::_ApplyTopology(PrimitiveType primType, uint32 primCount, unsigned* indexCount)
{
    D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    switch (primType)
    {
    case PRIMITIVE_TRIANGLELIST:
        *indexCount = primCount * 3;
        topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        *indexCount = 2 + primCount;
        topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        break;

    case PRIMITIVE_LINELIST:
        *indexCount = primCount * 2;
        topo = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        break;
    }

    if (topo != cur_topo)
    {
        #if RHI_DX11__USE_DEFERRED_CONTEXTS
        context->IASetPrimitiveTopology(topo);
        #endif
        cur_topo = topo;
    }
}

//------------------------------------------------------------------------------

void CommandBufferDX11_t::_ApplyVertexData()
{
#if RHI_DX11__USE_DEFERRED_CONTEXTS
    for (unsigned i = 0; i != cur_stream_count; ++i)
    {
        if (cur_vb[i] != last_vb[i] || cur_vb_stride[i] != last_vb_stride[i])
        {
            VertexBufferDX11::SetToRHI(cur_vb[i], i, 0, cur_vb_stride[i], context);
            StatSet::IncStat(stat_SET_VB, 1);
            last_vb[i] = cur_vb[i];
            last_vb_stride[i] = cur_vb_stride[i];
        }
    }
#endif
}

//------------------------------------------------------------------------------

void CommandBufferDX11_t::_ApplyRasterizerState()
{
#if RHI_DX11__USE_DEFERRED_CONTEXTS
    if (!cur_rs)
    {
        cur_rs = _GetRasterizerState(rs_param);
    }

    if (cur_rs != last_rs)
    {
        context->RSSetState(cur_rs);
        last_rs = cur_rs;
    }
#endif
}

//------------------------------------------------------------------------------

void CommandBufferDX11_t::_ApplyConstBuffers()
{
#if RHI_DX11__USE_DEFERRED_CONTEXTS
#if 0
    unsigned vertexBufCount = 0;
    unsigned fragmentBufCount = 0;

    PipelineStateDX11::GetConstBufferCount(last_ps, &vertexBufCount, &fragmentBufCount);

    context->VSSetConstantBuffers(0, vertexBufCount, vertexConstBuffer);
    context->PSSetConstantBuffers(0, fragmentBufCount, fragmentConstBuffer);
#else
    context->VSSetConstantBuffers(0, MAX_CONST_BUFFER_COUNT, vertexConstBuffer);
    context->PSSetConstantBuffers(0, MAX_CONST_BUFFER_COUNT, fragmentConstBuffer);
#endif
    StatSet::IncStat(stat_SET_CB, 2);
#endif
}

//------------------------------------------------------------------------------

void CommandBufferDX11_t::Execute()
{
    SCOPED_FUNCTION_TIMING();

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    DVASSERT(isComplete);
    context->Release();
    context = nullptr;

    if (contextAnnotation)
    {
        contextAnnotation->Release();
        contextAnnotation = nullptr;
    }

    _D3D11_ImmediateContext->ExecuteCommandList(commandList, FALSE);
	
    #if LUMIA_1020_DEPTHBUF_WORKAROUND
    {
        static int isLumia1020 = -1;

        if (isLumia1020 == -1)
            isLumia1020 = DAVA::DeviceInfo::GetModel().find("NOKIA RM-875") == 0 ? 1 : 0;

        if (isLumia1020)
            _D3D11_ImmediateContext->Flush();
    }
    #endif

    commandList->Release();
    commandList = nullptr;
#else

    for (const uint8 *c = cmdData, *c_end = cmdData + curUsedSize; c != c_end;)
    {
        const CommandDX11* cmd = (const CommandDX11*)c;

        switch (CommandDX11Type(cmd->type))
        {
        case DX11__BEGIN:
        {
            Reset();
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
            unsigned stream_i = ((CommandDX11_SetVertexData*)cmd)->streamIndex;

            cur_vb[stream_i] = vb;
            if (!cur_vb_stride[stream_i])
                cur_vb_stride[stream_i] = PipelineStateDX11::VertexLayoutStride(cur_pipelinestate, stream_i);
        }
        break;

        case DX11__SET_INDICES:
        {
            Handle ib = ((CommandDX11_SetIndices*)cmd)->ib;
            IndexBufferDX11::SetToRHI(ib, 0, _D3D11_ImmediateContext);
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
            cur_stream_count = PipelineStateDX11::VertexLayoutStreamCount(ps);
            for (unsigned i = 0; i != cur_stream_count; ++i)
                cur_vb_stride[i] = (vdecl) ? vdecl->Stride(i) : 0;

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

            for (unsigned s = 0; s != cur_stream_count; ++s)
                VertexBufferDX11::SetToRHI(cur_vb[s], s, 0, cur_vb_stride[s], _D3D11_ImmediateContext);

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferDX11::BeginQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);

            _D3D11_ImmediateContext->Draw(vertexCount, baseVertex);

            if (cur_query_i != DAVA::InvalidIndex)
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

            for (unsigned s = 0; s != cur_stream_count; ++s)
                VertexBufferDX11::SetToRHI(cur_vb[s], s, 0, cur_vb_stride[s], _D3D11_ImmediateContext);

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferDX11::BeginQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);

            _D3D11_ImmediateContext->DrawIndexed(indexCount, startIndex, baseVertex);

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferDX11::BeginQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);
        }
        break;

        case DX11__DRAW_INSTANCED_PRIMITIVE:
        {
            D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY(((CommandDX11_DrawPrimitive*)cmd)->topo);
            unsigned vertexCount = ((CommandDX11_DrawInstancedPrimitive*)cmd)->vertexCount;
            unsigned baseVertex = ((CommandDX11_DrawInstancedPrimitive*)cmd)->baseVertex;
            unsigned instCount = ((CommandDX11_DrawInstancedPrimitive*)cmd)->instCount;

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

            for (unsigned s = 0; s != cur_stream_count; ++s)
                VertexBufferDX11::SetToRHI(cur_vb[s], s, 0, cur_vb_stride[s], _D3D11_ImmediateContext);

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferDX11::BeginQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);

            _D3D11_ImmediateContext->DrawInstanced(vertexCount, instCount, baseVertex, 0);

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferDX11::EndQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);
        }
        break;

        case DX11__DRAW_INSTANCED_INDEXED_PRIMITIVE:
        {
            D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY(((CommandDX11_DrawInstancedIndexedPrimitive*)cmd)->topo);
            unsigned vertexCount = ((CommandDX11_DrawInstancedIndexedPrimitive*)cmd)->vertexCount;
            unsigned baseVertex = ((CommandDX11_DrawInstancedIndexedPrimitive*)cmd)->baseVertex;
            unsigned indexCount = ((CommandDX11_DrawInstancedIndexedPrimitive*)cmd)->indexCount;
            unsigned startIndex = ((CommandDX11_DrawInstancedIndexedPrimitive*)cmd)->startIndex;
            unsigned instCount = ((CommandDX11_DrawInstancedIndexedPrimitive*)cmd)->instCount;
            unsigned baseInst = ((CommandDX11_DrawInstancedIndexedPrimitive*)cmd)->baseInst;

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

            for (unsigned s = 0; s != cur_stream_count; ++s)
                VertexBufferDX11::SetToRHI(cur_vb[s], s, 0, cur_vb_stride[s], _D3D11_ImmediateContext);

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferDX11::BeginQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);

            _D3D11_ImmediateContext->DrawIndexedInstanced(indexCount, instCount, startIndex, baseVertex, baseInst);

            if (cur_query_i != DAVA::InvalidIndex)
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

        TextureDX11::SetRenderTarget(passCfg.colorBuffer[0].texture, passCfg.depthStencilBuffer.texture, passCfg.colorBuffer[0].textureLevel, passCfg.colorBuffer[0].textureFace, context);
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
    dispatch->impl_CommandBuffer_DrawInstancedPrimitive = &dx11_CommandBuffer_DrawInstancedPrimitive;
    dispatch->impl_CommandBuffer_DrawInstancedIndexedPrimitive = &dx11_CommandBuffer_DrawInstancedIndexedPrimitive;
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

    _DX11_FrameSync.Lock();
    _DX11_ResetPending = true;
    _DX11_FrameSync.Unlock();

    _DX11_FrameSync.Lock();
#if RHI_DX11__USE_DEFERRED_CONTEXTS
    for (std::vector<FrameDX11>::iterator f = _DX11_Frame.begin(); f != _DX11_Frame.end();)
    {
        if (f->readyToExecute && f->sync != InvalidHandle)
        {
            SyncObjectDX11_t* s = SyncObjectPoolDX11::Get(f->sync);
            s->is_signaled = true;
            s->is_used = true;
        }

        for (std::vector<Handle>::iterator p = f->pass.begin(), p_end = f->pass.end(); p != p_end; ++p)
        {
            RenderPassDX11_t* pp = RenderPassPoolDX11::Get(*p);

            for (std::vector<Handle>::iterator b = pp->cmdBuf.begin(), b_end = pp->cmdBuf.end(); b != b_end; ++b)
            {
                CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(*b);

                if (cb->sync != InvalidHandle)
                {
                    SyncObjectDX11_t* s = SyncObjectPoolDX11::Get(cb->sync);
                    s->is_signaled = true;
                    s->is_used = true;
                }

                if (cb->context)
                {
                    if (!cb->isComplete)
                    {
                        cb->context->ClearState();
                        cb->context->FinishCommandList(FALSE, &(cb->commandList));
                    }

                    if (nullptr != cb->contextAnnotation)
                    {
                        cb->contextAnnotation->Release();
                        cb->contextAnnotation = nullptr;
                    }

                    cb->context->Release();
                    cb->context = nullptr;
                }

                if (cb->commandList)
                {
                    cb->commandList->Release();
                    cb->commandList = nullptr;
                }

                if (f->readyToExecute)
                    CommandBufferPoolDX11::Free(*b);
            }

            if (f->readyToExecute)
                RenderPassPoolDX11::Free(*p);
        }

        if (f->readyToExecute)
        {
            if (f->cmdList)
            {
                f->cmdList->Release();
                f->cmdList = nullptr;
            }

            f = _DX11_Frame.erase(f);
        }
        else
        {
            for (std::vector<Handle>::iterator p = f->pass.begin(), p_end = f->pass.end(); p != p_end; ++p)
            {
                RenderPassDX11_t* pp = RenderPassPoolDX11::Get(*p);

                for (std::vector<Handle>::iterator b = pp->cmdBuf.begin(), b_end = pp->cmdBuf.end(); b != b_end; ++b)
                {
                    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(*b);

                    HRESULT hr = _D3D11_Device->CreateDeferredContext(0, &(cb->context));

                    DVASSERT(cb->context);
                    cb->Reset();
                }
            }

            f->toBeDiscarded = true;
            ++f;
        }
    }

    {
        ID3D11CommandList* cl = nullptr;

        _D3D11_SecondaryContext->ClearState();
        _D3D11_SecondaryContext->FinishCommandList(FALSE, &cl);
        cl->Release();
        _D3D11_SecondaryContext->Release();

        _D3D11_Device->CreateDeferredContext(0, &_D3D11_SecondaryContext);
    }
#endif
    _DX11_ResetPending = false;
    _DX11_FrameSync.Unlock();

    ID3D11RenderTargetView* view[] = { nullptr };
    _D3D11_ImmediateContext->OMSetRenderTargets(1, view, nullptr);

    if (_DX11_InitParam.FrameCommandExecutionSync)
        _DX11_InitParam.FrameCommandExecutionSync->Unlock();
}
}

//==============================================================================
} // namespace rhi
