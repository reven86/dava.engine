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
    #include "rhi_DX9.h"

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

    #include "_dx9.h"
    namespace rhi { extern void _InitDX9(); }
    #include <vector>    


namespace rhi
{
//==============================================================================

static bool _ResetPending   = false;

struct
FrameDX9
{
    unsigned            number;
    Handle              sync;
    std::vector<Handle> pass;
    uint32              readyToExecute:1;
};

static std::vector<FrameDX9>    _Frame;
static bool                     _FrameStarted   = false;
static unsigned                 _FrameNumber    = 1;
//static DAVA::Spinlock       _FrameSync;
static DAVA::Mutex              _FrameSync;

static void _ExecuteQueuedCommands();


static DAVA::Thread*        _DX9_RenderThread               = nullptr;
static unsigned             _DX9_RenderThreadFrameCount     = 0;
static bool                 _DX9_RenderThreadExitPending    = false;
static DAVA::Spinlock       _DX9_RenderThreadExitSync;
static DAVA::Semaphore      _DX9_RenderThreadStartedSync    (0);

static DX9Command*          _DX9_PendingImmediateCmd        = nullptr;
static uint32               _DX9_PendingImmediateCmdCount   = 0;
static DAVA::Mutex          _DX9_PendingImmediateCmdSync;


//------------------------------------------------------------------------------

enum
CommandDX9
{
    DX9__BEGIN,
    DX9__END,

    DX9__SET_VERTEX_DATA,
    DX9__SET_INDICES,
    DX9__SET_QUERY_BUFFER,
    DX9__SET_QUERY_INDEX,

    DX9__SET_PIPELINE_STATE,
    DX9__SET_CULL_MODE,
    DX9__SET_SCISSOR_RECT,
    DX9__SET_VIEWPORT,
    DX9__SET_VERTEX_PROG_CONST_BUFFER,
    DX9__SET_FRAGMENT_PROG_CONST_BUFFER,
    DX9__SET_TEXTURE,

    DX9__SET_DEPTHSTENCIL_STATE,
    DX9__SET_SAMPLER_STATE,

    DX9__DRAW_PRIMITIVE,
    DX9__DRAW_INDEXED_PRIMITIVE,

    DX9__DEBUG_MARKER,


    DX9__NOP
};


class 
RenderPassDX9_t
{
public:
    std::vector<Handle> cmdBuf;
    int                 priority;
};



class
CommandBufferDX9_t
{
public:
                CommandBufferDX9_t();
                ~CommandBufferDX9_t();

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



    static const uint64   EndCmd/* = 0xFFFFFFFF*/;

    std::vector<uint64> _cmd;

    RenderPassConfig    passCfg;
    uint32              isFirstInPass:1;
    uint32              isLastInPass:1;

    RingBuffer*         text;

    Handle              sync;
};


struct
SyncObjectDX9_t 
{
    uint32  frame;
    uint32  is_signaled:1;
    uint32  is_used:1;
};

typedef ResourcePool<CommandBufferDX9_t,RESOURCE_COMMAND_BUFFER,CommandBuffer::Descriptor,false> CommandBufferPool;
typedef ResourcePool<RenderPassDX9_t,RESOURCE_RENDER_PASS,RenderPassConfig,false>               RenderPassPool;
typedef ResourcePool<SyncObjectDX9_t,RESOURCE_SYNC_OBJECT,SyncObject::Descriptor,false>         SyncObjectPool;

RHI_IMPL_POOL(CommandBufferDX9_t,RESOURCE_COMMAND_BUFFER,CommandBuffer::Descriptor,false);
RHI_IMPL_POOL(RenderPassDX9_t,RESOURCE_RENDER_PASS,RenderPassConfig,false);
RHI_IMPL_POOL(SyncObjectDX9_t,RESOURCE_SYNC_OBJECT,SyncObject::Descriptor,false);

    
const uint64   CommandBufferDX9_t::EndCmd = 0xFFFFFFFF;


static Handle
dx9_RenderPass_Allocate( const RenderPassConfig& passDesc, uint32 cmdBufCount, Handle* cmdBuf )
{
    DVASSERT(cmdBufCount);

    Handle              handle = RenderPassPool::Alloc();
    RenderPassDX9_t*    pass   = RenderPassPool::Get( handle );
    
    pass->cmdBuf.resize( cmdBufCount );
    pass->priority = passDesc.priority;

    for( unsigned i=0; i!=cmdBufCount; ++i )
    {
        Handle              h  = CommandBufferPool::Alloc();
        CommandBufferDX9_t* cb = CommandBufferPool::Get( h );

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
dx9_RenderPass_Begin( Handle pass )
{
    _FrameSync.Lock();

    if( !_FrameStarted )
    {
        _Frame.push_back( FrameDX9() );
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
dx9_RenderPass_End( Handle pass )
{
}


namespace RenderPassDX9
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_Renderpass_Allocate  = &dx9_RenderPass_Allocate;
    dispatch->impl_Renderpass_Begin     = &dx9_RenderPass_Begin;
    dispatch->impl_Renderpass_End       = &dx9_RenderPass_End;
}

}





//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_Begin( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__BEGIN );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_End( Handle cmdBuf, Handle syncObject )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__END, syncObject );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_SetPipelineState( Handle cmdBuf, Handle ps, uint32 vdecl )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_PIPELINE_STATE, ps, vdecl );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_SetCullMode( Handle cmdBuf, CullMode mode )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_CULL_MODE, mode );
}


//------------------------------------------------------------------------------

void
dx9_CommandBuffer_SetScissorRect( Handle cmdBuf, ScissorRect rect )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_SCISSOR_RECT, rect.x, rect.y, rect.width, rect.height );
}


