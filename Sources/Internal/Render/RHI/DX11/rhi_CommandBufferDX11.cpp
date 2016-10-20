#include "../Common/rhi_Pool.h"
#include "../Common/rhi_Utils.h"
#include "_dx11.h"
#include "rhi_DX11.h"

#include "../rhi_Type.h"
#include "../Common/rhi_RingBuffer.h"
#include "../Common/dbg_StatSet.h"

#include "Debug/DVAssert.h"
#include "Logger/Logger.h"

#include "Core/Core.h"
#include "Debug/CPUProfiler.h"

#include "../Common/SoftwareCommandBuffer.h"
#include "../Common/RenderLoop.h"
#include "../Common/rhi_CommonImpl.h"

using DAVA::Logger;

#define LUMIA_1020_DEPTHBUF_WORKAROUND 1

#if LUMIA_1020_DEPTHBUF_WORKAROUND
#include "Platform/DeviceInfo.h"
#endif


#include <vector>

namespace rhi
{
struct RasterizerParamDX11
{
    uint32 cullMode : 3;
    uint32 scissorEnabled : 1;
    uint32 wireframe : 1;

    bool operator==(const RasterizerParamDX11& b) const
    {
        return this->cullMode == b.cullMode && this->scissorEnabled == b.scissorEnabled && this->wireframe == b.wireframe;
    }
};

struct RasterizerStateDX11
{
    RasterizerParamDX11 param;
    ID3D11RasterizerState* state;
};

//==============================================================================
#if RHI_DX11__USE_DEFERRED_CONTEXTS
class CommandBufferDX11_t
#else
class CommandBufferDX11_t : public SoftwareCommandBuffer
#endif
{
public:
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
    D3D11_VIEWPORT def_viewport;
    RasterizerParamDX11 rs_param;
    ID3D11RasterizerState* cur_rs;

    ID3D11RasterizerState* last_rs;
    Handle last_vb[MAX_VERTEX_STREAM_COUNT];
    uint32 last_vb_stride[MAX_VERTEX_STREAM_COUNT];
    Handle last_ps;
    uint32 last_vdecl;
#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ID3D11DeviceContext* context = nullptr;
    ID3DUserDefinedAnnotation* contextAnnotation = nullptr;
    ID3D11CommandList* commandList = nullptr;

    ID3D11Buffer* vertexConstBuffer[MAX_CONST_BUFFER_COUNT];
    ID3D11Buffer* fragmentConstBuffer[MAX_CONST_BUFFER_COUNT];
#endif

    Handle sync;
};

class RenderPassDX11_t
{
public:
    std::vector<Handle> cmdBuf;
    int priority;
    uint32 perfQueryIndex0;
    uint32 perfQueryIndex1;
};

struct SyncObjectDX11_t
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

static bool _DX11_PerfQuerySetPending = false;

#if RHI_DX11__USE_DEFERRED_CONTEXTS
DAVA::Vector<ID3D11CommandList*> pendingSecondaryCmdLists;
DAVA::Mutex pendingSecondaryCmdListSync;
#endif

//------------------------------------------------------------------------------

static std::vector<RasterizerStateDX11> _RasterizerStateDX11;

static ID3D11RasterizerState* dx11_GetRasterizerState(RasterizerParamDX11 param)
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
        CHECK_HR(hr);

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

static Handle dx11_RenderPass_Allocate(const RenderPassConfig& passDesc, uint32 cmdBufCount, Handle* cmdBuf)
{
    DVASSERT(cmdBufCount);
    DVASSERT(passDesc.IsValid());

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
            CHECK_HR(hr);

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

static void dx11_RenderPass_Begin(Handle pass)
{
}

//------------------------------------------------------------------------------

static void dx11_RenderPass_End(Handle pass)
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

static void dx11_CommandBuffer_Begin(Handle cmdBuf)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ID3D11DeviceContext* context = cb->context;
    cb->Reset();
    cb->Begin(context);
#else
    cb->curUsedSize = 0;
    cb->allocCmd<SWCommand_Begin>();
#endif
}

//------------------------------------------------------------------------------

static void dx11_CommandBuffer_End(Handle cmdBuf, Handle syncObject)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    if (cb->isLastInPass && cb->cur_query_buf != InvalidHandle)
        QueryBufferDX11::QueryComplete(cb->cur_query_buf, cb->context);

