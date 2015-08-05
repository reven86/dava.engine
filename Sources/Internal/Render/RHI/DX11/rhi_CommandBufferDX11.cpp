//==============================================================================
//
//  
//
//==============================================================================
//
//  externals:

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

    
    #include <d3d9.h>


    #define RHI__USE_DX11_RENDER_THREAD         0
    #define RHI__DX11_MAX_PREPARED_FRAME_COUNT  2
    #define RHI__DX11_USE_DEFERRED_CONTEXT      0


namespace rhi
{
//==============================================================================

class
CommandBufferDX11_t
{
public:
                CommandBufferDX11_t();
                ~CommandBufferDX11_t();

    void        Begin();
    void        End();
    void        Execute();


    void        Command( uint64 cmd );
    void        Command( uint64 cmd, uint64 arg1 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5, uint64 arg6 );



    RenderPassConfig    passCfg;
    uint32              isFirstInPass:1;
    uint32              isLastInPass:1;
    
    std::vector<uint64> _cmd;
    
    RingBuffer*         text;

    Handle              sync;
};

enum
CommandDX11
{
    DX11__BEGIN,
    DX11__END,

    DX11__SET_VERTEX_DATA,
    DX11__SET_INDICES,
    DX11__SET_QUERY_BUFFER,
    DX11__SET_QUERY_INDEX,

    DX11__SET_PIPELINE_STATE,
    DX11__SET_CULL_MODE,
    DX11__SET_SCISSOR_RECT,
    DX11__SET_VIEWPORT,
    DX11__SET_VERTEX_PROG_CONST_BUFFER,
    DX11__SET_FRAGMENT_PROG_CONST_BUFFER,
    DX11__SET_TEXTURE,

    DX11__SET_DEPTHSTENCIL_STATE,
    DX11__SET_SAMPLER_STATE,

    DX11__DRAW_PRIMITIVE,
    DX11__DRAW_INDEXED_PRIMITIVE,

    DX11__DEBUG_MARKER,


    DX11__NOP
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

typedef ResourcePool<CommandBufferDX11_t,RESOURCE_COMMAND_BUFFER>   CommandBufferPool;
typedef ResourcePool<RenderPassDX11_t,RESOURCE_RENDER_PASS>         RenderPassPool;
typedef ResourcePool<SyncObjectDX11_t,RESOURCE_SYNC_OBJECT>         SyncObjectPool;

RHI_IMPL_POOL(CommandBufferDX11_t,RESOURCE_COMMAND_BUFFER);
RHI_IMPL_POOL(RenderPassDX11_t,RESOURCE_RENDER_PASS);
RHI_IMPL_POOL(SyncObjectDX11_t,RESOURCE_SYNC_OBJECT);



struct
FrameDX11
{
    unsigned            number;
    std::vector<Handle> pass;
    uint32              readyToExecute:1;
};

static std::vector<FrameDX11>   _Frame;
static bool                     _FrameStarted   = false;
static unsigned                 _FrameNumber    = 1;
//static DAVA::Spinlock       _FrameSync;
static DAVA::Mutex              _FrameSync;

static void _ExecuteQueuedCommands();


#if RHI__USE_DX11_RENDER_THREAD
static DAVA::Thread*        _DX11_RenderThread              = nullptr;
static bool                 _DX11_RenderThreadExitPending   = false;
static DAVA::Spinlock       _DX11_RenderThreadExitSync;
static DAVA::Semaphore      _DX11_RenderThreadStartedSync   (0);
#endif

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
        desc.DepthClipEnable        = FALSE;
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
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
    CommandBufferPool::Get(cmdBuf)->Command( DX11__BEGIN );
#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_End( Handle cmdBuf, Handle syncObject )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
    CommandBufferPool::Get(cmdBuf)->Command( DX11__END, syncObject );
#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetPipelineState( Handle cmdBuf, Handle ps, uint32 vdecl )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
    CommandBufferPool::Get(cmdBuf)->Command( DX11__SET_PIPELINE_STATE, ps, vdecl );
#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetCullMode( Handle cmdBuf, CullMode mode )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
    CommandBufferPool::Get(cmdBuf)->Command( DX11__SET_CULL_MODE, mode );
#endif
}


//------------------------------------------------------------------------------

void
dx11_CommandBuffer_SetScissorRect( Handle cmdBuf, ScissorRect rect )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
    CommandBufferPool::Get(cmdBuf)->Command( DX11__SET_SCISSOR_RECT, rect.x, rect.y, rect.width, rect.height );
#endif
}


//------------------------------------------------------------------------------

