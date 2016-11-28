#include "../Common/rhi_Pool.h"
#include "../Common/rhi_Utils.h"
#include "../Common/rhi_RingBuffer.h"
#include "../Common/dbg_StatSet.h"
#include "../Common/SoftwareCommandBuffer.h"
#include "../Common/RenderLoop.h"
#include "../Common/rhi_CommonImpl.h"
#include "../rhi_Type.h"
#include "rhi_DX11.h"
#include "Core/Core.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Platform/SystemTimer.h"
#include <vector>

#define LUMIA_1020_DEPTHBUF_WORKAROUND 1

#if LUMIA_1020_DEPTHBUF_WORKAROUND
#include "Platform/DeviceInfo.h"
#endif

namespace rhi
{
struct RasterizerParamDX11
{
    union
    {
        struct
        {
            uint32 cullMode : 3;
            uint32 scissorEnabled : 1;
            uint32 wireframe : 1;
            uint32 pad : 27;
        };
        uint32 value;
    };

    RasterizerParamDX11()
        : value(0)
    {
    }

    bool operator==(const RasterizerParamDX11& b) const
    {
        return value == b.value;
    }
};

struct RasterizerStateDX11
{
    RasterizerParamDX11 param;
    ID3D11RasterizerState* state = nullptr;

    RasterizerStateDX11(const RasterizerParamDX11& p, ID3D11RasterizerState* st)
        : param(p)
        , state(st)
    {
    }

    bool operator==(const RasterizerParamDX11& p) const
    {
        return param == p;
    }
};

struct RenderPassDX11_t
{
    DAVA::Vector<Handle> commandBuffers;
    int priority;
    Handle perfQueryStart;
    Handle perfQueryEnd;
};

struct SyncObjectDX11_t
{
    uint32 frame;
    uint32 is_signaled : 1;
    uint32 is_used : 1;
    uint32 pad : 30;
};

class CommandBufferDX11_t : public SoftwareCommandBuffer
{
public: // shared
    void Begin(ID3D11DeviceContext* context);
    void Reset();
    void Execute();
    void ApplyTopology(PrimitiveType primType, uint32 primCount, uint32* indexCount);

public: // deferred context
    void HWApplyVertexData();
    void HWApplyRasterizerState();
    void HWApplyConstBuffers();
    void ExecuteHardwareBuffers();

    ID3D11Buffer* vertexConstBuffer[MAX_CONST_BUFFER_COUNT];
    ID3D11Buffer* fragmentConstBuffer[MAX_CONST_BUFFER_COUNT];
    DAVA::Vector<Handle> deferredPerfQueries;

public: // software command buffers
    void ExecuteSoftwareBuffers();

public:
    ID3D11RasterizerState* cur_rs = nullptr;
    ID3D11RasterizerState* last_rs = nullptr;
    ID3D11DeviceContext* context = nullptr;
    ID3DUserDefinedAnnotation* contextAnnotation = nullptr;
    ID3D11CommandList* commandList = nullptr;

    RenderPassConfig passCfg;
    RasterizerParamDX11 rs_param;
    D3D11_PRIMITIVE_TOPOLOGY cur_topo;
    D3D11_VIEWPORT def_viewport;

    Handle last_vb[MAX_VERTEX_STREAM_COUNT];
    Handle cur_vb[MAX_VERTEX_STREAM_COUNT];
    Handle cur_query_buf;
    Handle cur_pipelinestate;
    Handle last_ps;
    Handle sync;

    uint32 last_vb_stride[MAX_VERTEX_STREAM_COUNT];
    uint32 cur_vb_stride[MAX_VERTEX_STREAM_COUNT];
    uint32 cur_stream_count;
    uint32 cur_stride;
    uint32 last_vdecl;
    uint32 isFirstInPass : 1;
    uint32 isLastInPass : 1;
    uint32 isComplete : 1;
};

typedef ResourcePool<CommandBufferDX11_t, RESOURCE_COMMAND_BUFFER, CommandBuffer::Descriptor, false> CommandBufferPoolDX11;
typedef ResourcePool<RenderPassDX11_t, RESOURCE_RENDER_PASS, RenderPassConfig, false> RenderPassPoolDX11;
typedef ResourcePool<SyncObjectDX11_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false> SyncObjectPoolDX11;

RHI_IMPL_POOL(CommandBufferDX11_t, RESOURCE_COMMAND_BUFFER, CommandBuffer::Descriptor, false);
RHI_IMPL_POOL(RenderPassDX11_t, RESOURCE_RENDER_PASS, RenderPassConfig, false);
RHI_IMPL_POOL(SyncObjectDX11_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false);

static DAVA::Mutex _DX11_SyncObjectsSync;
static DAVA::Vector<ID3D11CommandList*> pendingSecondaryCmdLists;
static DAVA::Mutex pendingSecondaryCmdListSync;
static std::vector<RasterizerStateDX11> _RasterizerStateDX11;

static ID3D11RasterizerState* dx11_GetRasterizerState(RasterizerParamDX11 param)
{
    auto existing = std::find(_RasterizerStateDX11.begin(), _RasterizerStateDX11.end(), param);
    if (existing != _RasterizerStateDX11.end())
        return existing->state;

    D3D11_RASTERIZER_DESC desc = {};
    desc.FillMode = (param.wireframe) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
    desc.FrontCounterClockwise = FALSE;
    desc.DepthBias = 0;
    desc.DepthBiasClamp = 0;
    desc.SlopeScaledDepthBias = 0.0f;
    desc.DepthClipEnable = TRUE;
    desc.ScissorEnable = param.scissorEnabled;
    desc.MultisampleEnable = FALSE;
    desc.AntialiasedLineEnable = FALSE;

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
        default:
            DVASSERT_MSG(0, "Invalid CullMode provided");
    }