    CHECK_HR(cb->context->FinishCommandList(TRUE, &(cb->commandList)));
    cb->sync = syncObject;
    cb->isComplete = true;
#else
    SWCommand_End* cmd = cb->allocCmd<SWCommand_End>();
    cmd->syncObject = syncObject;
#endif
}

//------------------------------------------------------------------------------

static void dx11_CommandBuffer_SetPipelineState(Handle cmdBuf, Handle ps, uint32 vdeclUID)
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
    SWCommand_SetPipelineState* cmd = cb->allocCmd<SWCommand_SetPipelineState>();
    cmd->ps = ps;
    cmd->vdecl = vdeclUID;
#endif
}

//------------------------------------------------------------------------------

static void dx11_CommandBuffer_SetCullMode(Handle cmdBuf, CullMode mode)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    cb->rs_param.cullMode = mode;
    cb->cur_rs = nullptr;
#else
    SWCommand_SetCullMode* cmd = cb->allocCmd<SWCommand_SetCullMode>();
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
    SWCommand_SetScissorRect* cmd = cb->allocCmd<SWCommand_SetScissorRect>();
    cmd->x = x;
    cmd->y = y;
    cmd->width = w;
    cmd->height = h;
#endif
}

//------------------------------------------------------------------------------

static void dx11_CommandBuffer_SetViewport(Handle cmdBuf, Viewport vp)
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
    SWCommand_SetViewport* cmd = cb->allocCmd<SWCommand_SetViewport>();
    cmd->x = x;
    cmd->y = y;
    cmd->width = w;
    cmd->height = h;
#endif
}

//------------------------------------------------------------------------------

static void dx11_CommandBuffer_SetFillMode(Handle cmdBuf, FillMode mode)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    cb->rs_param.wireframe = (mode == FILLMODE_WIREFRAME);
#else
    SWCommand_SetFillMode* cmd = cb->allocCmd<SWCommand_SetFillMode>();
    cmd->mode = mode;
#endif
}

//------------------------------------------------------------------------------

static void dx11_CommandBuffer_SetVertexData(Handle cmdBuf, Handle vb, uint32 streamIndex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    cb->cur_vb[streamIndex] = vb;
    if (!cb->cur_vb_stride[streamIndex])
        cb->cur_vb_stride[streamIndex] = PipelineStateDX11::VertexLayoutStride(cb->cur_pipelinestate, streamIndex);
#else
    SWCommand_SetVertexData* cmd = cb->allocCmd<SWCommand_SetVertexData>();
    cmd->vb = vb;
    cmd->streamIndex = streamIndex;
#endif
}

//------------------------------------------------------------------------------

static void dx11_CommandBuffer_SetVertexConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ConstBufferDX11::SetToRHI(buffer, cb->context, cb->vertexConstBuffer);
#else
    SWCommand_SetVertexProgConstBuffer* cmd = cb->allocCmd<SWCommand_SetVertexProgConstBuffer>();
    cmd->bufIndex = bufIndex;
    cmd->buffer = buffer;
    cmd->inst = ConstBufferDX11::Instance(buffer);
#endif
}

//------------------------------------------------------------------------------

static void dx11_CommandBuffer_SetVertexTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    TextureDX11::SetToRHIVertex(tex, unitIndex, cb->context);
    StatSet::IncStat(stat_SET_TEX, 1);
#else
    SWCommand_SetVertexTexture* cmd = cb->allocCmd<SWCommand_SetVertexTexture>();
    cmd->unitIndex = unitIndex;
    cmd->tex = tex;
#endif
}

//------------------------------------------------------------------------------

static void dx11_CommandBuffer_SetIndices(Handle cmdBuf, Handle ib)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    IndexBufferDX11::SetToRHI(ib, 0, cb->context);
#else
    SWCommand_SetIndices* cmd = cb->allocCmd<SWCommand_SetIndices>();
    cmd->ib = ib;