void
dx11_CommandBuffer_SetViewport( Handle cmdBuf, Viewport vp )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
    CommandBufferPool::Get(cmdBuf)->Command( DX11__SET_VIEWPORT, vp.x, vp.y, vp.width, vp.height );
#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetVertexData( Handle cmdBuf, Handle vb, uint32 streamIndex )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
    CommandBufferPool::Get(cmdBuf)->Command( DX11__SET_VERTEX_DATA, vb, streamIndex );
#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetVertexConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);
    
    if( buffer != InvalidIndex )
        CommandBufferPool::Get(cmdBuf)->Command( DX11__SET_VERTEX_PROG_CONST_BUFFER, bufIndex, (uint64)(buffer), (uint64)(ConstBufferDX11::InstData(buffer)) );
#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetVertexTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetIndices( Handle cmdBuf, Handle ib )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
    CommandBufferPool::Get(cmdBuf)->Command( DX11__SET_INDICES, ib );
#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetQueryIndex( Handle cmdBuf, uint32 objectIndex )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetQueryBuffer( Handle cmdBuf, Handle queryBuf )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetFragmentConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);
    
    if( buffer != InvalidIndex )
        CommandBufferPool::Get(cmdBuf)->Command( DX11__SET_FRAGMENT_PROG_CONST_BUFFER, bufIndex, (uint64)(buffer), (uint64)(ConstBufferDX11::InstData(buffer)) );
#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetFragmentTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
        CommandBufferPool::Get(cmdBuf)->Command( DX11__SET_TEXTURE, tex, unitIndex );
#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetDepthStencilState( Handle cmdBuf, Handle depthStencilState )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
    CommandBufferPool::Get(cmdBuf)->Command( DX11__SET_DEPTHSTENCIL_STATE, depthStencilState );
#endif
}


//------------------------------------------------------------------------------

static void 
dx11_CommandBuffer_SetSamplerState( Handle cmdBuf, const Handle samplerState )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
    CommandBufferPool::Get(cmdBuf)->Command( DX11__SET_SAMPLER_STATE, samplerState );
#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_DrawPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else

    unsigned                    v_cnt   = 0;
    D3D11_PRIMITIVE_TOPOLOGY    topo    = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            v_cnt = count*3;
            topo  = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;

        case PRIMITIVE_LINELIST :
            v_cnt = count*2;
            topo  = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
    }

    CommandBufferPool::Get(cmdBuf)->Command( DX11__DRAW_PRIMITIVE, topo, v_cnt, 0 );

#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_DrawIndexedPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count, uint32 vertexCount, uint32 firstVertex, uint32 startIndex )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else

    unsigned                    i_cnt   = 0;
    D3D11_PRIMITIVE_TOPOLOGY    topo    = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            i_cnt = count*3;
            topo  = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;

        case PRIMITIVE_LINELIST :
            i_cnt = count*2;
            topo  = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
    }

    CommandBufferPool::Get(cmdBuf)->Command( DX11__DRAW_INDEXED_PRIMITIVE, topo, i_cnt, startIndex, firstVertex );

#endif
}


//------------------------------------------------------------------------------

static void
dx11_CommandBuffer_SetMarker( Handle cmdBuf, const char* text )
{
#if RHI__DX11_USE_DEFERRED_CONTEXT
#else
    CommandBufferDX11_t* cb = CommandBufferPool::Get(cmdBuf);

    if( !cb->text )
    {
        cb->text = new RingBuffer();
        cb->text->Initialize( 64*1024 );
    }
    
    int     len = strlen( text );
    char*   txt = (char*)cb->text->Alloc( len/sizeof(float)+1 );

    memcpy( txt, text, len );
    txt[len] = '\0';

    cb->Command( DX11__DEBUG_MARKER, (uint64)(txt) );
#endif
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

#if RHI__USE_DX11_RENDER_THREAD
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
        
        // CRAP: busy-wait
        do
        {
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


//            _CmdQueueSync.Lock();
//            cnt = _RenderQueue.size();
//            _CmdQueueSync.Unlock();
            _FrameSync.Lock();
            do_wait = !( _Frame.size()  &&  _Frame.begin()->readyToExecute );
            _FrameSync.Unlock();
        } while( do_wait );

        if( do_exit )
            break;

        _ExecuteQueuedCommands();
    }

    Trace( "RHI render-thread stopped\n" );
}
#endif

void
InitializeRenderThreadDX11()
{
#if RHI__USE_DX11_RENDER_THREAD

    _DX11_RenderThread = DAVA::Thread::Create( DAVA::Message(&_RenderFuncDX11) );
    _DX11_RenderThread->SetName( "RHI.dx11-render" );
    _DX11_RenderThread->Start();    
    _DX11_RenderThreadStartedSync.Wait();

#else

    _InitDX11();

#endif
}