    ID3D11RasterizerState* state = nullptr;
    HRESULT hr = DX11DeviceCommand(DX11Command::CREATE_RASTERIZER_STATE, &desc, &state);
    if (SUCCEEDED(hr) && (state != nullptr))
        _RasterizerStateDX11.emplace_back(param, state);

    return state;
}

static Handle dx11_RenderPass_Allocate(const RenderPassConfig& passDesc, uint32 cmdBufCount, Handle* cmdBuf)
{
    DVASSERT(cmdBufCount);
    DVASSERT(passDesc.IsValid());

    Handle handle = RenderPassPoolDX11::Alloc();
    RenderPassDX11_t* pass = RenderPassPoolDX11::Get(handle);

    pass->commandBuffers.resize(cmdBufCount);
    pass->priority = passDesc.priority;
    pass->perfQueryStart = passDesc.perfQueryStart;
    pass->perfQueryEnd = passDesc.perfQueryEnd;

    for (uint32 i = 0; i != cmdBufCount; ++i)
    {
        Handle h = CommandBufferPoolDX11::Alloc();
        CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(h);
        cb->passCfg = passDesc;
        cb->isFirstInPass = i == 0;
        cb->isLastInPass = i == cmdBufCount - 1;
        cb->commandList = nullptr;

        if (_DX11_UseHardwareCommandBuffers && (cb->context == nullptr))
        {
            HRESULT hr = DX11DeviceCommand(DX11Command::CREATE_DEFERRED_CONTEXT, 0, &cb->context);
            if (SUCCEEDED(hr))
            {
                cb->context->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)(&(cb->contextAnnotation)));
            }
        }

        pass->commandBuffers[i] = h;
        cmdBuf[i] = h;
    }

    return handle;
}

static void dx11_RenderPass_Begin(Handle pass)
{
}

static void dx11_RenderPass_End(Handle pass)
{
}

void RenderPassDX11::SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_Renderpass_Allocate = &dx11_RenderPass_Allocate;
    dispatch->impl_Renderpass_Begin = &dx11_RenderPass_Begin;
    dispatch->impl_Renderpass_End = &dx11_RenderPass_End;
}

static void dx11_CommandBuffer_Begin(Handle cmdBuf)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    if (_DX11_UseHardwareCommandBuffers)
    {
        cb->Reset();
        cb->Begin(cb->context);
    }
    else
    {
        cb->curUsedSize = 0;
        cb->allocCmd<SWCommand_Begin>();
    }
}

static void dx11_CommandBuffer_End(Handle cmdBuf, Handle syncObject)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    if (_DX11_UseHardwareCommandBuffers)
    {
        if (cb->isLastInPass && cb->cur_query_buf != InvalidHandle)
            QueryBufferDX11::QueryComplete(cb->cur_query_buf, cb->context);

        CHECK_HR(cb->context->FinishCommandList(TRUE, &cb->commandList));
        cb->sync = syncObject;
        cb->isComplete = true;
    }
    else
    {
        SWCommand_End* cmd = cb->allocCmd<SWCommand_End>();
        cmd->syncObject = syncObject;
    }
}

static void dx11_CommandBuffer_SetPipelineState(Handle cmdBuf, Handle ps, uint32 vdeclUID)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    if (_DX11_UseHardwareCommandBuffers)
    {
        const VertexLayout* vdecl = (vdeclUID == VertexLayout::InvalidUID) ? nullptr : VertexLayout::Get(vdeclUID);

        cb->cur_pipelinestate = ps;
        cb->cur_stream_count = PipelineStateDX11::VertexLayoutStreamCount(ps);
        for (uint32 i = 0; i != cb->cur_stream_count; ++i)
            cb->cur_vb_stride[i] = (vdecl) ? vdecl->Stride(i) : 0;

        if (ps != cb->last_ps || vdeclUID != cb->last_vdecl)
        {
            PipelineStateDX11::SetToRHI(ps, vdeclUID, cb->context);
            cb->last_ps = ps;
            cb->last_vdecl = vdeclUID;
            StatSet::IncStat(stat_SET_PS, 1);
        }
    }
    else
    {
        SWCommand_SetPipelineState* cmd = cb->allocCmd<SWCommand_SetPipelineState>();
        cmd->ps = ps;
        cmd->vdecl = vdeclUID;
    }
}

static void dx11_CommandBuffer_SetCullMode(Handle cmdBuf, CullMode mode)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    if (_DX11_UseHardwareCommandBuffers)
    {
        cb->rs_param.cullMode = mode;
        cb->cur_rs = nullptr;
    }
    else
    {
        SWCommand_SetCullMode* cmd = cb->allocCmd<SWCommand_SetCullMode>();
        cmd->mode = mode;
    }
}

void dx11_CommandBuffer_SetScissorRect(Handle cmdBuf, ScissorRect rect)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    int x = rect.x;
    int y = rect.y;
    int w = rect.width;
    int h = rect.height;

    if (_DX11_UseHardwareCommandBuffers)
    {
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
    }
    else
    {
        SWCommand_SetScissorRect* cmd = cb->allocCmd<SWCommand_SetScissorRect>();
        cmd->x = x;
        cmd->y = y;
        cmd->width = w;
        cmd->height = h;
    }
}

static void dx11_CommandBuffer_SetViewport(Handle cmdBuf, Viewport vp)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    int x = vp.x;
    int y = vp.y;
    int w = vp.width;
    int h = vp.height;

    if (_DX11_UseHardwareCommandBuffers)
    {
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
    }
    else
    {
        SWCommand_SetViewport* cmd = cb->allocCmd<SWCommand_SetViewport>();
        cmd->x = x;
        cmd->y = y;
        cmd->width = w;
        cmd->height = h;
    }
}

