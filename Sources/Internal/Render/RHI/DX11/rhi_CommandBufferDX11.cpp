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
    namespace rhi { extern void _InitDX11(); }
    #include <vector>

    


namespace rhi
{
//==============================================================================

struct
RasterizerParamDX11
{
    uint32  cullMode:3;
    uint32  scissorEnabled:1;

    bool    operator==( const RasterizerParamDX11& b ) const
            {
                return this->cullMode == b.cullMode  &&  this->scissorEnabled == b.scissorEnabled;
            }
};

struct
RasterizerStateDX11
{
    RasterizerParamDX11     param;
    ID3D11RasterizerState*  state;
};


//==============================================================================

class
CommandBufferDX11_t
{
public:
                                CommandBufferDX11_t();
                                ~CommandBufferDX11_t();

    void                        Execute();

    RenderPassConfig            passCfg;
    uint32                      isFirstInPass:1;
    uint32                      isLastInPass:1;


    D3D11_PRIMITIVE_TOPOLOGY    cur_topo;
    Handle                      cur_ib;
    Handle                      cur_vb;
    Handle                      cur_vb_stride;
    Handle                      cur_pipelinestate;
    uint32                      cur_stride;
    Handle                      cur_query_buf;
    uint32                      cur_query_i;
    D3D11_VIEWPORT              def_viewport;
    RasterizerParamDX11         rs_param;
    ID3D11RasterizerState*      cur_rs;

    ID3D11DeviceContext*        context;
    ID3DUserDefinedAnnotation*  contextAnnotation;
    ID3D11CommandList*          commandList;
    
    Handle                      sync;
};


class 
RenderPassDX11_t
{
public:
    std::vector<Handle> cmdBuf;
    int                 priority;
};


struct
SyncObjectDX11_t 
{
    uint32  frame;
    uint32  is_signaled:1;
};

typedef ResourcePool<CommandBufferDX11_t,RESOURCE_COMMAND_BUFFER,CommandBuffer::Descriptor,false>   CommandBufferPool;
typedef ResourcePool<RenderPassDX11_t,RESOURCE_RENDER_PASS,RenderPassConfig,false>            RenderPassPool;
typedef ResourcePool<SyncObjectDX11_t,RESOURCE_SYNC_OBJECT,SyncObject::Descriptor,false>            SyncObjectPool;

RHI_IMPL_POOL(CommandBufferDX11_t,RESOURCE_COMMAND_BUFFER,CommandBuffer::Descriptor,false);
RHI_IMPL_POOL(RenderPassDX11_t,RESOURCE_RENDER_PASS,RenderPassConfig,false);
RHI_IMPL_POOL(SyncObjectDX11_t,RESOURCE_SYNC_OBJECT,SyncObject::Descriptor,false);



struct
FrameDX11
{
    unsigned            number;
    Handle              sync;
    std::vector<Handle> pass;
    uint32              readyToExecute:1;
};

static std::vector<FrameDX11>   _Frame;
static bool                     _FrameStarted   = false;
static unsigned                 _FrameNumber    = 1;
//static DAVA::Spinlock       _FrameSync;
static DAVA::Mutex              _FrameSync;

static void _ExecuteQueuedCommandsDX11();


static DAVA::Thread*        _DX11_RenderThread              = nullptr;
static unsigned             _DX11_RenderThreadFrameCount    = 0;
static bool                 _DX11_RenderThreadExitPending   = false;
static DAVA::Spinlock       _DX11_RenderThreadExitSync;
static DAVA::Semaphore      _DX11_RenderThreadStartedSync   (0);


static DX11Command*         _DX11_PendingImmediateCmd       = nullptr;
static uint32               _DX11_PendingImmediateCmdCount  = 0;
static DAVA::Mutex          _DX11_PendingImmediateCmdSync;


//------------------------------------------------------------------------------

static std::vector<RasterizerStateDX11> _RasterizerStateDX11;

static ID3D11RasterizerState*
_GetRasterizerState( RasterizerParamDX11 param )
{
    ID3D11RasterizerState*  state = nullptr;
    
    for( std::vector<RasterizerStateDX11>::iterator s=_RasterizerStateDX11.begin(),s_end=_RasterizerStateDX11.end(); s!=s_end; ++s )
    {
        if( s->param == param )
        {
            state = s->state;
            break;
        }
    }

    if( !state )
    {
        D3D11_RASTERIZER_DESC   desc;
        HRESULT                 hr;

        desc.FillMode               = D3D11_FILL_SOLID;
        desc.FrontCounterClockwise  = FALSE;

        switch( CullMode(param.cullMode) )
        {
            case CULL_NONE  : desc.CullMode = D3D11_CULL_NONE; break;
            case CULL_CCW   : desc.CullMode = D3D11_CULL_BACK; break;
            case CULL_CW    : desc.CullMode = D3D11_CULL_FRONT; break;
        }

        desc.DepthBias              = 0;
        desc.DepthBiasClamp         = 0;
        desc.SlopeScaledDepthBias   = 0.0f;
        desc.DepthClipEnable        = TRUE;
        desc.ScissorEnable          = param.scissorEnabled;
        desc.MultisampleEnable      = FALSE;
        desc.AntialiasedLineEnable  = FALSE;
            
        
        hr = _D3D11_Device->CreateRasterizerState( &desc, &state );
        if( SUCCEEDED(hr) )
        {
            RasterizerStateDX11 s;

            s.param = param;
            s.state = state;
            _RasterizerStateDX11.push_back( s );
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
dx11_RenderPass_Allocate( const RenderPassConfig& passDesc, uint32 cmdBufCount, Handle* cmdBuf )
{
    DVASSERT(cmdBufCount);

    Handle              handle = RenderPassPool::Alloc();
    RenderPassDX11_t*   pass   = RenderPassPool::Get( handle );
    
    pass->cmdBuf.resize( cmdBufCount );
    pass->priority = passDesc.priority;

    for( unsigned i=0; i!=cmdBufCount; ++i )
    {
        Handle               h  = CommandBufferPool::Alloc();
        CommandBufferDX11_t* cb = CommandBufferPool::Get( h );

        cb->passCfg       = passDesc;
        cb->isFirstInPass = i == 0;
        cb->isLastInPass  = i == cmdBufCount-1;

        if( !cb->context )
        {
            HRESULT hr = _D3D11_Device->CreateDeferredContext( 0, &(cb->context) );
            
            if( SUCCEEDED(hr) )            
            {
                hr = cb->context->QueryInterface( __uuidof(ID3DUserDefinedAnnotation), (void**)(&(cb->contextAnnotation)) );
            }
        }

        pass->cmdBuf[i] = h;
        cmdBuf[i]       = h;
    }

    return handle;
}


//------------------------------------------------------------------------------

static void
dx11_RenderPass_Begin( Handle pass )
{
    _FrameSync.Lock();

    if( !_FrameStarted )
    {
        _Frame.push_back( FrameDX11() );
        _Frame.back().number         = _FrameNumber;
        _Frame.back().sync = rhi::InvalidHandle;
        _Frame.back().readyToExecute = false;

Trace("\n\n-------------------------------\nframe %u started\n",_FrameNumber);
        _FrameStarted = true;
        ++_FrameNumber;
    }

    _Frame.back().pass.push_back( pass );

    _FrameSync.Unlock();
//-    _CmdQueue.push_back( pass );
}


//------------------------------------------------------------------------------

static void
dx11_RenderPass_End( Handle pass )
{
}


//------------------------------------------------------------------------------

namespace RenderPassDX11
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_Renderpass_Allocate  = &dx11_RenderPass_Allocate;
    dispatch->impl_Renderpass_Begin     = &dx11_RenderPass_Begin;
    dispatch->impl_Renderpass_End       = &dx11_RenderPass_End;
}

}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_Begin( Handle cmdBuf )
{
    CommandBufferDX11_t*    cb          = CommandBufferPool::Get( cmdBuf );
    ID3D11DeviceContext*    context     = cb->context;
    bool                    clear_color = cb->isFirstInPass && cb->passCfg.colorBuffer[0].loadAction == LOADACTION_CLEAR;
    bool                    clear_depth = cb->isFirstInPass && cb->passCfg.depthStencilBuffer.loadAction == LOADACTION_CLEAR;
    ID3D11RenderTargetView* rt[1]       = { _D3D11_RenderTargetView };
    
    cb->cur_topo                = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    cb->cur_ib                  = InvalidIndex;
    cb->cur_vb                  = InvalidIndex;
    cb->cur_vb_stride           = 0;
    cb->cur_pipelinestate       = InvalidHandle;
    cb->cur_stride              = 0;
    cb->cur_query_buf           = InvalidHandle;
    cb->cur_query_i             = InvalidIndex;
    cb->cur_rs                  = nullptr;

    cb->rs_param.cullMode       = CULL_NONE;
    cb->rs_param.scissorEnabled = false;

    cb->sync                    = InvalidHandle;

    cb->def_viewport.TopLeftX   = 0;
    cb->def_viewport.TopLeftY   = 0;
    cb->def_viewport.MinDepth   = 0.0f;
    cb->def_viewport.MaxDepth   = 1.0f;


    if(     cb->passCfg.colorBuffer[0].texture != rhi::InvalidHandle
        &&  cb->passCfg.colorBuffer[0].texture != rhi::DefaultDepthBuffer
      )
    {
        Size2i  sz = TextureDX11::Size( cb->passCfg.colorBuffer[0].texture );
                        
        cb->def_viewport.Width  = float(sz.dx);
        cb->def_viewport.Height = float(sz.dy);

        TextureDX11::SetRenderTarget( cb->passCfg.colorBuffer[0].texture, cb->passCfg.depthStencilBuffer.texture, context );
    }
    else
    {
        context->OMSetRenderTargets( 1, rt, _D3D11_DepthStencilView );
    }


    ID3D11RenderTargetView* rt_view[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };
    ID3D11DepthStencilView* ds_view = NULL;
                    
    context->OMGetRenderTargets( countof(rt_view), rt_view, &ds_view );

    for( unsigned i=0; i!=countof(rt_view); ++i )
    {
        if( rt_view[i] )
        {
            if( i == 0 )
            {
                if( cb->passCfg.colorBuffer[0].texture == rhi::InvalidHandle )
                {
                    D3D11_TEXTURE2D_DESC    desc;

                    _D3D11_SwapChainBuffer->GetDesc( &desc );

                    cb->def_viewport.Width  = float(desc.Width);
                    cb->def_viewport.Height = float(desc.Height);                                
                }
                                                                
                context->RSSetViewports( 1, &(cb->def_viewport) );
            }
                            
            if( clear_color )
                context->ClearRenderTargetView( rt_view[i], cb->passCfg.colorBuffer[0].clearColor );
                            
            rt_view[i]->Release();
        }
    }

    if( ds_view )
    {
        if( clear_depth )
            context->ClearDepthStencilView( ds_view, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, cb->passCfg.depthStencilBuffer.clearDepth, cb->passCfg.depthStencilBuffer.clearStencil );
                        
        ds_view->Release();
    }

    context->IASetPrimitiveTopology( cb->cur_topo );
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_End( Handle cmdBuf, Handle syncObject )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );

    cb->context->FinishCommandList( TRUE, &(cb->commandList) );
    cb->sync = syncObject;
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetPipelineState( Handle cmdBuf, Handle ps, uint32 vdeclUID )
{
    CommandBufferDX11_t*    cb    = CommandBufferPool::Get( cmdBuf );
    const VertexLayout*     vdecl = (vdeclUID == VertexLayout::InvalidUID)  
                                    ? nullptr  
                                    : VertexLayout::Get( vdeclUID );
                
    cb->cur_pipelinestate = ps;
    cb->cur_vb_stride     = (vdecl)  ? vdecl->Stride()  : 0;

    PipelineStateDX11::SetToRHI( ps, vdeclUID, cb->context );
    StatSet::IncStat( stat_SET_PS, 1 );
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetCullMode( Handle cmdBuf, CullMode mode )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );

    cb->rs_param.cullMode = mode;
    cb->cur_rs = nullptr;
}


//------------------------------------------------------------------------------

void
dx11_CommandBuffer_SetScissorRect( Handle cmdBuf, ScissorRect rect )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );
    int                     x  = rect.x;
    int                     y  = rect.y;
    int                     w  = rect.width;
    int                     h  = rect.height;

    if( !(x==0  &&  y==0  &&  w==0  &&  h==0) )
    {
        D3D11_RECT  rect = { x, y, x+w-1, y+h-1 };

        cb->rs_param.scissorEnabled = true;
        cb->cur_rs                  = nullptr;

        cb->context->RSSetScissorRects( 1, &rect );
    }
    else
    {
        cb->rs_param.scissorEnabled = false;
        cb->cur_rs = nullptr;
    }
}