//------------------------------------------------------------------------------

void
UninitializeRenderThreadDX11()
{
#if RHI__USE_DX11_RENDER_THREAD
    _DX11_RenderThreadExitSync.Lock();
    _DX11_RenderThreadExitPending = true;
    _DX11_RenderThreadExitSync.Unlock();

    _DX11_RenderThread->Join();
#endif
}


//------------------------------------------------------------------------------

static void
dx11_Present()
{
#if RHI__USE_DX11_RENDER_THREAD

Trace("rhi-dx11.present\n");

    _FrameSync.Lock(); 
    {   
        if( _Frame.size() )
        {
            _Frame.back().readyToExecute = true;
            _FrameStarted = false;
Trace("\n\n-------------------------------\nframe %u generated\n",_Frame.back().number);
        }

//        _FrameStarted = false;
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
    while( frame_cnt >= RHI__DX11_MAX_PREPARED_FRAME_COUNT );

#else
    
    if( _Frame.size() )
    {
        _Frame.back().readyToExecute = true;
        _FrameStarted = false;
    }

    _ExecuteQueuedCommandsDX11(); 

#endif

    ConstBufferDX11::InvalidateAllConstBufferInstances();
}


//------------------------------------------------------------------------------

CommandBufferDX11_t::CommandBufferDX11_t()
  : text(nullptr)
{
}


//------------------------------------------------------------------------------

CommandBufferDX11_t::~CommandBufferDX11_t()
{
}


//------------------------------------------------------------------------------

void
CommandBufferDX11_t::Begin()
{
}


//------------------------------------------------------------------------------

void
CommandBufferDX11_t::End()
{
}


//------------------------------------------------------------------------------

void
CommandBufferDX11_t::Command( uint64 cmd )
{
    _cmd.push_back( cmd );
}


//------------------------------------------------------------------------------

void
CommandBufferDX11_t::Command( uint64 cmd, uint64 arg1 )
{
    _cmd.resize( _cmd.size()+1+1 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+1);

    b[0] = cmd; 
    b[1] = arg1; 
}


//------------------------------------------------------------------------------

void
CommandBufferDX11_t::Command( uint64 cmd, uint64 arg1, uint64 arg2 )
{
    _cmd.resize( _cmd.size()+1+2 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+2);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
}


//------------------------------------------------------------------------------

void
CommandBufferDX11_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3 )
{
    _cmd.resize( _cmd.size()+1+3 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+3);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
}


//------------------------------------------------------------------------------

void
CommandBufferDX11_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4 )
{
    _cmd.resize( _cmd.size()+1+4 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+4);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
    b[4] = arg4; 
}


//------------------------------------------------------------------------------

void
CommandBufferDX11_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5 )
{
    _cmd.resize( _cmd.size()+1+5 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+5);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
    b[4] = arg4; 
    b[5] = arg5; 
}


//------------------------------------------------------------------------------

inline void
CommandBufferDX11_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5, uint64 arg6 )
{
    _cmd.resize( _cmd.size()+1+6 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+6);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
    b[4] = arg4; 
    b[5] = arg5; 
    b[6] = arg6; 
}


//------------------------------------------------------------------------------