static void dx11_CommandBuffer_SetFillMode(Handle cmdBuf, FillMode mode)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    if (_DX11_UseHardwareCommandBuffers)
    {
        cb->rs_param.wireframe = (mode == FILLMODE_WIREFRAME);
    }
    else
    {
        SWCommand_SetFillMode* cmd = cb->allocCmd<SWCommand_SetFillMode>();
        cmd->mode = mode;
    }
}

static void dx11_CommandBuffer_SetVertexData(Handle cmdBuf, Handle vb, uint32 streamIndex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    if (_DX11_UseHardwareCommandBuffers)
    {
        cb->cur_vb[streamIndex] = vb;
        if (cb->cur_vb_stride[streamIndex] == 0)
            cb->cur_vb_stride[streamIndex] = PipelineStateDX11::VertexLayoutStride(cb->cur_pipelinestate, streamIndex);
    }
    else
    {
        SWCommand_SetVertexData* cmd = cb->allocCmd<SWCommand_SetVertexData>();
        cmd->vb = vb;
        cmd->streamIndex = streamIndex;
    }
}

static void dx11_CommandBuffer_SetVertexConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    if (_DX11_UseHardwareCommandBuffers)
    {
        ConstBufferDX11::SetToRHI(buffer, cb->context, cb->vertexConstBuffer);
    }
    else
    {
        SWCommand_SetVertexProgConstBuffer* cmd = cb->allocCmd<SWCommand_SetVertexProgConstBuffer>();
        cmd->bufIndex = bufIndex;
        cmd->buffer = buffer;
        cmd->inst = ConstBufferDX11::Instance(buffer);
    }
}

static void dx11_CommandBuffer_SetVertexTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    if (_DX11_UseHardwareCommandBuffers)
    {
        TextureDX11::SetToRHIVertex(tex, unitIndex, cb->context);
        StatSet::IncStat(stat_SET_TEX, 1);
    }
    else
    {
        SWCommand_SetVertexTexture* cmd = cb->allocCmd<SWCommand_SetVertexTexture>();
        cmd->unitIndex = unitIndex;
        cmd->tex = tex;
    }
}

static void dx11_CommandBuffer_SetIndices(Handle cmdBuf, Handle ib)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    if (_DX11_UseHardwareCommandBuffers)
    {
        IndexBufferDX11::SetToRHI(ib, 0, cb->context);
    }
    else
    {
        SWCommand_SetIndices* cmd = cb->allocCmd<SWCommand_SetIndices>();
        cmd->ib = ib;
    }
}

static void dx11_CommandBuffer_SetQueryIndex(Handle cmdBuf, uint32 objectIndex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    if (_DX11_UseHardwareCommandBuffers)
    {
        if (cb->cur_query_buf != InvalidHandle)
            QueryBufferDX11::SetQueryIndex(cb->cur_query_buf, objectIndex, cb->context);
    }
    else
    {
        SWCommand_SetQueryIndex* cmd = cb->allocCmd<SWCommand_SetQueryIndex>();
        cmd->objectIndex = objectIndex;
    }
}

static void dx11_CommandBuffer_SetQueryBuffer(Handle cmdBuf, Handle queryBuf)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    DVASSERT(cb->cur_query_buf == InvalidHandle);

    if (_DX11_UseHardwareCommandBuffers)
    {
        cb->cur_query_buf = queryBuf;
    }
    else
    {
        SWCommand_SetQueryBuffer* cmd = cb->allocCmd<SWCommand_SetQueryBuffer>();
        cmd->queryBuf = queryBuf;
    }
}

static void dx11_CommandBuffer_IssueTimestampQuery(Handle cmdBuf, Handle perfQuery)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    if (_DX11_UseHardwareCommandBuffers)
    {
        PerfQueryDX11::IssueTimestampQueryDeferred(perfQuery, cb->context);
        cb->deferredPerfQueries.push_back(perfQuery);
    }
    else
    {
        SWCommand_IssueTimestamptQuery* cmd = cb->allocCmd<SWCommand_IssueTimestamptQuery>();
        cmd->perfQuery = perfQuery;
    }
}

static void dx11_CommandBuffer_SetFragmentConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    if (_DX11_UseHardwareCommandBuffers)
    {
        ConstBufferDX11::SetToRHI(buffer, cb->context, cb->fragmentConstBuffer);
    }
    else
    {
        SWCommand_SetFragmentProgConstBuffer* cmd = cb->allocCmd<SWCommand_SetFragmentProgConstBuffer>();
        cmd->bufIndex = bufIndex;
        cmd->buffer = buffer;
        cmd->inst = ConstBufferDX11::Instance(buffer);
    }
}

static void dx11_CommandBuffer_SetFragmentTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    if (_DX11_UseHardwareCommandBuffers)
    {
        TextureDX11::SetToRHIFragment(tex, unitIndex, cb->context);
        StatSet::IncStat(stat_SET_TEX, 1);
    }
    else
    {
        SWCommand_SetFragmentTexture* cmd = cb->allocCmd<SWCommand_SetFragmentTexture>();
        cmd->unitIndex = unitIndex;
        cmd->tex = tex;
    }
}

static void dx11_CommandBuffer_SetDepthStencilState(Handle cmdBuf, Handle depthStencilState)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    if (_DX11_UseHardwareCommandBuffers)
    {
        DepthStencilStateDX11::SetToRHI(depthStencilState, cb->context);
    }
    else
    {
        SWCommand_SetDepthStencilState* cmd = cb->allocCmd<SWCommand_SetDepthStencilState>();
        cmd->depthStencilState = depthStencilState;
    }
}

static void dx11_CommandBuffer_SetSamplerState(Handle cmdBuf, const Handle samplerState)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    if (_DX11_UseHardwareCommandBuffers)
    {
        SamplerStateDX11::SetToRHI(samplerState, cb->context);
        StatSet::IncStat(stat_SET_SS, 1);
    }
    else
    {
        SWCommand_SetSamplerState* cmd = cb->allocCmd<SWCommand_SetSamplerState>();
        cmd->samplerState = samplerState;
    }
}