//------------------------------------------------------------------------------

void
dx9_CommandBuffer_SetViewport( Handle cmdBuf, Viewport vp )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_VIEWPORT, vp.x, vp.y, vp.width, vp.height );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_SetVertexData( Handle cmdBuf, Handle vb, uint32 streamIndex )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_VERTEX_DATA, vb, streamIndex );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_SetVertexConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
//    L_ASSERT(buffer);
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);
    
    if( buffer != InvalidIndex )
        CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_VERTEX_PROG_CONST_BUFFER, bufIndex, (uint64)(buffer), (uint64)(ConstBufferDX9::InstData(buffer)) );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_SetVertexTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_TEXTURE, D3DDMAPSAMPLER+1+unitIndex, (uint64)(tex) );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_SetIndices( Handle cmdBuf, Handle ib )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_INDICES, ib );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_SetQueryIndex( Handle cmdBuf, uint32 objectIndex )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_QUERY_INDEX, objectIndex );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_SetQueryBuffer( Handle cmdBuf, Handle queryBuf )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_QUERY_BUFFER, queryBuf );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_SetFragmentConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
//    L_ASSERT(buffer);
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);
    
    if( buffer != InvalidIndex )
        CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_FRAGMENT_PROG_CONST_BUFFER, bufIndex, (uint64)(buffer), (uint64)(ConstBufferDX9::InstData(buffer)) );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_SetFragmentTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_TEXTURE, unitIndex, (uint64)(tex) );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_SetDepthStencilState( Handle cmdBuf, Handle depthStencilState )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_DEPTHSTENCIL_STATE, depthStencilState );
}


//------------------------------------------------------------------------------

static void 
dx9_CommandBuffer_SetSamplerState( Handle cmdBuf, const Handle samplerState )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_SAMPLER_STATE, samplerState );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_DrawPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count )
{
    D3DPRIMITIVETYPE    type9   = D3DPT_TRIANGLELIST;

    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            type9 = D3DPT_TRIANGLELIST;
            break;

        case PRIMITIVE_TRIANGLESTRIP :
            type9 = D3DPT_TRIANGLESTRIP;
            break;
        
        case PRIMITIVE_LINELIST :
            type9 = D3DPT_LINELIST;
            break;
    }

    CommandBufferPool::Get(cmdBuf)->Command( DX9__DRAW_PRIMITIVE, type9, count );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_DrawIndexedPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count, uint32 vertexCount, uint32 firstVertex, uint32 startIndex )
{
    unsigned            v_cnt   = 0;
    D3DPRIMITIVETYPE    type9   = D3DPT_TRIANGLELIST;

    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            type9 = D3DPT_TRIANGLELIST;
            break;

        case PRIMITIVE_TRIANGLESTRIP :
            type9 = D3DPT_TRIANGLESTRIP;
            break;

        case PRIMITIVE_LINELIST :
            type9 = D3DPT_LINELIST;
            break;
    }

    CommandBufferPool::Get(cmdBuf)->Command( DX9__DRAW_INDEXED_PRIMITIVE, type9, count, vertexCount, firstVertex, startIndex );
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_SetMarker( Handle cmdBuf, const char* text )
{
    CommandBufferDX9_t* cb = CommandBufferPool::Get(cmdBuf);

    if( !cb->text )
    {
        cb->text = new RingBuffer();
        cb->text->Initialize( 64*1024 );
    }
    
    int     len = strlen( text );
    char*   txt = (char*)cb->text->Alloc( len/sizeof(float)+1 );

    memcpy( txt, text, len );
    txt[len] = '\0';

    cb->Command( DX9__DEBUG_MARKER, (uint64)(txt) );
}


//------------------------------------------------------------------------------

static Handle
dx9_SyncObject_Create()
{
    Handle              handle = SyncObjectPool::Alloc();
    SyncObjectDX9_t*    sync   = SyncObjectPool::Get( handle );

    sync->is_signaled = false;
    sync->is_used = false;

    return handle;
}


//------------------------------------------------------------------------------

static void
dx9_SyncObject_Delete( Handle obj )
{
    SyncObjectPool::Free( obj );
}


//------------------------------------------------------------------------------

static bool
dx9_SyncObject_IsSignaled( Handle obj )
{
    bool                signaled = false;
    SyncObjectDX9_t*    sync     = SyncObjectPool::Get( obj );
    
    if( sync )
        signaled = sync->is_signaled;

    return signaled;
}




CommandBufferDX9_t::CommandBufferDX9_t()
  : isFirstInPass(true),
    isLastInPass(true),
    text(nullptr),
    sync(InvalidHandle)
{
}


//------------------------------------------------------------------------------


CommandBufferDX9_t::~CommandBufferDX9_t()
{
}


//------------------------------------------------------------------------------