#endif
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetQueryIndex(Handle cmdBuf, uint32 objectIndex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

#if RHI_DX11__USE_DEFERRED_CONTEXTS
    if (cb->cur_query_buf != InvalidHandle)
        QueryBufferDX11::SetQueryIndex(cb->cur_query_buf, objectIndex, cb->context);
#else
    SWCommand_SetQueryIndex* cmd = cb->allocCmd<SWCommand_SetQueryIndex>();
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
    SWCommand_SetQueryBuffer* cmd = cb->allocCmd<SWCommand_SetQueryBuffer>();
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
    SWCommand_IssueTimestamptQuery* cmd = cb->allocCmd<SWCommand_IssueTimestamptQuery>();
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
    SWCommand_SetFragmentProgConstBuffer* cmd = cb->allocCmd<SWCommand_SetFragmentProgConstBuffer>();
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
    SWCommand_SetFragmentTexture* cmd = cb->allocCmd<SWCommand_SetFragmentTexture>();
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
    SWCommand_SetDepthStencilState* cmd = cb->allocCmd<SWCommand_SetDepthStencilState>();
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
    SWCommand_SetSamplerState* cmd = cb->allocCmd<SWCommand_SetSamplerState>();
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

    ctx->Draw(vertexCount, baseVertex);

    StatSet::IncStat(stat_DIP, 1);
#else
    SWCommand_DrawPrimitive* cmd = cb->allocCmd<SWCommand_DrawPrimitive>();
    cb->_ApplyTopology(type, count, &vertexCount);

    cmd->mode = cb->cur_topo;
    cmd->vertexCount = vertexCount;    
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

    ctx->DrawIndexed(indexCount, startIndex, firstVertex);

    StatSet::IncStat(stat_DIP, 1);
#else
    SWCommand_DrawIndexedPrimitive* cmd = cb->allocCmd<SWCommand_DrawIndexedPrimitive>();
    cb->_ApplyTopology(type, count, &indexCount);

    cmd->mode = cb->cur_topo;
    cmd->firstVertex = firstVertex;
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

    ctx->DrawInstanced(vertexCount, instCount, baseVertex, 0);

    StatSet::IncStat(stat_DIP, 1);
#else
    SWCommand_DrawInstancedPrimitive* cmd = cb->allocCmd<SWCommand_DrawInstancedPrimitive>();
    cb->_ApplyTopology(type, count, &vertexCount);

    cmd->mode = cb->cur_topo;
    cmd->instanceCount = instCount;
    cmd->vertexCount = vertexCount;    
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

    ctx->DrawIndexedInstanced(indexCount, instCount, startIndex, firstVertex, baseInstance);

    StatSet::IncStat(stat_DIP, 1);
#else
    SWCommand_DrawInstancedIndexedPrimitive* cmd = cb->allocCmd<SWCommand_DrawInstancedIndexedPrimitive>();
    cb->_ApplyTopology(type, count, &indexCount);

    cmd->mode = cb->cur_topo;
    cmd->indexCount = indexCount;
    cmd->instanceCount = instCount;
    cmd->firstVertex = firstVertex;
    cmd->instanceCount = instCount;
    cmd->baseInstance = baseInstance;
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
    if (!SyncObjectPoolDX11::IsAlive(obj))
        return true;

    bool signaled = false;
    SyncObjectDX11_t* sync = SyncObjectPoolDX11::Get(obj);

    if (sync)
        signaled = sync->is_signaled;

    return signaled;
}

//------------------------------------------------------------------------------