static void dx11_CommandBuffer_DrawPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count)
{
    uint32 vertexCount = 0;
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    cb->ApplyTopology(type, count, &vertexCount);

    if (_DX11_UseHardwareCommandBuffers)
    {
        INT baseVertex = 0;
        cb->HWApplyVertexData();
        cb->HWApplyRasterizerState();
        cb->HWApplyConstBuffers();
        cb->context->Draw(vertexCount, baseVertex);
        StatSet::IncStat(stat_DIP, 1);
    }
    else
    {
        SWCommand_DrawPrimitive* cmd = cb->allocCmd<SWCommand_DrawPrimitive>();
        cmd->mode = cb->cur_topo;
        cmd->vertexCount = vertexCount;
    }
}

static void dx11_CommandBuffer_DrawIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count, uint32 vertexCount, uint32 firstVertex, uint32 startIndex)
{
    uint32 indexCount = 0;
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    cb->ApplyTopology(type, count, &indexCount);

    if (_DX11_UseHardwareCommandBuffers)
    {
        cb->HWApplyVertexData();
        cb->HWApplyRasterizerState();
        cb->HWApplyConstBuffers();
        cb->context->DrawIndexed(indexCount, startIndex, firstVertex);
        StatSet::IncStat(stat_DIP, 1);
    }
    else
    {
        SWCommand_DrawIndexedPrimitive* cmd = cb->allocCmd<SWCommand_DrawIndexedPrimitive>();
        cmd->mode = cb->cur_topo;
        cmd->firstVertex = firstVertex;
        cmd->indexCount = indexCount;
        cmd->startIndex = startIndex;
    }
}

static void dx11_CommandBuffer_DrawInstancedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 instCount, uint32 count)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    INT baseVertex = 0;
    uint32 vertexCount = 0;
    cb->ApplyTopology(type, count, &vertexCount);

    if (_DX11_UseHardwareCommandBuffers)
    {
        cb->HWApplyVertexData();
        cb->HWApplyRasterizerState();
        cb->HWApplyConstBuffers();
        cb->context->DrawInstanced(vertexCount, instCount, baseVertex, 0);
        StatSet::IncStat(stat_DIP, 1);
    }
    else
    {
        SWCommand_DrawInstancedPrimitive* cmd = cb->allocCmd<SWCommand_DrawInstancedPrimitive>();
        cmd->mode = cb->cur_topo;
        cmd->instanceCount = instCount;
        cmd->vertexCount = vertexCount;
    }
}

static void dx11_CommandBuffer_DrawInstancedIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 instCount, uint32 count, uint32 vertexCount, uint32 firstVertex, uint32 startIndex, uint32 baseInstance)
{
    uint32 indexCount = 0;
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    cb->ApplyTopology(type, count, &indexCount);

    if (_DX11_UseHardwareCommandBuffers)
    {
        cb->HWApplyVertexData();
        cb->HWApplyRasterizerState();
        cb->HWApplyConstBuffers();
        cb->context->DrawIndexedInstanced(indexCount, instCount, startIndex, firstVertex, baseInstance);
        StatSet::IncStat(stat_DIP, 1);
    }
    else
    {
        SWCommand_DrawInstancedIndexedPrimitive* cmd = cb->allocCmd<SWCommand_DrawInstancedIndexedPrimitive>();
        cmd->mode = cb->cur_topo;
        cmd->indexCount = indexCount;
        cmd->instanceCount = instCount;
        cmd->firstVertex = firstVertex;
        cmd->instanceCount = instCount;
        cmd->baseInstance = baseInstance;
    }
}

static void dx11_CommandBuffer_SetMarker(Handle cmdBuf, const char* text)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    if (_DX11_UseHardwareCommandBuffers && (cb->contextAnnotation != nullptr))
    {
        wchar_t txt[128] = {};
        MultiByteToWideChar(CP_ACP, 0, text, -1, txt, countof(txt));
        cb->contextAnnotation->SetMarker(txt);
    }
}

static Handle dx11_SyncObject_Create()
{
    DAVA::LockGuard<DAVA::Mutex> guard(_DX11_SyncObjectsSync);

    Handle handle = SyncObjectPoolDX11::Alloc();
    SyncObjectDX11_t* sync = SyncObjectPoolDX11::Get(handle);
    sync->is_signaled = false;
    sync->is_used = false;
    return handle;
}

static void dx11_SyncObject_Delete(Handle obj)
{
    DAVA::LockGuard<DAVA::Mutex> guard(_DX11_SyncObjectsSync);
    SyncObjectPoolDX11::Free(obj);
}

static bool dx11_SyncObject_IsSignaled(Handle obj)
{
    DAVA::LockGuard<DAVA::Mutex> guard(_DX11_SyncObjectsSync);
    if (!SyncObjectPoolDX11::IsAlive(obj))
        return true;

    bool signaled = false;
    SyncObjectDX11_t* sync = SyncObjectPoolDX11::Get(obj);

    if (sync)
        signaled = sync->is_signaled;

    return signaled;
}