//------------------------------------------------------------------------------

void
dx11_CommandBuffer_SetViewport( Handle cmdBuf, Viewport vp )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );
    int                     x  = vp.x;
    int                     y  = vp.y;
    int                     w  = vp.width;
    int                     h  = vp.height;

    if( !(x==0  &&  y==0  &&  w==0  &&  h==0) )
    {
        D3D11_VIEWPORT  vp;

        vp.TopLeftX = float(x);
        vp.TopLeftY = float(y);
        vp.Width    = float(w);
        vp.Height   = float(h);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
                    
        cb->context->RSSetViewports( 1, &vp );
    }
    else
    {
        cb->context->RSSetViewports( 1, &(cb->def_viewport) );
    }
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetVertexData( Handle cmdBuf, Handle vb, uint32 streamIndex )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );

    cb->cur_vb        = vb;
    cb->cur_vb_stride = (cb->cur_vb_stride) 
                        ? cb->cur_vb_stride
                        : PipelineStateDX11::VertexLayoutStride( cb->cur_pipelinestate );
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetVertexConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );

    ConstBufferDX11::SetToRHI( buffer, cb->context );
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetVertexTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );

    TextureDX11::SetToRHIVertex( tex, unitIndex, cb->context );
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetIndices( Handle cmdBuf, Handle ib )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );

    cb->cur_ib = ib;
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetQueryIndex( Handle cmdBuf, uint32 objectIndex )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );

    cb->cur_query_i = objectIndex;
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetQueryBuffer( Handle cmdBuf, Handle queryBuf )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );

    DVASSERT(cb->cur_query_buf == InvalidHandle);
    cb->cur_query_buf = queryBuf;
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetFragmentConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );

    ConstBufferDX11::SetToRHI( buffer, cb->context );
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetFragmentTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );

    TextureDX11::SetToRHIFragment( tex, unitIndex, cb->context );
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetDepthStencilState( Handle cmdBuf, Handle depthStencilState )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );

    DepthStencilStateDX11::SetToRHI( depthStencilState, cb->context );
}