void
CommandBufferDX9_t::Begin()
{
    _cmd.clear();
}


//------------------------------------------------------------------------------

void        
CommandBufferDX9_t::End()
{
    _cmd.push_back( EndCmd );
}


//------------------------------------------------------------------------------

void
CommandBufferDX9_t::Command( uint64 cmd )
{
    _cmd.push_back( cmd );
}


//------------------------------------------------------------------------------

void
CommandBufferDX9_t::Command( uint64 cmd, uint64 arg1 )
{
    _cmd.resize( _cmd.size()+1+1 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+1);

    b[0] = cmd; 
    b[1] = arg1; 
}


//------------------------------------------------------------------------------

void
CommandBufferDX9_t::Command( uint64 cmd, uint64 arg1, uint64 arg2 )
{
    _cmd.resize( _cmd.size()+1+2 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+2);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
}


//------------------------------------------------------------------------------

void
CommandBufferDX9_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3 )
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
CommandBufferDX9_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4 )
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
CommandBufferDX9_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5 )
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
CommandBufferDX9_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5, uint64 arg6 )
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
CommandBufferDX9_t::Execute()
{
SCOPED_FUNCTION_TIMING();
    Handle          cur_pipelinestate   = InvalidHandle;
    uint32          cur_stride          = 0;
    Handle          cur_query_buf       = InvalidHandle;
    uint32          cur_query_i         = InvalidIndex;
    D3DVIEWPORT9    def_viewport;

    _D3D9_Device->GetViewport( &def_viewport );

    sync = InvalidHandle;

    for( std::vector<uint64>::const_iterator c=_cmd.begin(),c_end=_cmd.end(); c!=c_end; ++c )
    {
        const uint64                        cmd = *c;
        std::vector<uint64>::const_iterator arg = c+1;

        if( cmd == EndCmd )
            break;

        switch( cmd )
        {
            case DX9__BEGIN :
            {
                if( isFirstInPass )
                {
                    if( passCfg.colorBuffer[0].texture != rhi::InvalidHandle )
                    {
                        DVASSERT(!_D3D9_BackBuf);
                        _D3D9_Device->GetRenderTarget( 0, &_D3D9_BackBuf );
                        TextureDX9::SetAsRenderTarget( passCfg.colorBuffer[0].texture );                        
                    }

                    if(     passCfg.depthStencilBuffer.texture != rhi::InvalidHandle 
                        &&  passCfg.depthStencilBuffer.texture != DefaultDepthBuffer
                      )
                    {
                        DVASSERT(!_D3D9_DepthBuf);
                        _D3D9_Device->GetDepthStencilSurface( &_D3D9_DepthBuf );

                        if( passCfg.depthStencilBuffer.texture != rhi::InvalidHandle )
                            TextureDX9::SetAsDepthStencil( passCfg.depthStencilBuffer.texture );
                    }
                    
                    // update default viewport
                    {
                        IDirect3DSurface9*  rt = NULL;

                        _D3D9_Device->GetRenderTarget( 0, &rt );
                        if( rt )
                        {
                            D3DSURFACE_DESC desc;
                            
                            if( SUCCEEDED(rt->GetDesc( &desc )) )
                            {
                                def_viewport.X      = 0;
                                def_viewport.Y      = 0;
                                def_viewport.Width  = desc.Width;
                                def_viewport.Height = desc.Height;
                                def_viewport.MinZ   = 0.0f;
                                def_viewport.MaxZ   = 1.0f;
                            }

                            rt->Release();
                        }
                    }


                    bool    clear_color = passCfg.colorBuffer[0].loadAction == LOADACTION_CLEAR;
                    bool    clear_depth = passCfg.depthStencilBuffer.loadAction == LOADACTION_CLEAR;

                    DX9_CALL(_D3D9_Device->BeginScene(),"BeginScene");
                    
                    if( clear_color  ||  clear_depth )
                    {
                        DWORD   flags = 0;
                        int     r     = int(passCfg.colorBuffer[0].clearColor[0] * 255.0f);
                        int     g     = int(passCfg.colorBuffer[0].clearColor[1] * 255.0f);
                        int     b     = int(passCfg.colorBuffer[0].clearColor[2] * 255.0f);
                        int     a     = int(passCfg.colorBuffer[0].clearColor[3] * 255.0f);
                        
                        if( clear_color ) flags |= D3DCLEAR_TARGET;
                        if( clear_depth ) flags |= D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL;

                        DX9_CALL(_D3D9_Device->Clear( 0,NULL, flags, D3DCOLOR_RGBA(r,g,b,a), passCfg.depthStencilBuffer.clearDepth, 0 ),"Clear");
                    }
                }

            }   break;

            case DX9__END :
            {
                sync = Handle(arg[0]);

                if( isLastInPass )
                {
                    DX9_CALL(_D3D9_Device->EndScene(),"EndScene");
                    if( _D3D9_BackBuf )
                    {
                        DX9_CALL(_D3D9_Device->SetRenderTarget( 0, _D3D9_BackBuf ),"SetRenderTarget");
                        _D3D9_BackBuf->Release();
                        _D3D9_BackBuf = nullptr;
                    }
                    if( _D3D9_DepthBuf )
                    {
                        DX9_CALL(_D3D9_Device->SetDepthStencilSurface( _D3D9_DepthBuf ),"SetDepthStencilSurface");
                        _D3D9_DepthBuf->Release();
                        _D3D9_DepthBuf = nullptr;
                    }
                }

                c += 1;
            }   break;
            
            case DX9__SET_VERTEX_DATA :
            {
                DVASSERT(cur_pipelinestate != InvalidHandle);
                unsigned    stride = (cur_stride) 
                                     ? cur_stride
                                     : PipelineStateDX9::VertexLayoutStride( cur_pipelinestate );

                VertexBufferDX9::SetToRHI( (Handle)(arg[0]), 0, 0, stride );
                c += 2;
            }   break;
            
            case DX9__SET_INDICES :
            {
                IndexBufferDX9::SetToRHI( (Handle)(arg[0]) );
                c += 1;
            }   break;

            case DX9__SET_QUERY_BUFFER :
            {
                DVASSERT(cur_query_buf == InvalidHandle);
                cur_query_buf = (Handle)(arg[0]);
                c += 1;
            }   break;

            case DX9__SET_QUERY_INDEX :
            {
                cur_query_i = uint32(arg[0]);
                c += 1;
            }   break;

            case DX9__SET_PIPELINE_STATE :
            {
                uint32              vd_uid = (uint32)(arg[1]);
                const VertexLayout* vdecl  = (vd_uid == VertexLayout::InvalidUID)  
                                             ? nullptr  
                                             : VertexLayout::Get( vd_uid );
                cur_pipelinestate = (Handle)(arg[0]);
                cur_stride        = (vdecl)  ? vdecl->Stride()  : 0;
                
                PipelineStateDX9::SetToRHI( cur_pipelinestate, vd_uid );

                StatSet::IncStat( stat_SET_PS, 1 );
                c += 2;
            }   break;
            
            case DX9__SET_CULL_MODE :
            {
                DWORD   mode = D3DCULL_CCW;

                switch( CullMode(arg[0]) )
                {
                    case CULL_NONE  : mode = D3DCULL_NONE; break;
                    case CULL_CW    : mode = D3DCULL_CW; break;
                    case CULL_CCW   : mode = D3DCULL_CCW; break;
                }

                _D3D9_Device->SetRenderState( D3DRS_CULLMODE, mode ); 
                c += 1;
            }   break;

            case DX9__SET_SCISSOR_RECT :
            {
                int x = int(arg[0]);
                int y = int(arg[1]);
                int w = int(arg[2]);
                int h = int(arg[3]);

                if( !(x==0  &&  y==0  &&  w==0  &&  h==0) )
                {
                    RECT    rect = { x, y, x+w-1, y+h-1 };

                    _D3D9_Device->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
                    _D3D9_Device->SetScissorRect( &rect );
                }
                else
                {
                    _D3D9_Device->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
                }

                c += 4;
            }   break;
            
            case DX9__SET_VIEWPORT :
            {
                int x = int(arg[0]);
                int y = int(arg[1]);
                int w = int(arg[2]);
                int h = int(arg[3]);

                if( !(x==0  &&  y==0  &&  w==0  &&  h==0) )
                {
                    D3DVIEWPORT9    vp;

                    vp.X      = x;
                    vp.Y      = y;
                    vp.Width  = w;
                    vp.Height = h;
                    vp.MinZ   = 0.0f;
                    vp.MaxZ   = 1.0f;
                    
                    _D3D9_Device->SetViewport( &vp );
                }
                else
                {
                    _D3D9_Device->SetViewport( &def_viewport );
                }

                c += 4;
            }   break;

            case DX9__SET_DEPTHSTENCIL_STATE :
            {
                DepthStencilStateDX9::SetToRHI( (Handle)(arg[0]) );
                c += 1;
            }   break;

            case DX9__SET_SAMPLER_STATE :
            {
                SamplerStateDX9::SetToRHI( (Handle)(arg[0]) );
                c += 1;
            }   break;
            
            case DX9__SET_VERTEX_PROG_CONST_BUFFER :
            {
                ConstBufferDX9::SetToRHI( (Handle)(arg[1]), (const void*)(arg[2]) );
                StatSet::IncStat( stat_SET_CB, 1 );
                c += 3;
            }   break;

            case DX9__SET_FRAGMENT_PROG_CONST_BUFFER :
            {
                ConstBufferDX9::SetToRHI( (Handle)(arg[1]), (const void*)(arg[2]) );
                StatSet::IncStat( stat_SET_CB, 1 );
                c += 3;
            }   break;

            case DX9__SET_TEXTURE :
            {
                TextureDX9::SetToRHI( (Handle)(arg[1]), (unsigned)(arg[0]) );
                StatSet::IncStat( stat_SET_TEX, 1 );
                c += 2;
            }   break;
            
            case DX9__DRAW_PRIMITIVE :
            {
                if( cur_query_i != InvalidIndex )
                    QueryBufferDX9::BeginQuery( cur_query_buf, cur_query_i );

                DX9_CALL(_D3D9_Device->DrawPrimitive( (D3DPRIMITIVETYPE)(arg[0]), /*base_vertex*/0, UINT(arg[1]) ),"DrawPrimitive");
                StatSet::IncStat( stat_DP, 1 );
                
                if( cur_query_i != InvalidIndex )
                    QueryBufferDX9::EndQuery( cur_query_buf, cur_query_i );

                c += 2;    
            }   break;
            
            case DX9__DRAW_INDEXED_PRIMITIVE :
            {
                D3DPRIMITIVETYPE    type        = (D3DPRIMITIVETYPE)(arg[0]);
                uint32              primCount   = uint32(arg[1]);
                uint32              vertexCount = uint32(arg[2]);
                uint32              firstVertex = uint32(arg[3]);
                uint32              startIndex  = uint32(arg[4]);

                if( cur_query_i != InvalidIndex )
                    QueryBufferDX9::BeginQuery( cur_query_buf, cur_query_i );
                
                DX9_CALL(_D3D9_Device->DrawIndexedPrimitive( type, firstVertex, 0, vertexCount, startIndex, primCount ),"DrawIndexedPrimitive");
                
                if( cur_query_i != InvalidIndex )
                    QueryBufferDX9::EndQuery( cur_query_buf, cur_query_i );
                
                StatSet::IncStat( stat_DIP, 1 );
                c += 5;
            }   break;

            case DX9__DEBUG_MARKER :
            {
                wchar_t txt[128];

                ::MultiByteToWideChar( CP_ACP, 0, (const char*)(arg[0]), -1, txt, countof(txt));
                ::D3DPERF_SetMarker( D3DCOLOR_ARGB(0xFF,0x40,0x40,0x80), txt );
            }   break;

            default:
                DVASSERT("unknown DX9 render-command");
        }
    }

    _cmd.clear();
}