static void dx11_ExecuteQueuedCommands(const CommonImpl::Frame& frame)
{
    DVASSERT(frame.readyToExecute);
    DVASSERT((frame.sync == InvalidHandle) || SyncObjectPoolDX11::IsAlive(frame.sync));

    StatSet::ResetAll();

    DAVA::Vector<RenderPassDX11_t*> pass;
    pass.reserve(frame.pass.size());
    for (Handle p : frame.pass)
    {
        RenderPassDX11_t* pp = RenderPassPoolDX11::Get(p);
        pass.emplace_back(pp);
    }
    std::stable_sort(pass.begin(), pass.end(), [](RenderPassDX11_t* l, RenderPassDX11_t* r) {
        // sort from highest to lowest priorities
        return l->priority > r->priority;
    });

    if (frame.sync != InvalidHandle)
    {
        SyncObjectDX11_t* sync = SyncObjectPoolDX11::Get(frame.sync);
        sync->frame = frame.frameNumber;
        sync->is_signaled = false;
        sync->is_used = true;
    }

    PerfQueryDX11::ObtainPerfQueryMeasurment(_D3D11_ImmediateContext);
    PerfQueryDX11::BeginMeasurment(_D3D11_ImmediateContext);

    if (frame.perfQueryStart != InvalidHandle)
        PerfQueryDX11::IssueTimestampQuery(frame.perfQueryStart, _D3D11_ImmediateContext);

    if (_DX11_UseHardwareCommandBuffers)
    {
        DAVA::LockGuard<DAVA::Mutex> lock(pendingSecondaryCmdListSync);
        for (ID3D11CommandList* cmdList : pendingSecondaryCmdLists)
        {
            _D3D11_ImmediateContext->ExecuteCommandList(cmdList, FALSE);
            cmdList->Release();
        }
        pendingSecondaryCmdLists.clear();
    }

    for (RenderPassDX11_t* pp : pass)
    {
        if (pp->perfQueryStart != InvalidHandle)
            PerfQueryDX11::IssueTimestampQuery(pp->perfQueryStart, _D3D11_ImmediateContext);

        for (Handle cb_h : pp->commandBuffers)
        {
            CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cb_h);
            cb->Execute();

            if (cb->isLastInPass && cb->passCfg.UsingMSAA())
            {
                const rhi::RenderPassConfig::ColorBuffer& colorBuffer = cb->passCfg.colorBuffer[0];
                TextureDX11::ResolveMultisampling(colorBuffer.multisampleTexture, colorBuffer.texture, _D3D11_ImmediateContext);
            }

            if (_DX11_UseHardwareCommandBuffers)
            {
                PerfQueryDX11::DeferredPerfQueriesIssued(cb->deferredPerfQueries);
            }

            if (cb->sync != InvalidHandle)
            {
                SyncObjectDX11_t* sync = SyncObjectPoolDX11::Get(cb->sync);
                sync->frame = frame.frameNumber;
                sync->is_signaled = false;
            }
            CommandBufferPoolDX11::Free(cb_h);
        }
        pp->commandBuffers.clear();

        if (pp->perfQueryEnd != InvalidHandle)
            PerfQueryDX11::IssueTimestampQuery(pp->perfQueryEnd, _D3D11_ImmediateContext);
    }

    for (Handle p : frame.pass)
        RenderPassPoolDX11::Free(p);

    if (frame.perfQueryEnd != InvalidHandle)
        PerfQueryDX11::IssueTimestampQuery(frame.perfQueryEnd, _D3D11_ImmediateContext);

    _DX11_SyncObjectsSync.Lock();
    for (SyncObjectPoolDX11::Iterator s = SyncObjectPoolDX11::Begin(), s_end = SyncObjectPoolDX11::End(); s != s_end; ++s)
    {
        if (s->is_used && (frame.frameNumber - s->frame >= 2))
            s->is_signaled = true;
    }
    _DX11_SyncObjectsSync.Unlock();
}

bool dx11_PresentBuffer()
{
    DX11_ProcessCallResult(_D3D11_SwapChain->Present(1, 0), __FUNCTION__, __FILE__, __LINE__);
    PerfQueryDX11::EndMeasurment(_D3D11_ImmediateContext);
    return true;
}