//------------------------------------------------------------------------------

static void 
dx11_CommandBuffer_SetSamplerState( Handle cmdBuf, const Handle samplerState )
{
    CommandBufferDX11_t*    cb = CommandBufferPool::Get( cmdBuf );

    SamplerStateDX11::SetToRHI( samplerState, cb->context );
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_DrawPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count )
{
    CommandBufferDX11_t*        cb          = CommandBufferPool::Get( cmdBuf );
    ID3D11DeviceContext*        ctx         = cb->context;
    unsigned                    vertexCount = 0;
    D3D11_PRIMITIVE_TOPOLOGY    topo        = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    INT                         baseVertex  = 0;
    
    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            vertexCount = count*3;
            topo        = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;

        case PRIMITIVE_TRIANGLESTRIP :
            vertexCount = 2 + count;
            topo        = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            break;

        case PRIMITIVE_LINELIST :
            vertexCount = count*2;
            topo        = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
    }

    if( topo != cb->cur_topo )
    {
        ctx->IASetPrimitiveTopology( topo );
        cb->cur_topo = topo;
    }

    if( !cb->cur_rs )
    {
        cb->cur_rs = _GetRasterizerState( cb->rs_param );
        ctx->RSSetState( cb->cur_rs );
    }

    VertexBufferDX11::SetToRHI( cb->cur_vb, 0, 0, cb->cur_vb_stride, ctx );

    if( cb->cur_query_i != InvalidIndex )
        QueryBufferDX11::BeginQuery( cb->cur_query_buf, cb->cur_query_i, ctx );
                
    ctx->Draw( vertexCount, baseVertex );
                
    if( cb->cur_query_i != InvalidIndex )
        QueryBufferDX11::EndQuery( cb->cur_query_buf, cb->cur_query_i, ctx );    
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_DrawIndexedPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count, uint32 vertexCount, uint32 firstVertex, uint32 startIndex )
{
    CommandBufferDX11_t*        cb          = CommandBufferPool::Get( cmdBuf );
    ID3D11DeviceContext*        ctx         = cb->context;
    unsigned                    indexCount  = 0;
    D3D11_PRIMITIVE_TOPOLOGY    topo        = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            indexCount  = count*3;
            topo        = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;

        case PRIMITIVE_TRIANGLESTRIP :
            indexCount  = 2 + count;
            topo        = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            break;

        case PRIMITIVE_LINELIST :
            indexCount  = count*2;
            topo        = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
    }

    if( topo != cb->cur_topo )
    {
        ctx->IASetPrimitiveTopology( topo );
        cb->cur_topo = topo;
    }

    if( !cb->cur_rs )
    {
        cb->cur_rs = _GetRasterizerState( cb->rs_param );
        ctx->RSSetState( cb->cur_rs );
    }

    IndexBufferDX11::SetToRHI( cb->cur_ib, 0, ctx );
    VertexBufferDX11::SetToRHI( cb->cur_vb, 0, 0, cb->cur_vb_stride, ctx );
                
    if( cb->cur_query_i != InvalidIndex )
        QueryBufferDX11::BeginQuery( cb->cur_query_buf, cb->cur_query_i, ctx );
                
    ctx->DrawIndexed( indexCount, startIndex, firstVertex );    

    if( cb->cur_query_i != InvalidIndex )
        QueryBufferDX11::BeginQuery( cb->cur_query_buf, cb->cur_query_i, ctx );
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetMarker( Handle cmdBuf, const char* text )
{
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);
    wchar_t              txt[128];

    ::MultiByteToWideChar( CP_ACP, 0, text, -1, txt, countof(txt));
    
    if( cb->contextAnnotation )
    {
        cb->contextAnnotation->SetMarker( txt );
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
    Handle              handle = SyncObjectPool::Alloc();
    SyncObjectDX11_t*   sync   = SyncObjectPool::Get( handle );

    sync->is_signaled = false;

    return handle;
}