static void dx11_ExecuteQueuedCommands(const CommonImpl::Frame& frame)
{
    DAVA_CPU_PROFILER_SCOPE("rhi::ExecuteQueuedCmds");

    DVASSERT(frame.readyToExecute);

    StatSet::ResetAll();

    Trace("rhi-dx11.exec-queued-cmd\n");

    std::vector<RenderPassDX11_t*> pass;
    Handle perfQuerySet = InvalidHandle;
    unsigned frame_n = 0;

    for (Handle p : frame.pass)
    {
        RenderPassDX11_t* pp = RenderPassPoolDX11::Get(p);
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

    frame_n = frame.frameNumber;

    perfQuerySet = frame.perfQuerySet;

    if (frame.sync != InvalidHandle)
    {
        SyncObjectDX11_t* sync = SyncObjectPoolDX11::Get(frame.sync);

        sync->frame = frame_n;
        sync->is_signaled = false;
        sync->is_used = true;
    }

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
    pendingSecondaryCmdListSync.Lock();
    for (ID3D11CommandList* cmdList : pendingSecondaryCmdLists)
    {
        _D3D11_ImmediateContext->ExecuteCommandList(cmdList, FALSE);
        cmdList->Release();
    }
    pendingSecondaryCmdLists.clear();
    pendingSecondaryCmdListSync.Unlock();
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
            cb->Execute();

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

    if (perfQuerySet != InvalidHandle && !_DX11_PerfQuerySetPending)
    {
        PerfQuerySetDX11::IssueFrameEndQuery(perfQuerySet, _D3D11_ImmediateContext);
        PerfQuerySetDX11::EndFreqMeasurment(perfQuerySet, _D3D11_ImmediateContext);
        _DX11_PerfQuerySetPending = true;
    }

    for (Handle p : frame.pass)
        RenderPassPoolDX11::Free(p);

    for (SyncObjectPoolDX11::Iterator s = SyncObjectPoolDX11::Begin(), s_end = SyncObjectPoolDX11::End(); s != s_end; ++s)
    {
        if (s->is_used && (frame_n - s->frame >= 2))
            s->is_signaled = true;
    }
}

bool dx11_PresentBuffer()
{
    // do present
    HRESULT hr = _D3D11_SwapChain->Present(1, 0);
    CHECK_HR(hr)
    if (hr == DXGI_ERROR_DEVICE_REMOVED)
    {
        CHECK_HR(_D3D11_Device->GetDeviceRemovedReason())
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------

static void dx11_ExecImmediateCommand(CommonImpl::ImmediateCommand* command)
{
    DX11Command* commandData = reinterpret_cast<DX11Command*>(command->cmdData);
    for (DX11Command *cmd = commandData, *cmdEnd = commandData + command->cmdCount; cmd != cmdEnd; ++cmd)
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
}

//------------------------------------------------------------------------------

void ExecDX11(DX11Command* command, uint32 cmdCount, bool force_immediate)
{
    CommonImpl::ImmediateCommand cmd;
    cmd.cmdData = command;
    cmd.cmdCount = cmdCount;
    cmd.forceImmediate = force_immediate;
    RenderLoop::IssueImmediateCommand(&cmd);
}

static void dx11_EndFrame()
{
#if RHI_DX11__USE_DEFERRED_CONTEXTS
    ID3D11CommandList* cmdList = nullptr;
    CHECK_HR(_D3D11_SecondaryContext->FinishCommandList(TRUE, &cmdList))
    pendingSecondaryCmdListSync.Lock();
    pendingSecondaryCmdLists.push_back(cmdList);
    pendingSecondaryCmdListSync.Unlock();
#else
    ConstBufferDX11::InvalidateAllInstances();
#endif
}

static void dx11_RejectFrame(const CommonImpl::Frame& frame)
{
    if (frame.sync != InvalidHandle)
    {
        SyncObjectDX11_t* s = SyncObjectPoolDX11::Get(frame.sync);
        s->is_signaled = true;
        s->is_used = true;
    }

    for (Handle p : frame.pass)
    {
        RenderPassDX11_t* pp = RenderPassPoolDX11::Get(p);
        for (std::vector<Handle>::iterator b = pp->cmdBuf.begin(), b_end = pp->cmdBuf.end(); b != b_end; ++b)
        {
            CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(*b);

            if (cb->sync != InvalidHandle)
            {
                SyncObjectDX11_t* s = SyncObjectPoolDX11::Get(cb->sync);
                s->is_signaled = true;
                s->is_used = true;
            }

#if RHI_DX11__USE_DEFERRED_CONTEXTS
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
#endif
            CommandBufferPoolDX11::Free(*b);
        }
        RenderPassPoolDX11::Free(p);
    }
}

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
        cur_rs = dx11_GetRasterizerState(rs_param);
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
    DAVA_CPU_PROFILER_SCOPE("cb::Execute");

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

    if (isLastInPass && passCfg.UsingMSAA())
    {
        TextureDX11::ResolveMultisampling(passCfg.colorBuffer[0].multisampleTexture,
                                          passCfg.colorBuffer[0].texture, _D3D11_ImmediateContext);
    }

    commandList->Release();
    commandList = nullptr;

    
#else

    for (const uint8 *c = cmdData, *c_end = cmdData + curUsedSize; c != c_end;)
    {
        const SWCommand* cmd = (const SWCommand*)c;

        switch (SoftwareCommandType(cmd->type))
        {
        case CMD_BEGIN:
        {
            Reset();
            Begin(_D3D11_ImmediateContext);
        }
        break;

        case CMD_END:
        {
            sync = static_cast<const SWCommand_End*>(cmd)->syncObject;
        }
        break;

        case CMD_SET_VERTEX_DATA:
        {
            Handle vb = static_cast<const SWCommand_SetVertexData*>(cmd)->vb;
            unsigned stream_i = static_cast<const SWCommand_SetVertexData*>(cmd)->streamIndex;

            cur_vb[stream_i] = vb;
            if (!cur_vb_stride[stream_i])
                cur_vb_stride[stream_i] = PipelineStateDX11::VertexLayoutStride(cur_pipelinestate, stream_i);
        }
        break;

        case CMD_SET_INDICES:
        {
            Handle ib = static_cast<const SWCommand_SetIndices*>(cmd)->ib;
            IndexBufferDX11::SetToRHI(ib, 0, _D3D11_ImmediateContext);
        }
        break;

        case CMD_SET_QUERY_BUFFER:
        {
            cur_query_buf = static_cast<const SWCommand_SetQueryBuffer*>(cmd)->queryBuf;
        }
        break;

        case CMD_SET_QUERY_INDEX:
        {
            if (cur_query_buf != InvalidHandle)
                QueryBufferDX11::SetQueryIndex(cur_query_buf, static_cast<const SWCommand_SetQueryIndex*>(cmd)->objectIndex, context);
        }
        break;

        case CMD_ISSUE_TIMESTAMP_QUERY:
        {
            Handle hset = static_cast<const SWCommand_IssueTimestamptQuery*>(cmd)->querySet;
            uint32 timestampIndex = static_cast<const SWCommand_IssueTimestamptQuery*>(cmd)->timestampIndex;

            PerfQuerySetDX11::IssueTimestampQuery(hset, timestampIndex, _D3D11_ImmediateContext);
        }
        break;

        case CMD_SET_PIPELINE_STATE:
        {
            Handle ps = static_cast<const SWCommand_SetPipelineState*>(cmd)->ps;
            Handle vdeclUID = static_cast<const SWCommand_SetPipelineState*>(cmd)->vdecl;
            const VertexLayout* vdecl = (vdeclUID == VertexLayout::InvalidUID) ? nullptr : VertexLayout::Get(vdeclUID);

            cur_pipelinestate = ps;
            cur_stream_count = PipelineStateDX11::VertexLayoutStreamCount(ps);
            for (unsigned i = 0; i != cur_stream_count; ++i)
                cur_vb_stride[i] = (vdecl) ? vdecl->Stride(i) : 0;

            PipelineStateDX11::SetToRHI(ps, vdeclUID, _D3D11_ImmediateContext);
        }
        break;

        case CMD_SET_CULL_MODE:
        {
            rs_param.cullMode = CullMode(static_cast<const SWCommand_SetCullMode*>(cmd)->mode);
            cur_rs = nullptr;
        }
        break;

        case CMD_SET_SCISSOR_RECT:
        {
            int x = static_cast<const SWCommand_SetScissorRect*>(cmd)->x;
            int y = static_cast<const SWCommand_SetScissorRect*>(cmd)->y;
            int w = static_cast<const SWCommand_SetScissorRect*>(cmd)->width;
            int h = static_cast<const SWCommand_SetScissorRect*>(cmd)->height;

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

        case CMD_SET_VIEWPORT:
        {
            int x = static_cast<const SWCommand_SetViewport*>(cmd)->x;
            int y = static_cast<const SWCommand_SetViewport*>(cmd)->y;
            int w = static_cast<const SWCommand_SetViewport*>(cmd)->width;
            int h = static_cast<const SWCommand_SetViewport*>(cmd)->height;

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

        case CMD_SET_FILLMODE:
        {
            rs_param.wireframe = FillMode(static_cast<const SWCommand_SetFillMode*>(cmd)->mode) == FILLMODE_WIREFRAME;
        }
        break;

        case CMD_SET_VERTEX_PROG_CONST_BUFFER:
        {
            Handle buffer = static_cast<const SWCommand_SetVertexProgConstBuffer*>(cmd)->buffer;
            const void* inst = static_cast<const SWCommand_SetVertexProgConstBuffer*>(cmd)->inst;

            ConstBufferDX11::SetToRHI(buffer, inst);
        }
        break;

        case CMD_SET_FRAGMENT_PROG_CONST_BUFFER:
        {
            Handle buffer = static_cast<const SWCommand_SetFragmentProgConstBuffer*>(cmd)->buffer;
            const void* inst = static_cast<const SWCommand_SetFragmentProgConstBuffer*>(cmd)->inst;

            ConstBufferDX11::SetToRHI(buffer, inst);
        }
        break;

        case CMD_SET_FRAGMENT_TEXTURE:
        {
            Handle tex = static_cast<const SWCommand_SetFragmentTexture*>(cmd)->tex;
            unsigned unitIndex = static_cast<const SWCommand_SetFragmentTexture*>(cmd)->unitIndex;
            TextureDX11::SetToRHIFragment(tex, unitIndex, _D3D11_ImmediateContext);
        }
        break;

        case CMD_SET_VERTEX_TEXTURE:
        {
            Handle tex = static_cast<const SWCommand_SetVertexTexture*>(cmd)->tex;
            unsigned unitIndex = static_cast<const SWCommand_SetVertexTexture*>(cmd)->unitIndex;
            TextureDX11::SetToRHIVertex(tex, unitIndex, _D3D11_ImmediateContext);
        }
        break;

        case CMD_SET_DEPTHSTENCIL_STATE:
        {
            DepthStencilStateDX11::SetToRHI(static_cast<const SWCommand_SetDepthStencilState*>(cmd)->depthStencilState, _D3D11_ImmediateContext);
        }
        break;

        case CMD_SET_SAMPLER_STATE:
        {
            SamplerStateDX11::SetToRHI(static_cast<const SWCommand_SetSamplerState*>(cmd)->samplerState, _D3D11_ImmediateContext);
        }
        break;

        case CMD_DRAW_PRIMITIVE:
        {
            D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY(static_cast<const SWCommand_DrawPrimitive*>(cmd)->mode);
            unsigned vertexCount = static_cast<const SWCommand_DrawPrimitive*>(cmd)->vertexCount;

            if (topo != cur_topo)
            {
                _D3D11_ImmediateContext->IASetPrimitiveTopology(topo);
                cur_topo = topo;
            }

            if (!cur_rs)
            {
                cur_rs = dx11_GetRasterizerState(rs_param);
                _D3D11_ImmediateContext->RSSetState(cur_rs);
            }

            for (unsigned s = 0; s != cur_stream_count; ++s)
                VertexBufferDX11::SetToRHI(cur_vb[s], s, 0, cur_vb_stride[s], _D3D11_ImmediateContext);

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferDX11::BeginQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);

            _D3D11_ImmediateContext->Draw(vertexCount, 0);

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferDX11::EndQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);
        }
        break;

        case CMD_DRAW_INDEXED_PRIMITIVE:
        {
            D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY(static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd)->mode);
            unsigned baseVertex = static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd)->firstVertex;
            unsigned indexCount = static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd)->indexCount;
            unsigned startIndex = static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd)->startIndex;

            if (topo != cur_topo)
            {
                _D3D11_ImmediateContext->IASetPrimitiveTopology(topo);
                cur_topo = topo;
            }

            if (!cur_rs)
            {
                cur_rs = dx11_GetRasterizerState(rs_param);
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

        case CMD_DRAW_INSTANCED_PRIMITIVE:
        {
            D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY(static_cast<const SWCommand_DrawInstancedPrimitive*>(cmd)->mode);
            unsigned vertexCount = static_cast<const SWCommand_DrawInstancedPrimitive*>(cmd)->vertexCount;
            unsigned instCount = static_cast<const SWCommand_DrawInstancedPrimitive*>(cmd)->instanceCount;

            if (topo != cur_topo)
            {
                _D3D11_ImmediateContext->IASetPrimitiveTopology(topo);
                cur_topo = topo;
            }

            if (!cur_rs)
            {
                cur_rs = dx11_GetRasterizerState(rs_param);
                _D3D11_ImmediateContext->RSSetState(cur_rs);
            }

            for (unsigned s = 0; s != cur_stream_count; ++s)
                VertexBufferDX11::SetToRHI(cur_vb[s], s, 0, cur_vb_stride[s], _D3D11_ImmediateContext);

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferDX11::BeginQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);

            _D3D11_ImmediateContext->DrawInstanced(vertexCount, instCount, 0, 0);

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferDX11::EndQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);
        }
        break;

        case CMD_DRAW_INSTANCED_INDEXED_PRIMITIVE:
        {
            D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY(static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->mode);
            unsigned vertexCount = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->indexCount;
            unsigned indexCount = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->indexCount;
            unsigned startIndex = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->startIndex;
            unsigned instCount = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->instanceCount;
            unsigned baseInst = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->baseInstance;

            if (topo != cur_topo)
            {
                _D3D11_ImmediateContext->IASetPrimitiveTopology(topo);
                cur_topo = topo;
            }

            if (!cur_rs)
            {
                cur_rs = dx11_GetRasterizerState(rs_param);
                _D3D11_ImmediateContext->RSSetState(cur_rs);
            }

            for (unsigned s = 0; s != cur_stream_count; ++s)
                VertexBufferDX11::SetToRHI(cur_vb[s], s, 0, cur_vb_stride[s], _D3D11_ImmediateContext);

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferDX11::BeginQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);

            _D3D11_ImmediateContext->DrawIndexedInstanced(indexCount, instCount, startIndex, 0, baseInst);

            if (cur_query_i != DAVA::InvalidIndex)
                QueryBufferDX11::BeginQuery(cur_query_buf, cur_query_i, _D3D11_ImmediateContext);
        }
        break;
        default:
            Logger::Error("unsupported command: %d", cmd->type);
            DVASSERT_MSG(false, "unsupported command");
        }

        if (cmd->type == CMD_END)
            break;
        c += cmd->size;
    }