static void dx11_ExecImmediateCommand(CommonImpl::ImmediateCommand* command)
{
    DX11Command* commandData = reinterpret_cast<DX11Command*>(command->cmdData);
    for (DX11Command *cmd = commandData, *cmdEnd = commandData + command->cmdCount; cmd != cmdEnd; ++cmd)
    {
        DAVA::uint64* arg = cmd->arguments.arg;

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

        case DX11Command::SYNC_CPU_GPU:
            {
                if (DeviceCaps().isPerfQuerySupported)
                {
                    ID3D11Query* tsQuery = nullptr;
                    ID3D11Query* fqQuery = nullptr;

                    D3D11_QUERY_DESC desc = { D3D11_QUERY_TIMESTAMP };
                    DX11DeviceCommand(DX11Command::CREATE_QUERY, &desc, &tsQuery);

                    desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
                    DX11DeviceCommand(DX11Command::CREATE_QUERY, &desc, &fqQuery);

                    if (tsQuery && fqQuery)
                    {
                        _D3D11_ImmediateContext->Begin(fqQuery);
                        _D3D11_ImmediateContext->End(tsQuery);
                        _D3D11_ImmediateContext->End(fqQuery);

                        uint64 timestamp = 0;
                        while (S_FALSE == _D3D11_ImmediateContext->GetData(tsQuery, &timestamp, sizeof(uint64), 0))
                        {
                        };

                        if (timestamp)
                        {
                            *reinterpret_cast<uint64*>(arg[0]) = DAVA::SystemTimer::Instance()->GetAbsoluteUs();

                            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT data;
                            while (S_FALSE == _D3D11_ImmediateContext->GetData(fqQuery, &data, sizeof(data), 0))
                            {
                            };

                            if (!data.Disjoint && data.Frequency)
                            {
                                *reinterpret_cast<uint64*>(arg[1]) = timestamp / (data.Frequency / 1000000); //mcs
                            }
                        }
                    }
                    DAVA::SafeRelease(tsQuery);
                    DAVA::SafeRelease(fqQuery);
                }
            }
            break;
            case DX11Command::QUERY_INTERFACE:
            {
                ValidateDX11Device("QueryInterface");
                cmd->retval = _D3D11_Device->QueryInterface(*(const IID*)(arg[0]), (void**)(arg[1]));
                break;
            }
            case DX11Command::CREATE_DEFERRED_CONTEXT:
            {
                ValidateDX11Device("CreateDeferredContext");
                cmd->retval = _D3D11_Device->CreateDeferredContext((UINT)arg[0], (ID3D11DeviceContext**)(arg[1]));
                break;
            }
            case DX11Command::CREATE_BLEND_STATE:
            {
                ValidateDX11Device("CreateBlendState");
                cmd->retval = _D3D11_Device->CreateBlendState((const D3D11_BLEND_DESC*)(arg[0]), (ID3D11BlendState**)(arg[1]));
                break;
            }
            case DX11Command::CREATE_SAMPLER_STATE:
            {
                ValidateDX11Device("CreateSamplerState");
                cmd->retval = _D3D11_Device->CreateSamplerState((const D3D11_SAMPLER_DESC*)(arg[0]), (ID3D11SamplerState**)(arg[1]));
                break;
            }
            case DX11Command::CREATE_RASTERIZER_STATE:
            {
                ValidateDX11Device("CreateRasterizerState");
                cmd->retval = _D3D11_Device->CreateRasterizerState((const D3D11_RASTERIZER_DESC*)(arg[0]), (ID3D11RasterizerState**)(arg[1]));
                break;
            }
            case DX11Command::CREATE_DEPTH_STENCIL_STATE:
            {
                ValidateDX11Device("CreateDepthStencilState");
                cmd->retval = _D3D11_Device->CreateDepthStencilState((const D3D11_DEPTH_STENCIL_DESC*)(arg[0]), (ID3D11DepthStencilState**)(arg[1]));
                break;
            }
            case DX11Command::CREATE_VERTEX_SHADER:
            {
                ValidateDX11Device("CreateVertexShader");
                cmd->retval = _D3D11_Device->CreateVertexShader((const void*)(arg[0]), (SIZE_T)(arg[1]), (ID3D11ClassLinkage*)(arg[2]), (ID3D11VertexShader**)(arg[3]));
                break;
            }
            case DX11Command::CREATE_PIXEL_SHADER:
            {
                ValidateDX11Device("CreatePixelShader");
                cmd->retval = _D3D11_Device->CreatePixelShader((const void*)(arg[0]), (SIZE_T)(arg[1]), (ID3D11ClassLinkage*)(arg[2]), (ID3D11PixelShader**)(arg[3]));
                break;
            }
            case DX11Command::CREATE_INPUT_LAYOUT:
            {
                ValidateDX11Device("CreateInputLayout");
                cmd->retval = _D3D11_Device->CreateInputLayout((const D3D11_INPUT_ELEMENT_DESC*)(arg[0]), (UINT)(arg[1]), (const void*)(arg[2]), (SIZE_T)(arg[3]), (ID3D11InputLayout**)(arg[4]));
                break;
            }
            case DX11Command::CREATE_QUERY:
            {
                ValidateDX11Device("CreateQuery");
                cmd->retval = _D3D11_Device->CreateQuery((const D3D11_QUERY_DESC*)(arg[0]), (ID3D11Query**)(arg[1]));
                break;
            }
            case DX11Command::CREATE_BUFFER:
            {
                ValidateDX11Device("CreateBuffer");
                cmd->retval = _D3D11_Device->CreateBuffer((const D3D11_BUFFER_DESC*)(arg[0]), (const D3D11_SUBRESOURCE_DATA*)(arg[1]), (ID3D11Buffer**)(arg[2]));
                break;
            }
            case DX11Command::CREATE_TEXTURE_2D:
            {
                ValidateDX11Device("CreateTexture2D");
                cmd->retval = _D3D11_Device->CreateTexture2D((const D3D11_TEXTURE2D_DESC*)(arg[0]), (const D3D11_SUBRESOURCE_DATA*)(arg[1]), (ID3D11Texture2D**)(arg[2]));
                break;
            }
            case DX11Command::CREATE_RENDER_TARGET_VIEW:
            {
                ValidateDX11Device("CreateRenderTargetView");
                cmd->retval = _D3D11_Device->CreateRenderTargetView((ID3D11Resource*)(arg[0]), (const D3D11_RENDER_TARGET_VIEW_DESC*)(arg[1]), (ID3D11RenderTargetView**)(arg[2]));
                break;
            }
            case DX11Command::CREATE_DEPTH_STENCIL_VIEW:
            {
                ValidateDX11Device("CreateDepthStencilView");
                cmd->retval = _D3D11_Device->CreateDepthStencilView((ID3D11Resource*)(arg[0]), (const D3D11_DEPTH_STENCIL_VIEW_DESC*)(arg[1]), (ID3D11DepthStencilView**)(arg[2]));
                break;
            }
            case DX11Command::CREATE_SHADER_RESOURCE_VIEW:
            {
                ValidateDX11Device("CreateShaderResourceView");
                cmd->retval = _D3D11_Device->CreateShaderResourceView((ID3D11Resource*)(arg[0]), (const D3D11_SHADER_RESOURCE_VIEW_DESC*)(arg[1]), (ID3D11ShaderResourceView**)(arg[2]));
                break;
            }
            default:
            {
                DAVA::String message = DAVA::Format("Invalid or unsupported DX11 command: %u", cmd->func);
                DVASSERT_MSG(0, message.c_str());
            }
        }
    }
}

void ExecDX11(DX11Command* command, uint32 cmdCount, bool force_immediate)
{
    CommonImpl::ImmediateCommand cmd;
    cmd.cmdData = command;
    cmd.cmdCount = cmdCount;
    cmd.forceImmediate = force_immediate;
    RenderLoop::IssueImmediateCommand(&cmd);
}

