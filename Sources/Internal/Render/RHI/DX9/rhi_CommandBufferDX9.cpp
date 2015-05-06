//==============================================================================
//
//  
//
//==============================================================================
//
//  externals:

    #include "../Common/rhi_Pool.h"
    #include "rhi_DX9.h"

    #include "../rhi_Type.h"
    #include "../Common/dbg_StatSet.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "Core/Core.h"
    #include "Debug/Profiler.h"

    #include "_dx9.h"
    #include <vector>



namespace rhi
{
//==============================================================================

static bool _ResetPending   = false;


static std::vector<Handle>  _CmdQueue;


//------------------------------------------------------------------------------

enum
CommandDX9
{
    DX9__BEGIN,
    DX9__END,

    DX9__SET_VERTEX_DATA,
    DX9__SET_INDICES,

    DX9__SET_PIPELINE_STATE,
    DX9__SET_CULL_MODE,
    DX9__SET_VERTEX_PROG_CONST_BUFFER,
    DX9__SET_FRAGMENT_PROG_CONST_BUFFER,
    DX9__SET_TEXTURE,

    DX9__SET_DEPTHSTENCIL_STATE,
    DX9__SET_SAMPLER_STATE,

    DX9__DRAW_PRIMITIVE,
    DX9__DRAW_INDEXED_PRIMITIVE,


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
};


typedef Pool<CommandBufferDX9_t,RESOURCE_COMMAND_BUFFER>    CommandBufferPool;
typedef Pool<RenderPassDX9_t,RESOURCE_RENDER_PASS>          RenderPassPool;

RHI_IMPL_POOL(CommandBufferDX9_t,RESOURCE_COMMAND_BUFFER);
RHI_IMPL_POOL(RenderPassDX9_t,RESOURCE_RENDER_PASS);

    
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
    _CmdQueue.push_back( pass );
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
dx9_CommandBuffer_End( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__END );
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
}


//------------------------------------------------------------------------------

static void
dx9_CommandBuffer_SetIndices( Handle cmdBuf, Handle ib )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_INDICES, ib );
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
    }

    CommandBufferPool::Get(cmdBuf)->Command( DX9__DRAW_INDEXED_PRIMITIVE, type9, count, vertexCount, firstVertex, startIndex );
}



CommandBufferDX9_t::CommandBufferDX9_t()
  : isFirstInPass(true),
    isLastInPass(true)
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
    Handle  cur_pipelinestate   = InvalidHandle;
    uint32  cur_stride          = 0;

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
                        if( clear_depth ) flags |= D3DCLEAR_ZBUFFER;

                        DX9_CALL(_D3D9_Device->Clear( 0,NULL, flags, D3DCOLOR_RGBA(r,g,b,a), passCfg.depthStencilBuffer.clearDepth, 0 ),"Clear");
                    }
                }
            }   break;

            case DX9__END :
            {
                if( isLastInPass )
                {
                    DX9_CALL(_D3D9_Device->EndScene(),"EndScene");
                    if( _D3D9_BackBuf )
                    {
                        DX9_CALL(_D3D9_Device->SetRenderTarget( 0, _D3D9_BackBuf ),"SetRenderTarget");
                        _D3D9_BackBuf->Release();
                        _D3D9_BackBuf = nullptr;
                    }
                }
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
                DX9_CALL(_D3D9_Device->DrawPrimitive( (D3DPRIMITIVETYPE)(arg[0]), /*base_vertex*/0, UINT(arg[1]) ),"DrawPrimitive");
                StatSet::IncStat( stat_DP, 1 );
                c += 2;    
            }   break;
            
            case DX9__DRAW_INDEXED_PRIMITIVE :
            {
                D3DPRIMITIVETYPE    type        = (D3DPRIMITIVETYPE)(arg[0]);
                uint32              primCount   = uint32(arg[1]);
                uint32              vertexCount = uint32(arg[2]);
                uint32              firstVertex = uint32(arg[3]);
                uint32              startIndex  = uint32(arg[4]);

                DX9_CALL(_D3D9_Device->DrawIndexedPrimitive( type, firstVertex, 0, vertexCount, startIndex, primCount ),"DrawIndexedPrimitive");
                StatSet::IncStat( stat_DIP, 1 );
                c += 5;
            }   break;

            default:
                DVASSERT("unknown DX9 render-command");
        }
    }

    _cmd.clear();
}