void
CommandBufferDX11_t::Execute()
{
SCOPED_FUNCTION_TIMING();
    D3D11_PRIMITIVE_TOPOLOGY    cur_topo            = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    Handle                      cur_ib              = InvalidIndex;
    Handle                      cur_vb              = InvalidIndex;
    Handle                      cur_vb_stride       = 0;
    Handle                      cur_pipelinestate   = InvalidHandle;
    uint32                      cur_stride          = 0;
    Handle                      cur_query_buf       = InvalidHandle;
    uint32                      cur_query_i         = InvalidIndex;
    D3D11_VIEWPORT              def_viewport;
    RasterizerParamDX11         rs_param;
    ID3D11RasterizerState*      cur_rs              = nullptr;

    rs_param.cullMode       = CULL_NONE;
    rs_param.scissorEnabled = false;

    sync = InvalidHandle;

    def_viewport.TopLeftX = 0;
    def_viewport.TopLeftY = 0;
    def_viewport.MinDepth = 0.0f;
    def_viewport.MaxDepth = 1.0f;

    for( std::vector<uint64>::const_iterator c=_cmd.begin(),c_end=_cmd.end(); c!=c_end; ++c )
    {
        const uint64                        cmd = *c;
        std::vector<uint64>::const_iterator arg = c+1;

//        if( cmd == EndCmd )
//            break;

        switch( cmd )
        {
            case DX11__BEGIN :
            {
                if( isFirstInPass )
                {
                    bool                    clear_color = passCfg.colorBuffer[0].loadAction == LOADACTION_CLEAR;
                    bool                    clear_depth = passCfg.depthStencilBuffer.loadAction == LOADACTION_CLEAR;
                    ID3D11RenderTargetView* rt[1]       = { _D3D11_RenderTargetView };

                    if( passCfg.colorBuffer[0].texture != rhi::InvalidHandle )
                    {
                        Size2i  sz = TextureDX11::Size( passCfg.colorBuffer[0].texture );
                        
                        def_viewport.Width  = float(sz.dx);
                        def_viewport.Height = float(sz.dy);

                        TextureDX11::SetRenderTarget( passCfg.colorBuffer[0].texture, passCfg.depthStencilBuffer.texture );
                    }
                    else
                    {
                        _D3D11_ImmediateContext->OMSetRenderTargets( 1, rt, _D3D11_DepthStencilView );
                    }


                    ID3D11RenderTargetView* rt_view[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };
                    ID3D11DepthStencilView* ds_view = NULL;
                    
                    _D3D11_ImmediateContext->OMGetRenderTargets( countof(rt_view), rt_view, &ds_view );

                    for( unsigned i=0; i!=countof(rt_view); ++i )
                    {
                        if( rt_view[i] )
                        {
                            if( i == 0 )
                            {
                                if( passCfg.colorBuffer[0].texture == rhi::InvalidHandle )
                                {
                                    D3D11_TEXTURE2D_DESC    desc;

                                    _D3D11_SwapChainBuffer->GetDesc( &desc );

                                    def_viewport.Width  = float(desc.Width);
                                    def_viewport.Height = float(desc.Height);                                
                                }
                                                                
                                _D3D11_ImmediateContext->RSSetViewports( 1, &def_viewport );
                            }
                            
                            if( clear_color )
                                _D3D11_ImmediateContext->ClearRenderTargetView( rt_view[i], passCfg.colorBuffer[0].clearColor );
                            
                            rt_view[i]->Release();
                        }
                    }

                    if( ds_view )
                    {
                        if( clear_depth )
                            _D3D11_ImmediateContext->ClearDepthStencilView( ds_view, D3D11_CLEAR_DEPTH, passCfg.depthStencilBuffer.clearDepth, 0 );
                        
                        ds_view->Release();
                    }
                }

                _D3D11_ImmediateContext->IASetPrimitiveTopology( cur_topo );
            }   break;
            
            case DX11__END :
            {
                sync = Handle(arg[0]);

                if( isLastInPass )
                {
                }
                c += 1;
            }   break;

            case DX11__SET_PIPELINE_STATE :
            {
                uint32              vd_uid = (uint32)(arg[1]);
                const VertexLayout* vdecl  = (vd_uid == VertexLayout::InvalidUID)  
                                             ? nullptr  
                                             : VertexLayout::Get( vd_uid );
                
                cur_pipelinestate = (Handle)(arg[0]);
                cur_vb_stride     = (vdecl)  ? vdecl->Stride()  : 0;


                PipelineStateDX11::SetToRHI( cur_pipelinestate, vd_uid );

                StatSet::IncStat( stat_SET_PS, 1 );
                c += 2;
            }   break;

            case DX11__SET_CULL_MODE :
            {
                rs_param.cullMode = CullMode(arg[0]);
                cur_rs = nullptr;
                c += 1;
            }   break;
            
            case DX11__SET_VERTEX_PROG_CONST_BUFFER :
            {
//                unsigned    buf_i = arg[0];
                Handle      cb   = Handle(arg[1]);
                const void* inst = (const void*)(arg[2]);

                ConstBufferDX11::SetToRHI( cb, inst );
                c += 3;
            }   break;

            case DX11__SET_FRAGMENT_PROG_CONST_BUFFER :
            {
//                unsigned    buf_i = arg[0];
                Handle      cb   = Handle(arg[1]);
                const void* inst = (const void*)(arg[2]);

                ConstBufferDX11::SetToRHI( cb, inst );
                c += 3;
            }   break;
            
            case DX11__SET_SCISSOR_RECT :
            {
                int x = int(arg[0]);
                int y = int(arg[1]);
                int w = int(arg[2]);
                int h = int(arg[3]);

                if( !(x==0  &&  y==0  &&  w==0  &&  h==0) )
                {
                    D3D11_RECT  rect = { x, y, x+w-1, y+h-1 };

                    rs_param.scissorEnabled = true;
                    cur_rs                  = nullptr;
                    _D3D11_ImmediateContext->RSSetScissorRects( 1, &rect );
                }
                else
                {
                    rs_param.scissorEnabled = false;
                    cur_rs = nullptr;
                }

                c += 4;
            }   break;

            case DX11__SET_VIEWPORT :
            {
                int x = int(arg[0]);
                int y = int(arg[1]);
                int w = int(arg[2]);
                int h = int(arg[3]);

                if( !(x==0  &&  y==0  &&  w==0  &&  h==0) )
                {
                    D3D11_VIEWPORT  vp;

                    vp.TopLeftX = float(x);
                    vp.TopLeftY = float(y);
                    vp.Width    = float(w);
                    vp.Height   = float(h);
                    vp.MinDepth = 0.0f;
                    vp.MaxDepth = 1.0f;
                    
                    _D3D11_ImmediateContext->RSSetViewports( 1, &vp );
                }
                else
                {
                    _D3D11_ImmediateContext->RSSetViewports( 1, &def_viewport );
                }

                c += 4;
            }   break;

            case DX11__SET_DEPTHSTENCIL_STATE :
            {
                Handle  ds = Handle(arg[0]);

                DepthStencilStateDX11::SetToRHI( ds );
                c += 1;
            }   break;

            case DX11__SET_SAMPLER_STATE :
            {
                Handle  ss = Handle(arg[0]);

                SamplerStateDX11::SetToRHI( ss );
                c += 1;
            }   break;

            case DX11__SET_INDICES :
            {
                cur_ib = Handle(arg[0]);
                c += 1;
            }   break;

            case DX11__SET_VERTEX_DATA :
            {
                DVASSERT(cur_pipelinestate != InvalidHandle);
                
                cur_vb        = Handle(arg[0]);
                cur_vb_stride = (cur_vb_stride) 
                                ? cur_vb_stride
                                : PipelineStateDX11::VertexLayoutStride( cur_pipelinestate );

                c += 2;
            }   break;

            case DX11__SET_TEXTURE :
            {
                Handle      tex    = Handle(arg[0]);
                unsigned    unit_i = unsigned(arg[1]);

                TextureDX11::SetToRHI( tex, unit_i );
                c += 2;
            }   break;

            case DX11__DRAW_PRIMITIVE :
            {
                D3D11_PRIMITIVE_TOPOLOGY    topo        = (D3D11_PRIMITIVE_TOPOLOGY)(arg[0]);
                UINT                        vertexCount = (UINT)(arg[1]);
                INT                         baseVertex  = (UINT)(arg[2]);
                
                if( topo != cur_topo )
                {
                    _D3D11_ImmediateContext->IASetPrimitiveTopology( topo );
                    cur_topo = topo;
                }

                if( !cur_rs )
                {
                    cur_rs = _GetRasterizerState( rs_param );
                    _D3D11_ImmediateContext->RSSetState( cur_rs );
                }

                VertexBufferDX11::SetToRHI( cur_vb, 0, 0, cur_vb_stride );

                _D3D11_ImmediateContext->Draw( vertexCount, baseVertex );
                
                c += 3;
            }   break;

            case DX11__DRAW_INDEXED_PRIMITIVE :
            {
                D3D11_PRIMITIVE_TOPOLOGY    topo        = (D3D11_PRIMITIVE_TOPOLOGY)(arg[0]);
                UINT                        indexCount  = (UINT)(arg[1]);
                UINT                        startIndex  = (UINT)(arg[2]);
                INT                         baseVertex  = (UINT)(arg[3]);
                
                if( topo != cur_topo )
                {
                    _D3D11_ImmediateContext->IASetPrimitiveTopology( topo );
                    cur_topo = topo;
                }

                if( !cur_rs )
                {
                    cur_rs = _GetRasterizerState( rs_param );
                    _D3D11_ImmediateContext->RSSetState( cur_rs );
                }

                IndexBufferDX11::SetToRHI( cur_ib, 0 );
                VertexBufferDX11::SetToRHI( cur_vb, 0, 0, cur_vb_stride );
                _D3D11_ImmediateContext->DrawIndexed( indexCount, startIndex, baseVertex );    
                
                c += 4;
            }   break;

            case DX11__DEBUG_MARKER :
            {
                wchar_t txt[128];

                ::MultiByteToWideChar( CP_ACP, 0, (const char*)(arg[0]), -1, txt, countof(txt));
                ::D3DPERF_SetMarker( D3DCOLOR_ARGB(0xFF,0x40,0x40,0x80), txt );
            }   break;

            default:
                DVASSERT("unknown DX11 render-command");
        }
    }

    _cmd.clear();
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