HRESULT ExecDX11DeviceCommand(DX11Command cmd, const char* cmdName, const char* fileName, DAVA::uint32 line)
{
    DVASSERT(cmd.func >= DX11Command::DEVICE_FIRST_COMMAND);
    DVASSERT(cmd.func < DX11Command::DEVICE_LAST_COMMAND);

    if (GetCurrentThreadId() == _DX11_RenderThreadId)
    {
        // running on render thread
        // immediate execution, device validation will occur inside
        ExecDX11(&cmd, 1, true);
    }
    else
    {
        // call occured from secondary (non-render thread)
        // validate device before sending commands to execution
        ValidateDX11Device(cmdName);
        ExecDX11(&cmd, 1, false);
    }

    DX11_ProcessCallResult(cmd.retval, cmdName, fileName, line);
    return cmd.retval;
}

void ValidateDX11Device(const char* call)
{
    if (_D3D11_Device == nullptr)
    {
        DAVA::Logger::Error("DX11 Device is not ready, %s and further calls will be blocked.", call);
        for (;;)
        {
            Sleep(1);
        }
    }
}

void FlushContextIfRequired(ID3D11DeviceContext* context)
{
#if LUMIA_1020_DEPTHBUF_WORKAROUND
    static int runningOnLumia1020 = -1;
    if (runningOnLumia1020 == -1)
        runningOnLumia1020 = (DAVA::DeviceInfo::GetModel().find("NOKIA RM-875") != DAVA::String::npos) ? 1 : 0;

    if (runningOnLumia1020)
        context->Flush();
#endif
}

static void dx11_EndFrame()
{
    if (_DX11_UseHardwareCommandBuffers)
    {
        ID3D11CommandList* cmdList = nullptr;
        CHECK_HR(_D3D11_SecondaryContext->FinishCommandList(TRUE, &cmdList));

        DAVA::LockGuard<DAVA::Mutex> lock(pendingSecondaryCmdListSync);
        pendingSecondaryCmdLists.push_back(cmdList);
    }
    else
    {
        ConstBufferDX11::InvalidateAllInstances();
    }
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
        for (Handle cmdBuffer : pp->commandBuffers)
        {
            CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuffer);
            if (cb->sync != InvalidHandle)
            {
                SyncObjectDX11_t* s = SyncObjectPoolDX11::Get(cb->sync);
                s->is_signaled = true;
                s->is_used = true;
            }

            if (_DX11_UseHardwareCommandBuffers)
            {
                if ((cb->context != nullptr) && !cb->isComplete)
                {
                    cb->context->ClearState();
                    cb->context->FinishCommandList(FALSE, &(cb->commandList));
                }
                DAVA::SafeRelease(cb->contextAnnotation);
                DAVA::SafeRelease(cb->commandList);
                DAVA::SafeRelease(cb->context);
            }
            CommandBufferPoolDX11::Free(cmdBuffer);
        }
        pp->commandBuffers.clear();
        RenderPassPoolDX11::Free(p);
    }
}

void CommandBufferDX11_t::Reset()
{
    cur_topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    for (uint32 i = 0; i != MAX_VERTEX_STREAM_COUNT; ++i)
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
    for (uint32 i = 0; i != MAX_VERTEX_STREAM_COUNT; ++i)
    {
        last_vb[i] = InvalidHandle;
        last_vb_stride[i] = 0;
    }
    last_ps = InvalidHandle;
    last_vdecl = VertexLayout::InvalidUID;

    if (_DX11_UseHardwareCommandBuffers)
    {
        memset(vertexConstBuffer, 0, sizeof(vertexConstBuffer));
        memset(fragmentConstBuffer, 0, sizeof(fragmentConstBuffer));
        context->IASetPrimitiveTopology(cur_topo);
        deferredPerfQueries.clear();
    }

    isComplete = false;
}

void CommandBufferDX11_t::ApplyTopology(PrimitiveType primType, uint32 primCount, uint32* indexCount)
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
        if (_DX11_UseHardwareCommandBuffers)
            context->IASetPrimitiveTopology(topo);

        cur_topo = topo;
    }
}

void CommandBufferDX11_t::HWApplyVertexData()
{
    DVASSERT(_DX11_UseHardwareCommandBuffers);

    for (uint32 i = 0; i != cur_stream_count; ++i)
    {
        if (cur_vb[i] != last_vb[i] || cur_vb_stride[i] != last_vb_stride[i])
        {
            VertexBufferDX11::SetToRHI(cur_vb[i], i, 0, cur_vb_stride[i], context);
            StatSet::IncStat(stat_SET_VB, 1);
            last_vb[i] = cur_vb[i];
            last_vb_stride[i] = cur_vb_stride[i];
        }
    }
}

void CommandBufferDX11_t::HWApplyRasterizerState()
{
    DVASSERT(_DX11_UseHardwareCommandBuffers);

    if (cur_rs == nullptr)
        cur_rs = dx11_GetRasterizerState(rs_param);

    if (cur_rs != last_rs)
    {
        context->RSSetState(cur_rs);
        last_rs = cur_rs;
    }
}

void CommandBufferDX11_t::HWApplyConstBuffers()
{
    DVASSERT(_DX11_UseHardwareCommandBuffers);

    context->VSSetConstantBuffers(0, MAX_CONST_BUFFER_COUNT, vertexConstBuffer);
    context->PSSetConstantBuffers(0, MAX_CONST_BUFFER_COUNT, fragmentConstBuffer);
    StatSet::IncStat(stat_SET_CB, 2);
}

void CommandBufferDX11_t::Execute()
{
    if (_DX11_UseHardwareCommandBuffers)
        ExecuteHardwareBuffers();
    else
        ExecuteSoftwareBuffers();
}

void CommandBufferDX11_t::ExecuteHardwareBuffers()
{
    DVASSERT(_DX11_UseHardwareCommandBuffers);
    DVASSERT(isComplete);

    DAVA_PROFILER_CPU_SCOPE(DAVA::ProfilerCPUMarkerName::RHI_CMD_BUFFER_EXECUTE);
    _D3D11_ImmediateContext->ExecuteCommandList(commandList, FALSE);
    FlushContextIfRequired(_D3D11_ImmediateContext);

    DAVA::SafeRelease(context);
    DAVA::SafeRelease(contextAnnotation);
    DAVA::SafeRelease(commandList);
}