//------------------------------------------------------------------------------

static void
dx11_SyncObject_Delete( Handle obj )
{
    SyncObjectPool::Free( obj );
}


//------------------------------------------------------------------------------

static bool
dx11_SyncObject_IsSignaled( Handle obj )
{
    bool                signaled = false;
    SyncObjectDX11_t*   sync     = SyncObjectPool::Get( obj );
    
    if( sync )
        signaled = sync->is_signaled;

    return signaled;
}


//------------------------------------------------------------------------------

static void
_ExecuteQueuedCommandsDX11()
{
#ifdef __DAVAENGINE_WIN_UAP__
    // this hack need removed, when rhi_dx thread will synchronized with rander::reset
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__
    DAVA::UniqueLock<DAVA::Mutex> lock(need_synchronized);
#endif

Trace("rhi-dx11.exec-queued-cmd\n");

    std::vector<RenderPassDX11_t*>  pass;
    std::vector<Handle>             pass_h;
    unsigned                        frame_n = 0;
    bool                            do_exit = false;

    _FrameSync.Lock();
    if( _Frame.size() )
    {
        for( std::vector<Handle>::iterator p=_Frame.begin()->pass.begin(),p_end=_Frame.begin()->pass.end(); p!=p_end; ++p )
        {
            RenderPassDX11_t*   pp      = RenderPassPool::Get( *p );
            bool                do_add  = true;
            
            for( unsigned i=0; i!=pass.size(); ++i )
            {
                if( pp->priority > pass[i]->priority )
                {
                    pass.insert( pass.begin()+i, 1, pp );
                    do_add = false;
                    break;
                }
            }
            
            if( do_add )
                pass.push_back( pp );
        }

        pass_h  = _Frame.begin()->pass;
        frame_n = _Frame.begin()->number;
    }
    else
    {
        do_exit = true;
    }
    _FrameSync.Unlock();

    if( do_exit )
        return;

Trace("\n\n-------------------------------\nexecuting frame %u\n",frame_n);
    for( std::vector<RenderPassDX11_t*>::iterator p=pass.begin(),p_end=pass.end(); p!=p_end; ++p )
    {
        RenderPassDX11_t*   pp = *p;

        for( unsigned b=0; b!=pp->cmdBuf.size(); ++b )
        {
            Handle               cb_h = pp->cmdBuf[b];
            CommandBufferDX11_t* cb   = CommandBufferPool::Get( cb_h );
            
            cb->Execute();

            if( cb->sync != InvalidHandle )
            {
                SyncObjectDX11_t*   sync = SyncObjectPool::Get( cb->sync );

                sync->frame       = frame_n;
                sync->is_signaled = false;
            }

            CommandBufferPool::Free( cb_h );
        }
        
//        RenderPassPool::Free( *p );
    }

    _FrameSync.Lock();
    {
Trace("\n\n-------------------------------\nframe %u executed(submitted to GPU)\n",frame_n);
        _Frame.erase( _Frame.begin() );
        
        for( std::vector<Handle>::iterator p=pass_h.begin(),p_end=pass_h.end(); p!=p_end; ++p )
            RenderPassPool::Free( *p );
    }    
    _FrameSync.Unlock();

    
    // do flip, reset/restore device if necessary

//    HRESULT hr;
/*
    if( _ResetPending )
    {
        hr = _D3D9_Device->TestCooperativeLevel();

        if( hr == D3DERR_DEVICENOTRESET )
        {
///            reset( Size2i(_present_param->BackBufferWidth,_present_param->BackBufferHeight) );

            _ResetPending = false;
        }
        else
        {
            ::Sleep( 100 );
        }
    }
    else
*/    
    {
        _D3D11_SwapChain->Present( 0, 0 );
    }    


    // update sync-objects

    for( SyncObjectPool::Iterator s=SyncObjectPool::Begin(),s_end=SyncObjectPool::End(); s!=s_end; ++s )
    {
        if( frame_n - s->frame > 3 )
            s->is_signaled = true;
    }
}