#endif
}

void CommandBufferDX11_t::Begin(ID3D11DeviceContext* context)
{
    bool clear_color = isFirstInPass && passCfg.colorBuffer[0].loadAction == LOADACTION_CLEAR;
    bool clear_depth = isFirstInPass && passCfg.depthStencilBuffer.loadAction == LOADACTION_CLEAR;

    sync = InvalidHandle;

    def_viewport.TopLeftX = 0;
    def_viewport.TopLeftY = 0;
    def_viewport.MinDepth = 0.0f;
    def_viewport.MaxDepth = 1.0f;

    const RenderPassConfig::ColorBuffer& color0 = passCfg.colorBuffer[0];
    if ((color0.texture != rhi::InvalidHandle) && (color0.texture != rhi::DefaultDepthBuffer))
    {
        Handle targetTexture = color0.texture;
        Handle targetDepth = passCfg.depthStencilBuffer.texture;
        if (passCfg.UsingMSAA())
        {
            targetTexture = color0.multisampleTexture;
            targetDepth = passCfg.depthStencilBuffer.multisampleTexture;
        }
        TextureDX11::SetRenderTarget(targetTexture, targetDepth, color0.textureLevel, color0.textureFace, context);

        Size2i sz = TextureDX11::Size(color0.texture);
        def_viewport.Width = static_cast<float>(sz.dx);
        def_viewport.Height = static_cast<float>(sz.dy);
    }
    else if (passCfg.UsingMSAA())
    {
        TextureDX11::SetRenderTarget(color0.multisampleTexture, passCfg.depthStencilBuffer.multisampleTexture, color0.textureLevel, color0.textureFace, context);
        Size2i sz = TextureDX11::Size(color0.multisampleTexture);
        def_viewport.Width = static_cast<float>(sz.dx);
        def_viewport.Height = static_cast<float>(sz.dy);
    }
    else
    {
        context->OMSetRenderTargets(1, &_D3D11_RenderTargetView, _D3D11_DepthStencilView);
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

    DVASSERT(!isFirstInPass || cur_query_buf == InvalidHandle || !QueryBufferDX11::QueryIsCompleted(cur_query_buf));
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

    dispatch->impl_ProcessImmediateCommand = &dx11_ExecImmediateCommand;
    dispatch->impl_ExecuteFrame = &dx11_ExecuteQueuedCommands;
    dispatch->impl_RejectFrame = &dx11_RejectFrame;
    dispatch->impl_PresentBuffer = &dx11_PresentBuffer;
    dispatch->impl_FinishFrame = &dx11_EndFrame;
}
}

//==============================================================================
} // namespace rhi
