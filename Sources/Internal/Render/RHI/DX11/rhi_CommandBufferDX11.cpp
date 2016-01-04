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
    Handle cur_vb;
    uint32 cur_vb_stride;
    Handle cur_pipelinestate;
    uint32 cur_stride;
    Handle cur_query_buf;
    uint32 cur_query_i;
    D3D11_VIEWPORT def_viewport;
    RasterizerParamDX11 rs_param;
    ID3D11RasterizerState* cur_rs;

    ID3D11RasterizerState* last_rs;
    Handle last_vb;
    uint32 last_vb_stride;
    Handle last_ps;
    uint32 last_vdecl;

    ID3D11DeviceContext* context;
    ID3DUserDefinedAnnotation* contextAnnotation;
    ID3D11CommandList* commandList;

    ID3D11Buffer* vertexConstBuffer[MAX_CONST_BUFFER_COUNT];
    ID3D11Buffer* fragmentConstBuffer[MAX_CONST_BUFFER_COUNT];

    Handle sync;
};

class
RenderPassDX11_t
{
public:
    std::vector<Handle> cmdBuf;
    int priority;
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

static void _ExecuteQueuedCommandsDX11();

static DAVA::Thread* _DX11_RenderThread = nullptr;
static unsigned _DX11_RenderThreadFrameCount = 0;
static bool _DX11_RenderThreadExitPending = false;
static DAVA::Spinlock _DX11_RenderThreadExitSync;
static DAVA::Semaphore _DX11_RenderThreadStartedSync(0);

static DX11Command* _DX11_PendingImmediateCmd = nullptr;
static uint32 _DX11_PendingImmediateCmdCount = 0;
static DAVA::Mutex _DX11_PendingImmediateCmdSync;

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

    for (unsigned i = 0; i != cmdBufCount; ++i)
    {
        Handle h = CommandBufferPoolDX11::Alloc();
        CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(h);

        cb->commandList = nullptr;
        cb->passCfg = passDesc;
        cb->isFirstInPass = i == 0;
        cb->isLastInPass = i == cmdBufCount - 1;

        if (!cb->context)
        {
            HRESULT hr = _D3D11_Device->CreateDeferredContext(0, &(cb->context));

            if (SUCCEEDED(hr))
            {
                hr = cb->context->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)(&(cb->contextAnnotation)));
            }
        }

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
        _DX11_Frame.back().readyToExecute = false;
        _DX11_Frame.back().toBeDiscarded = false;
        _DX11_Frame.back().cmdList = nullptr;

        Trace("\n\n-------------------------------\nframe %u started\n", _DX11_FrameNumber);
        _DX11_FrameStarted = true;
        ++_DX11_FrameNumber;
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
    ID3D11DeviceContext* context = cb->context;
    bool clear_color = cb->isFirstInPass && cb->passCfg.colorBuffer[0].loadAction == LOADACTION_CLEAR;
    bool clear_depth = cb->isFirstInPass && cb->passCfg.depthStencilBuffer.loadAction == LOADACTION_CLEAR;
    ID3D11RenderTargetView* rt[1] = { _D3D11_RenderTargetView };

    cb->Reset();

    cb->sync = InvalidHandle;

    cb->def_viewport.TopLeftX = 0;
    cb->def_viewport.TopLeftY = 0;
    cb->def_viewport.MinDepth = 0.0f;
    cb->def_viewport.MaxDepth = 1.0f;

    if (cb->passCfg.colorBuffer[0].texture != rhi::InvalidHandle && cb->passCfg.colorBuffer[0].texture != rhi::DefaultDepthBuffer)
    {
        Size2i sz = TextureDX11::Size(cb->passCfg.colorBuffer[0].texture);

        cb->def_viewport.Width = float(sz.dx);
        cb->def_viewport.Height = float(sz.dy);

        TextureDX11::SetRenderTarget(cb->passCfg.colorBuffer[0].texture, cb->passCfg.depthStencilBuffer.texture, cb->passCfg.colorBuffer[0].textureLevel, cb->passCfg.colorBuffer[0].textureFace, context);
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
                if (cb->passCfg.colorBuffer[0].texture == rhi::InvalidHandle)
                {
                    D3D11_TEXTURE2D_DESC desc;

                    _D3D11_SwapChainBuffer->GetDesc(&desc);

                    cb->def_viewport.Width = float(desc.Width);
                    cb->def_viewport.Height = float(desc.Height);
                }

                context->RSSetViewports(1, &(cb->def_viewport));
            }

            if (clear_color)
                context->ClearRenderTargetView(rt_view[i], cb->passCfg.colorBuffer[0].clearColor);

            rt_view[i]->Release();
        }
    }

    if (ds_view)
    {
        if (clear_depth)
            context->ClearDepthStencilView(ds_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, cb->passCfg.depthStencilBuffer.clearDepth, cb->passCfg.depthStencilBuffer.clearStencil);

        ds_view->Release();
    }
    //-    context->IASetPrimitiveTopology(cb->cur_topo);
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_End(Handle cmdBuf, Handle syncObject)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    cb->context->FinishCommandList(TRUE, &(cb->commandList));
    cb->sync = syncObject;
    cb->isComplete = true;
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetPipelineState(Handle cmdBuf, Handle ps, uint32 vdeclUID)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    const VertexLayout* vdecl = (vdeclUID == VertexLayout::InvalidUID) ? nullptr : VertexLayout::Get(vdeclUID);

    cb->cur_pipelinestate = ps;
    cb->cur_vb_stride = (vdecl) ? vdecl->Stride() : 0;

    if (ps != cb->last_ps || vdeclUID != cb->last_vdecl)
    {
        PipelineStateDX11::SetToRHI(ps, vdeclUID, cb->context);
        cb->last_ps = ps;
        cb->last_vdecl = vdeclUID;
        StatSet::IncStat(stat_SET_PS, 1);
    }
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetCullMode(Handle cmdBuf, CullMode mode)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    cb->rs_param.cullMode = mode;
    cb->cur_rs = nullptr;
}