//------------------------------------------------------------------------------

static void
_ExecDX11( DX11Command* command, uint32 cmdCount )
{
#if 1
    #define CHECK_HR(hr) \
    if( FAILED(hr) ) \
        Logger::Error( "%s", D3D11ErrorText(hr) );
#else
    CHECK_HR(hr)
#endif

    for( DX11Command* cmd=command,*cmdEnd=command+cmdCount; cmd!=cmdEnd; ++cmd )
    {
        const uint64*   arg = cmd->arg;

Trace("exec %i\n",int(cmd->func));
        switch( cmd->func )
        {
            case DX11Command::NOP :
                break;
            
            case DX11Command::MAP :
                cmd->retval = _D3D11_ImmediateContext->Map( (ID3D11Resource*)(arg[0]), UINT(arg[1]), D3D11_MAP(arg[2]), UINT(arg[3]), (D3D11_MAPPED_SUBRESOURCE*)(arg[4]) );
                break;
            
            case DX11Command::UNMAP :
                _D3D11_ImmediateContext->Unmap( (ID3D11Resource*)(arg[0]), UINT(arg[1]) );
                break;
            
            case DX11Command::UPDATE_SUBRESOURCE :
                _D3D11_ImmediateContext->UpdateSubresource( (ID3D11Resource*)(arg[0]), UINT(arg[1]), (const D3D11_BOX*)(arg[2]), (const void*)(arg[3]), UINT(arg[4]), UINT(arg[5]) );
                break;

            default:
                DVASSERT(!"unknown DX11-cmd");
        }
    }

    #undef CHECK_HR
}