//------------------------------------------------------------------------------

static void
dx9_Present(Handle sync)
{
    if( _DX9_RenderThreadFrameCount )
    {
Trace("rhi-dx9.present\n");
        _FrameSync.Lock(); 
        {   
            if( _Frame.size() )
            {
                _Frame.back().readyToExecute = true;
                _Frame.back().sync = sync;
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
        while( frame_cnt >= _DX9_RenderThreadFrameCount );
    }
    else
    {    
        if( _Frame.size() )
        {
            _Frame.back().readyToExecute = true;
            _Frame.back().sync = sync;
            _FrameStarted = false;
        }

        _ExecuteQueuedCommands(); 
    }

    ConstBufferDX9::InvalidateAllConstBufferInstances();
}


//------------------------------------------------------------------------------

static void
_RejectAllFrames()
{
    for( std::vector<FrameDX9>::iterator f=_Frame.begin(),f_end=_Frame.end(); f!=f_end; ++f )
    {
        if (f->sync != InvalidHandle)
        {
            SyncObjectDX9_t*    s = SyncObjectPool::Get(f->sync);
            s->is_signaled = true;
            s->is_used = true;
        }
        for( std::vector<Handle>::iterator p=f->pass.begin(),p_end=f->pass.end(); p!=p_end; ++p )
        {
            RenderPassDX9_t*    pp = RenderPassPool::Get( *p );
            
            for( std::vector<Handle>::iterator c=pp->cmdBuf.begin(),c_end=pp->cmdBuf.end(); c!=c_end; ++c )
            {
                CommandBufferDX9_t* cc = CommandBufferPool::Get( *c );
                if (cc->sync != InvalidHandle)
                {
                    SyncObjectDX9_t*    s = SyncObjectPool::Get(cc->sync);
                    s->is_signaled = true;
                    s->is_used = true;
                }                
                cc->_cmd.clear();
                CommandBufferPool::Free( *c ); 
            }

            RenderPassPool::Free( *p );
        }
    }

    _Frame.clear();
}


//------------------------------------------------------------------------------

static void
_ExecuteQueuedCommands()
{
Trace("rhi-dx9.exec-queued-cmd\n");

    std::vector<RenderPassDX9_t*>   pass;
    std::vector<Handle>             pass_h;
    unsigned                        frame_n   = 0;
    bool                            do_render = true;

    if( _ResetPending  ||  NeedRestoreResources() )
        _RejectAllFrames();

    _FrameSync.Lock();
    if( _Frame.size() )
    {
        for( std::vector<Handle>::iterator p=_Frame.begin()->pass.begin(),p_end=_Frame.begin()->pass.end(); p!=p_end; ++p )
        {
            RenderPassDX9_t*    pp      = RenderPassPool::Get( *p );
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
        do_render = false;
    }
    
    if( _Frame.size()  &&  _Frame.begin()->sync != InvalidHandle )
    {
        SyncObjectDX9_t*  sync = SyncObjectPool::Get(_Frame.begin()->sync);

        sync->frame       = frame_n;
        sync->is_signaled = false;
        sync->is_used     = true;
    }
    _FrameSync.Unlock();

    if( do_render )
    {
Trace("\n\n-------------------------------\nexecuting frame %u\n",frame_n);
        for( std::vector<RenderPassDX9_t*>::iterator p=pass.begin(),p_end=pass.end(); p!=p_end; ++p )
        {
            RenderPassDX9_t*    pp = *p;

            for( unsigned b=0; b!=pp->cmdBuf.size(); ++b )
            {
                Handle              cb_h = pp->cmdBuf[b];
                CommandBufferDX9_t* cb   = CommandBufferPool::Get( cb_h );

                cb->Execute();

                if( cb->sync != InvalidHandle )
                {
                    SyncObjectDX9_t*    sync = SyncObjectPool::Get( cb->sync );

                    sync->frame       = frame_n;
                    sync->is_signaled = false;
                    sync->is_used     = true;
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
    }
    

    // do flip, reset/restore device if necessary

    HRESULT hr;

    if( _ResetPending )
    {
        hr = _D3D9_Device->TestCooperativeLevel();

        if( hr == D3DERR_DEVICENOTRESET )
        {
            D3DPRESENT_PARAMETERS   param = _DX9_PresentParam;
            
            param.BackBufferFormat = (_DX9_PresentParam.Windowed)  ? D3DFMT_UNKNOWN  : D3DFMT_A8B8G8R8;
            
            TextureDX9::ReleaseAll();
            VertexBufferDX9::ReleaseAll();
            IndexBufferDX9::ReleaseAll();

            hr = _D3D9_Device->Reset( &param );

            Logger::Info( "trying device reset\n");
            if( SUCCEEDED(hr) )
            {                
                Logger::Info( "device reset\n");

                TextureDX9::ReCreateAll();
                VertexBufferDX9::ReCreateAll();
                IndexBufferDX9::ReCreateAll();

                VertexBufferDX9::PatchCommands( _DX9_PendingImmediateCmd, _DX9_PendingImmediateCmdCount );
                IndexBufferDX9::PatchCommands( _DX9_PendingImmediateCmd, _DX9_PendingImmediateCmdCount );

                _ResetPending = false;
            }
            else
            {
                Logger::Info("device reset failed (%08X) : %s",hr,D3D9ErrorText(hr));
            }
        }
        else
        {
            ::Sleep( 100 );
        }
    }
    else
    {
        hr = _D3D9_Device->Present( NULL, NULL, NULL, NULL );

        if( FAILED(hr) )
            Logger::Error( "present() failed:\n%s\n", D3D9ErrorText(hr) );

        if( hr == D3DERR_DEVICELOST )
        {            
            _ResetPending = true;
            _RejectAllFrames();
        }
    }    


    // update sync-objects

    for( SyncObjectPool::Iterator s=SyncObjectPool::Begin(),s_end=SyncObjectPool::End(); s!=s_end; ++s )
    {
        if (s->is_used && (frame_n - s->frame >= 2))
            s->is_signaled = true;
    }
}


//------------------------------------------------------------------------------

static void
_ExecDX9( DX9Command* command, uint32 cmdCount )
{
#if 1
    #define CHECK_HR(hr) \
    if( FAILED(hr) ) \
        Logger::Error( "%s", D3D9ErrorText(hr) );
#else
    CHECK_HR(hr)
#endif

    for( DX9Command* cmd=command,*cmdEnd=command+cmdCount; cmd!=cmdEnd; ++cmd )
    {
        const uint64*   arg = cmd->arg;

Trace("exec %i\n",int(cmd->func));
        switch( cmd->func )
        {
            case DX9Command::NOP :
                break;

            case DX9Command::CREATE_VERTEX_BUFFER :
            {
                cmd->retval = _D3D9_Device->CreateVertexBuffer( UINT(arg[0]), DWORD(arg[1]), DWORD(arg[2]), D3DPOOL(arg[3]), (IDirect3DVertexBuffer9**)(arg[4]), (HANDLE*)(arg[5]) );
                CHECK_HR(cmd->retval);
            }   break;
            
            case DX9Command::LOCK_VERTEX_BUFFER :
            {
                cmd->retval = ((IDirect3DVertexBuffer9*)(arg[0]))->Lock( UINT(arg[1]), UINT(arg[2]), (VOID**)(arg[3]), DWORD(arg[4]) );
                CHECK_HR(cmd->retval);
            }   break;
            
            case DX9Command::UNLOCK_VERTEX_BUFFER :
            {
                cmd->retval = ((IDirect3DVertexBuffer9*)(arg[0]))->Unlock();
                CHECK_HR(cmd->retval);
            }   break;
            
            case DX9Command::CREATE_INDEX_BUFFER :
            {
                cmd->retval = _D3D9_Device->CreateIndexBuffer( UINT(arg[0]), DWORD(arg[1]), D3DFORMAT(arg[2]), D3DPOOL(arg[3]), (IDirect3DIndexBuffer9**)(arg[4]), (HANDLE*)(arg[5]) );
                CHECK_HR(cmd->retval);
            }   break;
            
            case DX9Command::LOCK_INDEX_BUFFER :
            {
                cmd->retval = ((IDirect3DIndexBuffer9*)(arg[0]))->Lock( UINT(arg[1]), UINT(arg[2]), (VOID**)(arg[3]), DWORD(arg[4]) );
                CHECK_HR(cmd->retval);
            }   break;
            
            case DX9Command::UNLOCK_INDEX_BUFFER :
            {
                cmd->retval = ((IDirect3DIndexBuffer9*)(arg[0]))->Unlock();
                CHECK_HR(cmd->retval);
            }   break;

            case DX9Command::CREATE_TEXTURE :
            {
                cmd->retval = _D3D9_Device->CreateTexture( UINT(arg[0]), UINT(arg[1]), UINT(arg[2]), DWORD(arg[3]), D3DFORMAT(arg[4]), D3DPOOL(arg[5]), (IDirect3DTexture9**)(arg[6]), (HANDLE*)(arg[7]) );
                CHECK_HR(cmd->retval);
            }   break;
            
            case DX9Command::CREATE_CUBE_TEXTURE :
            {
                cmd->retval = _D3D9_Device->CreateCubeTexture( UINT(arg[0]), UINT(arg[1]), DWORD(arg[2]), D3DFORMAT(arg[3]), D3DPOOL(arg[4]), (IDirect3DCubeTexture9**)(arg[5]), (HANDLE*)(arg[6]) );
                CHECK_HR(cmd->retval);
            }   break;

            case DX9Command::SET_TEXTURE_AUTOGEN_FILTER_TYPE :
            {
                cmd->retval = ((IDirect3DTexture9*)(arg[0]))->SetAutoGenFilterType( D3DTEXTUREFILTERTYPE(arg[1]) );
                CHECK_HR(cmd->retval);
            }   break;

            case DX9Command::GET_TEXTURE_SURFACE_LEVEl :
            {
                cmd->retval = ((IDirect3DTexture9*)(arg[0]))->GetSurfaceLevel( UINT(arg[1]), (IDirect3DSurface9**)(arg[2]) );
                CHECK_HR(cmd->retval);
            }   break;

            case DX9Command::LOCK_TEXTURE_RECT :
            {
                cmd->retval = ((IDirect3DTexture9*)(arg[0]))->LockRect( UINT(arg[1]), (D3DLOCKED_RECT*)(arg[2]), (const RECT*)(arg[3]), DWORD(arg[4]) );
                CHECK_HR(cmd->retval);
            }   break;
            
            case DX9Command::UNLOCK_TEXTURE_RECT :
            {
                cmd->retval = ((IDirect3DTexture9*)(arg[0]))->UnlockRect( UINT(arg[1]) );
                CHECK_HR(cmd->retval);
            }   break;

            case DX9Command::LOCK_CUBETEXTURE_RECT :
            {
                cmd->retval = ((IDirect3DCubeTexture9*)(arg[0]))->LockRect( D3DCUBEMAP_FACES(arg[1]), UINT(arg[2]), (D3DLOCKED_RECT*)(arg[3]), (const RECT*)(arg[4]), DWORD(arg[5]) );
                CHECK_HR(cmd->retval);
            }   break;
            
            case DX9Command::UNLOCK_CUBETEXTURE_RECT :
            {
                cmd->retval = ((IDirect3DCubeTexture9*)(arg[0]))->UnlockRect( D3DCUBEMAP_FACES(arg[1]), UINT(arg[2]) );
                CHECK_HR(cmd->retval);
            }   break;

            case DX9Command::GET_RENDERTARGET_DATA :
            {
                cmd->retval = _D3D9_Device->GetRenderTargetData( (IDirect3DSurface9*)arg[0], (IDirect3DSurface9*)arg[1] );
                CHECK_HR(cmd->retval);
            }   break;

            case DX9Command::CREATE_VERTEX_SHADER :
            {
                cmd->retval = _D3D9_Device->CreateVertexShader( (const DWORD*)(arg[0]), (IDirect3DVertexShader9**)(arg[1]) );
                CHECK_HR(cmd->retval);
            }   break;

            case DX9Command::CREATE_VERTEX_DECLARATION :
            {
                cmd->retval = _D3D9_Device->CreateVertexDeclaration( (const D3DVERTEXELEMENT9*)(arg[0]), (IDirect3DVertexDeclaration9**)(arg[1]) );
                CHECK_HR(cmd->retval);
            }   break;

            case DX9Command::CREATE_PIXEL_SHADER :
            {
                cmd->retval = _D3D9_Device->CreatePixelShader( (const DWORD*)(arg[0]), (IDirect3DPixelShader9**)(arg[1]) );
                CHECK_HR(cmd->retval);
            }   break;

            case DX9Command::GET_QUERY_DATA :
            {
                cmd->retval = ((IDirect3DQuery9*)(arg[0]))->GetData( (void*)(arg[1]), DWORD(arg[2]), DWORD(arg[3]) );
                CHECK_HR(cmd->retval);
            }   break;

            case DX9Command::QUERY_INTERFACE :
            {
                cmd->retval = ((IUnknown*)(arg[0]))->QueryInterface( *((const GUID*)(arg[1])), (void**)(arg[2]) );
            }   break;
            
            case DX9Command::RELEASE :
            {
                ((IUnknown*)arg[0])->Release();
            }   break;

            default:
                DVASSERT(!"unknown DX-cmd");
        }
    }

    #undef CHECK_HR
}


//------------------------------------------------------------------------------

void
ExecDX9( DX9Command* command, uint32 cmdCount, bool force_immediate )
{
    if( force_immediate  ||  !_DX9_RenderThreadFrameCount )
    {
        _ExecDX9( command, cmdCount );
    }
    else
    {
        bool    scheduled = false;
        bool    executed  = false;
    
        // CRAP: busy-wait
        do
        {
            _DX9_PendingImmediateCmdSync.Lock();
            if( !_DX9_PendingImmediateCmd )
            {
                _DX9_PendingImmediateCmd      = command;
                _DX9_PendingImmediateCmdCount = cmdCount;
                scheduled                     = true;
            }
            _DX9_PendingImmediateCmdSync.Unlock();
        } while( !scheduled );

        // CRAP: busy-wait
        do
        {
            _DX9_PendingImmediateCmdSync.Lock();
            if( !_DX9_PendingImmediateCmd )
            {
                executed = true;
            }
            _DX9_PendingImmediateCmdSync.Unlock();
        } while( !executed );
    }
}


//------------------------------------------------------------------------------

static void
_RenderFuncDX9( DAVA::BaseObject* obj, void*, void* )
{
    _InitDX9();

    _DX9_RenderThreadStartedSync.Post();
    Trace( "RHI render-thread started\n" );

    while( true )
    {
        bool    do_wait = true;
        bool    do_exit = false;
        
        // CRAP: busy-wait
        do
        {
            _DX9_RenderThreadExitSync.Lock();
            do_exit = _DX9_RenderThreadExitPending;
            _DX9_RenderThreadExitSync.Unlock();
            
            if( do_exit )
                break;

            
            _DX9_PendingImmediateCmdSync.Lock();
            if( _DX9_PendingImmediateCmd )
            {
Trace("exec imm cmd (%u)\n",_DX9_PendingImmediateCmdCount);
                _ExecDX9( _DX9_PendingImmediateCmd, _DX9_PendingImmediateCmdCount );
                _DX9_PendingImmediateCmd      = nullptr;
                _DX9_PendingImmediateCmdCount = 0;
Trace("exec-imm-cmd done\n");
            }
            _DX9_PendingImmediateCmdSync.Unlock();


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

void
InitializeRenderThreadDX9( uint32 frameCount )
{
    _DX9_RenderThreadFrameCount = frameCount;

    if( _DX9_RenderThreadFrameCount )
    {
        _DX9_RenderThread = DAVA::Thread::Create( DAVA::Message(&_RenderFuncDX9) );
        _DX9_RenderThread->SetName( "RHI.dx9-render" );
        _DX9_RenderThread->Start();    
        _DX9_RenderThreadStartedSync.Wait();
    }
    else
    {
        _InitDX9();
    }
}


//------------------------------------------------------------------------------

void
UninitializeRenderThreadDX9()
{
    if( _DX9_RenderThreadFrameCount )
    {
        _DX9_RenderThreadExitSync.Lock();
        _DX9_RenderThreadExitPending = true;
        _DX9_RenderThreadExitSync.Unlock();

        _DX9_RenderThread->Join();
    }
}


namespace CommandBufferDX9
{
void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_CommandBuffer_Begin                  = &dx9_CommandBuffer_Begin;
    dispatch->impl_CommandBuffer_End                    = &dx9_CommandBuffer_End;
    dispatch->impl_CommandBuffer_SetPipelineState       = &dx9_CommandBuffer_SetPipelineState;
    dispatch->impl_CommandBuffer_SetCullMode            = &dx9_CommandBuffer_SetCullMode;
    dispatch->impl_CommandBuffer_SetScissorRect         = &dx9_CommandBuffer_SetScissorRect;
    dispatch->impl_CommandBuffer_SetViewport            = &dx9_CommandBuffer_SetViewport;
    dispatch->impl_CommandBuffer_SetVertexData          = &dx9_CommandBuffer_SetVertexData;
    dispatch->impl_CommandBuffer_SetVertexConstBuffer   = &dx9_CommandBuffer_SetVertexConstBuffer;
    dispatch->impl_CommandBuffer_SetVertexTexture       = &dx9_CommandBuffer_SetVertexTexture;
    dispatch->impl_CommandBuffer_SetIndices             = &dx9_CommandBuffer_SetIndices;    
    dispatch->impl_CommandBuffer_SetQueryBuffer         = &dx9_CommandBuffer_SetQueryBuffer;
    dispatch->impl_CommandBuffer_SetQueryIndex          = &dx9_CommandBuffer_SetQueryIndex;
    dispatch->impl_CommandBuffer_SetFragmentConstBuffer = &dx9_CommandBuffer_SetFragmentConstBuffer;
    dispatch->impl_CommandBuffer_SetFragmentTexture     = &dx9_CommandBuffer_SetFragmentTexture;
    dispatch->impl_CommandBuffer_SetDepthStencilState   = &dx9_CommandBuffer_SetDepthStencilState;
    dispatch->impl_CommandBuffer_SetSamplerState        = &dx9_CommandBuffer_SetSamplerState;
    dispatch->impl_CommandBuffer_DrawPrimitive          = &dx9_CommandBuffer_DrawPrimitive;
    dispatch->impl_CommandBuffer_DrawIndexedPrimitive   = &dx9_CommandBuffer_DrawIndexedPrimitive;
    dispatch->impl_CommandBuffer_SetMarker              = &dx9_CommandBuffer_SetMarker;

    dispatch->impl_SyncObject_Create                    = &dx9_SyncObject_Create;
    dispatch->impl_SyncObject_Delete                    = &dx9_SyncObject_Delete;
    dispatch->impl_SyncObject_IsSignaled                = &dx9_SyncObject_IsSignaled;
    
    dispatch->impl_Present                              = &dx9_Present;
}
}



//==============================================================================
} // namespace rhi