//------------------------------------------------------------------------------

void dx11_CommandBuffer_SetScissorRect(Handle cmdBuf, ScissorRect rect)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    int x = rect.x;
    int y = rect.y;
    int w = rect.width;
    int h = rect.height;

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

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetViewport(Handle cmdBuf, Viewport vp)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    int x = vp.x;
    int y = vp.y;
    int w = vp.width;
    int h = vp.height;

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

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetFillMode(Handle cmdBuf, FillMode mode)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    cb->rs_param.wireframe = (mode == FILLMODE_WIREFRAME);
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetVertexData(Handle cmdBuf, Handle vb, uint32 streamIndex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    cb->cur_vb = vb;
    cb->cur_vb_stride = (cb->cur_vb_stride) ? cb->cur_vb_stride : PipelineStateDX11::VertexLayoutStride(cb->cur_pipelinestate);
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetVertexConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    ConstBufferDX11::SetToRHI(buffer, cb->context, cb->vertexConstBuffer);
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetVertexTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    TextureDX11::SetToRHIVertex(tex, unitIndex, cb->context);

    StatSet::IncStat(stat_SET_TEX, 1);
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetIndices(Handle cmdBuf, Handle ib)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    IndexBufferDX11::SetToRHI(ib, 0, cb->context);
    StatSet::IncStat(stat_SET_IB, 1);
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetQueryIndex(Handle cmdBuf, uint32 objectIndex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    cb->cur_query_i = objectIndex;
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetQueryBuffer(Handle cmdBuf, Handle queryBuf)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    DVASSERT(cb->cur_query_buf == InvalidHandle);
    cb->cur_query_buf = queryBuf;
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetFragmentConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    ConstBufferDX11::SetToRHI(buffer, cb->context, cb->fragmentConstBuffer);
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetFragmentTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    TextureDX11::SetToRHIFragment(tex, unitIndex, cb->context);

    StatSet::IncStat(stat_SET_TEX, 1);
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetDepthStencilState(Handle cmdBuf, Handle depthStencilState)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    DepthStencilStateDX11::SetToRHI(depthStencilState, cb->context);
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetSamplerState(Handle cmdBuf, const Handle samplerState)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);

    SamplerStateDX11::SetToRHI(samplerState, cb->context);

    StatSet::IncStat(stat_SET_SS, 1);
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_DrawPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    ID3D11DeviceContext* ctx = cb->context;
    unsigned vertexCount = 0;
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
    /*
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
*/
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_DrawIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count, uint32 vertexCount, uint32 firstVertex, uint32 startIndex)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    ID3D11DeviceContext* ctx = cb->context;
    unsigned indexCount = 0;

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
    /*
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
*/
}