//------------------------------------------------------------------------------

void
ExecDX11( DX11Command* command, uint32 cmdCount, bool force_immediate )
{
    if( force_immediate  ||  !_DX11_RenderThreadFrameCount )
    {
        _ExecDX11( command, cmdCount );
    }
    else
    {
        bool    scheduled = false;
        bool    executed  = false;
    
        // CRAP: busy-wait
        do
        {
            _DX11_PendingImmediateCmdSync.Lock();
            if( !_DX11_PendingImmediateCmd )
            {
                _DX11_PendingImmediateCmd      = command;
                _DX11_PendingImmediateCmdCount = cmdCount;
                scheduled                     = true;
            }
            _DX11_PendingImmediateCmdSync.Unlock();
        } while( !scheduled );

        // CRAP: busy-wait
        do
        {
            _DX11_PendingImmediateCmdSync.Lock();
            if( !_DX11_PendingImmediateCmd )
            {
                executed = true;
            }
            _DX11_PendingImmediateCmdSync.Unlock();
        } while( !executed );
    }
}


//------------------------------------------------------------------------------

static void
_RenderFuncDX11( DAVA::BaseObject* obj, void*, void* )
{
    _InitDX11();

    _DX11_RenderThreadStartedSync.Post();
    Trace( "RHI render-thread started\n" );

    while( true )
    {
        bool    do_wait = true;
        bool    do_exit = false;
        
        do
        {
            // CRAP: busy-wait
            _DX11_RenderThreadExitSync.Lock();
            do_exit = _DX11_RenderThreadExitPending;
            _DX11_RenderThreadExitSync.Unlock();
            
            if( do_exit )
                break;

            _DX11_PendingImmediateCmdSync.Lock();
            if( _DX11_PendingImmediateCmd )
            {
Trace("exec imm cmd (%u)\n",_DX11_PendingImmediateCmdCount);
                _ExecDX11( _DX11_PendingImmediateCmd, _DX11_PendingImmediateCmdCount );
                _DX11_PendingImmediateCmd      = nullptr;
                _DX11_PendingImmediateCmdCount = 0;
Trace("exec-imm-cmd done\n");
            }
            _DX11_PendingImmediateCmdSync.Unlock();
            
            _FrameSync.Lock();
            do_wait = !( _Frame.size()  &&  _Frame.begin()->readyToExecute );
            _FrameSync.Unlock();
        } while( do_wait );

        if( do_exit )
            break;

        _ExecuteQueuedCommandsDX11();
    }

    Trace( "RHI render-thread stopped\n" );
}