void CommandBufferDX11_t::ExecuteSoftwareBuffers()
{
    DVASSERT(_DX11_UseHardwareCommandBuffers == false);

    for (const uint8 *c = cmdData, *c_end = cmdData + curUsedSize; c != c_end;)
    {
        const SWCommand* cmd = reinterpret_cast<const SWCommand*>(c);

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
            uint32 stream_i = static_cast<const SWCommand_SetVertexData*>(cmd)->streamIndex;

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
            Handle perfQuery = ((SWCommand_IssueTimestamptQuery*)cmd)->perfQuery;
            PerfQueryDX11::IssueTimestampQuery(perfQuery, _D3D11_ImmediateContext);
        }
        break;

        case CMD_SET_PIPELINE_STATE:
            {
                Handle ps = static_cast<const SWCommand_SetPipelineState*>(cmd)->ps;
                Handle vdeclUID = static_cast<const SWCommand_SetPipelineState*>(cmd)->vdecl;
                const VertexLayout* vdecl = (vdeclUID == VertexLayout::InvalidUID) ? nullptr : VertexLayout::Get(vdeclUID);

                cur_pipelinestate = ps;
                cur_stream_count = PipelineStateDX11::VertexLayoutStreamCount(ps);
                for (uint32 i = 0; i != cur_stream_count; ++i)
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
                uint32 unitIndex = static_cast<const SWCommand_SetFragmentTexture*>(cmd)->unitIndex;
                TextureDX11::SetToRHIFragment(tex, unitIndex, _D3D11_ImmediateContext);
            }
            break;

            case CMD_SET_VERTEX_TEXTURE:
            {
                Handle tex = static_cast<const SWCommand_SetVertexTexture*>(cmd)->tex;
                uint32 unitIndex = static_cast<const SWCommand_SetVertexTexture*>(cmd)->unitIndex;
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
                uint32 vertexCount = static_cast<const SWCommand_DrawPrimitive*>(cmd)->vertexCount;

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

                for (uint32 s = 0; s != cur_stream_count; ++s)
                    VertexBufferDX11::SetToRHI(cur_vb[s], s, 0, cur_vb_stride[s], _D3D11_ImmediateContext);

                _D3D11_ImmediateContext->Draw(vertexCount, 0);
            }
            break;

            case CMD_DRAW_INDEXED_PRIMITIVE:
            {
                D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY(static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd)->mode);
                uint32 baseVertex = static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd)->firstVertex;
                uint32 indexCount = static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd)->indexCount;
                uint32 startIndex = static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd)->startIndex;

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

                for (uint32 s = 0; s != cur_stream_count; ++s)
                    VertexBufferDX11::SetToRHI(cur_vb[s], s, 0, cur_vb_stride[s], _D3D11_ImmediateContext);

                _D3D11_ImmediateContext->DrawIndexed(indexCount, startIndex, baseVertex);
            }
            break;

            case CMD_DRAW_INSTANCED_PRIMITIVE:
            {
                D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY(static_cast<const SWCommand_DrawInstancedPrimitive*>(cmd)->mode);
                uint32 vertexCount = static_cast<const SWCommand_DrawInstancedPrimitive*>(cmd)->vertexCount;
                uint32 instCount = static_cast<const SWCommand_DrawInstancedPrimitive*>(cmd)->instanceCount;

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

                for (uint32 s = 0; s != cur_stream_count; ++s)
                    VertexBufferDX11::SetToRHI(cur_vb[s], s, 0, cur_vb_stride[s], _D3D11_ImmediateContext);

                _D3D11_ImmediateContext->DrawInstanced(vertexCount, instCount, 0, 0);
            }
            break;

            case CMD_DRAW_INSTANCED_INDEXED_PRIMITIVE:
            {
                D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY(static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->mode);
                uint32 vertexCount = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->indexCount;
                uint32 indexCount = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->indexCount;
                uint32 startIndex = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->startIndex;
                uint32 instCount = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->instanceCount;
                uint32 baseInst = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->baseInstance;

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

                for (uint32 s = 0; s != cur_stream_count; ++s)
                    VertexBufferDX11::SetToRHI(cur_vb[s], s, 0, cur_vb_stride[s], _D3D11_ImmediateContext);

                _D3D11_ImmediateContext->DrawIndexedInstanced(indexCount, instCount, startIndex, 0, baseInst);
            }
            break;

            default:
                DAVA::Logger::Error("unsupported command: %d", cmd->type);
                DVASSERT_MSG(false, "unsupported command");
        }

        if (cmd->type == CMD_END)
            break;
        c += cmd->size;
    }
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

    ID3D11RenderTargetView* rt_view[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
    ID3D11DepthStencilView* ds_view = nullptr;

    context->OMGetRenderTargets(countof(rt_view), rt_view, &ds_view);

    for (uint32 i = 0; i != countof(rt_view); ++i)
    {
        if (rt_view[i])
        {
            if (i == 0)
            {
                if (passCfg.colorBuffer[0].texture == rhi::InvalidHandle)
                {
                    D3D11_TEXTURE2D_DESC desc = {};
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
        {
            context->ClearDepthStencilView(ds_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, passCfg.depthStencilBuffer.clearDepth, passCfg.depthStencilBuffer.clearStencil);
        }
        ds_view->Release();
    }

    context->IASetPrimitiveTopology(cur_topo);

    DVASSERT(!isFirstInPass || cur_query_buf == InvalidHandle || !QueryBufferDX11::QueryIsCompleted(cur_query_buf));
}

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
    dispatch->impl_CommandBuffer_IssueTimestampQuery = &dx11_CommandBuffer_IssueTimestampQuery;
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
}