//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetMarker(Handle cmdBuf, const char* text)
{
    CommandBufferDX11_t* cb = CommandBufferPoolDX11::Get(cmdBuf);
    wchar_t txt[128];

    ::MultiByteToWideChar(CP_ACP, 0, text, -1, txt, countof(txt));

    if (cb->contextAnnotation)
    {
        cb->contextAnnotation->SetMarker(txt);
    }
    //    else
    //    {
    //        ::D3DPERF_SetMarker( D3DCOLOR_ARGB(0xFF,0x40,0x40,0x80), txt );
    //    }
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

        if (cmdList)
        {
            _D3D11_ImmediateContext->ExecuteCommandList(cmdList, FALSE);
            cmdList->Release();
            cmdList = nullptr;
        }

        for (std::vector<RenderPassDX11_t *>::iterator p = pass.begin(), p_end = pass.end(); p != p_end; ++p)
        {
            RenderPassDX11_t* pp = *p;

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
        _D3D11_SwapChain->Present(0, 0);
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "SwapChain::Present");

        // update sync-objects

        for (SyncObjectPoolDX11::Iterator s = SyncObjectPoolDX11::Begin(), s_end = SyncObjectPoolDX11::End(); s != s_end; ++s)
        {
            if (s->is_used && (frame_n - s->frame >= 2))
                s->is_signaled = true;
        }
    }

    if (_DX11_InitParam.FrameCommandExecutionSync)
        _DX11_InitParam.FrameCommandExecutionSync->Unlock();
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
    Trace("RHI render-thread started\n");

    while (true)
    {
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::render_loop");

        bool do_wait = true;
        bool do_exit = false;

        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "renderer_wait_core");
        do
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
        } while (do_wait);
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "renderer_wait_core");

        if (do_exit)
            break;

        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "exec_que_cmds");
        _ExecuteQueuedCommandsDX11();
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "exec_que_cmds");

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::render_loop");
    }

    Trace("RHI render-thread stopped\n");
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
                if (!_DX11_Frame.back().toBeDiscarded)
                    _D3D11_SecondaryContext->FinishCommandList(TRUE, &(_DX11_Frame.back().cmdList));

                _DX11_Frame.back().readyToExecute = true;
                _DX11_Frame.back().sync = sync;
                _DX11_FrameStarted = false;
                Trace("\n\n-------------------------------\nframe %u generated\n", _DX11_Frame.back().number);
            }
        }
        _DX11_FrameSync.Unlock();

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
                DAVA::Thread::Yield();

        } while (frame_cnt >= _DX11_RenderThreadFrameCount || reset_pending);
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "core_wait_renderer");
    }
    else
    {
        if (_DX11_Frame.size())
        {
            _D3D11_SecondaryContext->FinishCommandList(TRUE, &(_DX11_Frame.back().cmdList));
            _DX11_Frame.back().readyToExecute = true;
            _DX11_Frame.back().sync = sync;
            _DX11_FrameStarted = false;
        }
        else
        {
            ID3D11CommandList* cl = nullptr;

            _D3D11_SecondaryContext->FinishCommandList(TRUE, &cl);
            _D3D11_ImmediateContext->ExecuteCommandList(cl, FALSE);
            cl->Release();
        }

        _ExecuteQueuedCommandsDX11();
    }
    TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::present");
}

//------------------------------------------------------------------------------

CommandBufferDX11_t::CommandBufferDX11_t()
    : context(nullptr)
    , contextAnnotation(nullptr)
    , commandList(nullptr)
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
    cur_vb = InvalidHandle;
    cur_vb_stride = 0;
    cur_pipelinestate = InvalidHandle;
    cur_stride = 0;
    cur_query_buf = InvalidHandle;
    cur_query_i = DAVA::InvalidIndex;
    cur_rs = nullptr;

    rs_param.cullMode = CULL_NONE;
    rs_param.scissorEnabled = false;
    rs_param.wireframe = false;

    last_rs = nullptr;
    last_vb = InvalidHandle;
    last_vb_stride = 0;
    last_ps = InvalidHandle;
    last_vdecl = VertexLayout::InvalidUID;

    memset(vertexConstBuffer, 0, sizeof(vertexConstBuffer));
    memset(fragmentConstBuffer, 0, sizeof(fragmentConstBuffer));

    context->IASetPrimitiveTopology(cur_topo);

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
        context->IASetPrimitiveTopology(topo);
        cur_topo = topo;
    }
}

//------------------------------------------------------------------------------

void CommandBufferDX11_t::_ApplyVertexData()
{
    if (cur_vb != last_vb || cur_vb_stride != last_vb_stride)
    {
        VertexBufferDX11::SetToRHI(cur_vb, 0, 0, cur_vb_stride, context);
        StatSet::IncStat(stat_SET_VB, 1);
        last_vb = cur_vb;
        last_vb_stride = cur_vb_stride;
    }
}

//------------------------------------------------------------------------------

void CommandBufferDX11_t::_ApplyRasterizerState()
{
    if (!cur_rs)
    {
        cur_rs = _GetRasterizerState(rs_param);
    }

    if (cur_rs != last_rs)
    {
        context->RSSetState(cur_rs);
        last_rs = cur_rs;
    }
}

//------------------------------------------------------------------------------

void CommandBufferDX11_t::_ApplyConstBuffers()
{
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
}

//------------------------------------------------------------------------------

void CommandBufferDX11_t::Execute()
{
    SCOPED_FUNCTION_TIMING();

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

    _DX11_FrameSync.Lock();
    _DX11_ResetPending = true;
    _DX11_FrameSync.Unlock();

    _DX11_FrameSync.Lock();
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

                    cb->contextAnnotation->Release();
                    cb->contextAnnotation = nullptr;

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