void
InitializeRenderThreadDX11( uint32 frameCount )
{
    _DX11_RenderThreadFrameCount = frameCount;

    if( _DX11_RenderThreadFrameCount )
    {
        _DX11_RenderThread = DAVA::Thread::Create( DAVA::Message(&_RenderFuncDX11) );
        _DX11_RenderThread->SetName( "RHI.dx11-render" );
        _DX11_RenderThread->Start();    
        _DX11_RenderThreadStartedSync.Wait();
    }
    else
    {
        _InitDX11();
    }
}


//------------------------------------------------------------------------------

void
UninitializeRenderThreadDX11()
{
    if( _DX11_RenderThreadFrameCount )
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
    if( _DX11_RenderThreadFrameCount )
    {
Trace("rhi-dx11.present\n");
        _FrameSync.Lock(); 
        {   
            if( _Frame.size() )
            {
                _Frame.back().readyToExecute = true;
                _FrameStarted = false;
Trace("\n\n-------------------------------\nframe %u generated\n",_Frame.back().number);
            }
        }
        _FrameSync.Unlock();

        unsigned frame_cnt = 0;

        do
        {
        _FrameSync.Lock();
        frame_cnt = _Frame.size();
//Trace("rhi-gl.present frame-cnt= %u\n",frame_cnt);
        _FrameSync.Unlock();
        }
        while( frame_cnt >= _DX11_RenderThreadFrameCount );
    }
    else
    {
        if( _Frame.size() )
        {
            _Frame.back().readyToExecute = true;
            _FrameStarted = false;
        }

        _ExecuteQueuedCommandsDX11(); 
    }
}