//------------------------------------------------------------------------------

static void
dx9_Present()
{
    DVASSERT(_D3D9_Device)

    static std::vector<RenderPassDX9_t*>    pass;

    // sort cmd-lists by priority

    pass.clear();
    for( unsigned i=0; i!=_CmdQueue.size(); ++i )
    {
        RenderPassDX9_t*    rp     = RenderPassPool::Get( _CmdQueue[i] );
        bool                do_add = true;
        
        for( std::vector<RenderPassDX9_t*>::iterator p=pass.begin(),p_end=pass.end(); p!=p_end; ++p )
        {
            if( rp->priority > (*p)->priority )
            {
                pass.insert( p, 1, rp );
                do_add = false;
                break;
            }
        }

        if( do_add )
            pass.push_back( rp );
    }

    
    // execute command-lists
    
    for( std::vector<RenderPassDX9_t*>::iterator p=pass.begin(),p_end=pass.end(); p!=p_end; ++p )
    {
        for( unsigned b=0; b!=(*p)->cmdBuf.size(); ++b )
        {
            Handle              cb_h = (*p)->cmdBuf[b];
            CommandBufferDX9_t* cb   = CommandBufferPool::Get( cb_h );

            cb->Execute();
            CommandBufferPool::Free( cb_h );
        }
    }
    
    for( unsigned i=0; i!=_CmdQueue.size(); ++i )
        RenderPassPool::Free( _CmdQueue[i] );
    _CmdQueue.clear();



    ConstBufferDX9::InvalidateAllConstBufferInstances();

    HRESULT hr;

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
    {
        hr = _D3D9_Device->Present( NULL, NULL, NULL, NULL );

        if( FAILED(hr) )
            Logger::Error( "present() failed:\n%s\n", D3D9ErrorText(hr) );

        if( hr == D3DERR_DEVICELOST )
            _ResetPending = true;
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
    dispatch->impl_CommandBuffer_SetVertexData          = &dx9_CommandBuffer_SetVertexData;
    dispatch->impl_CommandBuffer_SetVertexConstBuffer   = &dx9_CommandBuffer_SetVertexConstBuffer;
    dispatch->impl_CommandBuffer_SetVertexTexture       = &dx9_CommandBuffer_SetVertexTexture;
    dispatch->impl_CommandBuffer_SetIndices             = &dx9_CommandBuffer_SetIndices;
    dispatch->impl_CommandBuffer_SetFragmentConstBuffer = &dx9_CommandBuffer_SetFragmentConstBuffer;
    dispatch->impl_CommandBuffer_SetFragmentTexture     = &dx9_CommandBuffer_SetFragmentTexture;
    dispatch->impl_CommandBuffer_SetDepthStencilState   = &dx9_CommandBuffer_SetDepthStencilState;
    dispatch->impl_CommandBuffer_SetSamplerState        = &dx9_CommandBuffer_SetSamplerState;
    dispatch->impl_CommandBuffer_DrawPrimitive          = &dx9_CommandBuffer_DrawPrimitive;
    dispatch->impl_CommandBuffer_DrawIndexedPrimitive   = &dx9_CommandBuffer_DrawIndexedPrimitive;
//    dispatch->impl_CommandBuffer_SetMarker              = &dx9_CommandBuffer_SetMarker;
    
    dispatch->impl_Present                              = &dx9_Present;
}
}



//==============================================================================
} // namespace rhi