//------------------------------------------------------------------------------

CommandBufferDX11_t::CommandBufferDX11_t()
  : context(nullptr),
    contextAnnotation(nullptr),
    commandList(nullptr)
{
}


//------------------------------------------------------------------------------

CommandBufferDX11_t::~CommandBufferDX11_t()
{
}


//------------------------------------------------------------------------------

void
CommandBufferDX11_t::Execute()
{
SCOPED_FUNCTION_TIMING();

    context->Release();
    context = nullptr;

    if (contextAnnotation)
    {
        contextAnnotation->Release();
        contextAnnotation = nullptr;
    }
    
    _D3D11_ImmediateContext->ExecuteCommandList( commandList, FALSE );
    commandList->Release();
    commandList = nullptr;
}


//------------------------------------------------------------------------------



namespace CommandBufferDX11
{
void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_CommandBuffer_Begin                  = &dx11_CommandBuffer_Begin;
    dispatch->impl_CommandBuffer_End                    = &dx11_CommandBuffer_End;
    dispatch->impl_CommandBuffer_SetPipelineState       = &dx11_CommandBuffer_SetPipelineState;
    dispatch->impl_CommandBuffer_SetCullMode            = &dx11_CommandBuffer_SetCullMode;
    dispatch->impl_CommandBuffer_SetScissorRect         = &dx11_CommandBuffer_SetScissorRect;
    dispatch->impl_CommandBuffer_SetViewport            = &dx11_CommandBuffer_SetViewport;
    dispatch->impl_CommandBuffer_SetVertexData          = &dx11_CommandBuffer_SetVertexData;
    dispatch->impl_CommandBuffer_SetVertexConstBuffer   = &dx11_CommandBuffer_SetVertexConstBuffer;
    dispatch->impl_CommandBuffer_SetVertexTexture       = &dx11_CommandBuffer_SetVertexTexture;
    dispatch->impl_CommandBuffer_SetIndices             = &dx11_CommandBuffer_SetIndices;    
    dispatch->impl_CommandBuffer_SetQueryBuffer         = &dx11_CommandBuffer_SetQueryBuffer;
    dispatch->impl_CommandBuffer_SetQueryIndex          = &dx11_CommandBuffer_SetQueryIndex;
    dispatch->impl_CommandBuffer_SetFragmentConstBuffer = &dx11_CommandBuffer_SetFragmentConstBuffer;
    dispatch->impl_CommandBuffer_SetFragmentTexture     = &dx11_CommandBuffer_SetFragmentTexture;
    dispatch->impl_CommandBuffer_SetDepthStencilState   = &dx11_CommandBuffer_SetDepthStencilState;
    dispatch->impl_CommandBuffer_SetSamplerState        = &dx11_CommandBuffer_SetSamplerState;
    dispatch->impl_CommandBuffer_DrawPrimitive          = &dx11_CommandBuffer_DrawPrimitive;
    dispatch->impl_CommandBuffer_DrawIndexedPrimitive   = &dx11_CommandBuffer_DrawIndexedPrimitive;
    dispatch->impl_CommandBuffer_SetMarker              = &dx11_CommandBuffer_SetMarker;

    dispatch->impl_SyncObject_Create                    = &dx11_SyncObject_Create;
    dispatch->impl_SyncObject_Delete                    = &dx11_SyncObject_Delete;
    dispatch->impl_SyncObject_IsSignaled                = &dx11_SyncObject_IsSignaled;
    
    dispatch->impl_Present                              = &dx11_Present;
}
}


//==============================================================================
} // namespace rhi

