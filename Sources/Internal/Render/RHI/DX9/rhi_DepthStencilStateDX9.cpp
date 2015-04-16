
    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_DX9.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_dx9.h"


namespace rhi
{
//==============================================================================

struct
DepthStencilStateDX9_t
{
    uint32  depthTestEnabled:1;
    uint32  depthWriteEnabled:1;
    DWORD   depthFunc;

    DWORD   stencilFunc;
    DWORD   stencilReadMask;
    DWORD   stencilWriteMask;
    DWORD   stencilRefValue;
    DWORD   stencilFailOperation;
    DWORD   depthFailOperation;
    DWORD   depthStencilPassOperation;
};

typedef Pool<DepthStencilStateDX9_t,RESOURCE_DEPTHSTENCIL_STATE>    DepthStencilStateDX9Pool;
RHI_IMPL_POOL(DepthStencilStateDX9_t,RESOURCE_DEPTHSTENCIL_STATE);


//------------------------------------------------------------------------------

static DWORD
_CmpFunc( CmpFunc func )
{
    DWORD   f = D3DCMP_ALWAYS;

    switch( func )
    {
        case CMP_NEVER          : f = D3DCMP_NEVER; break;
        case CMP_LESS           : f = D3DCMP_LESS; break;
        case CMP_EQUAL          : f = D3DCMP_EQUAL; break;
        case CMP_LESSEQUAL      : f = D3DCMP_LESSEQUAL; break;
        case CMP_GREATER        : f = D3DCMP_GREATER; break;
        case CMP_NOTEQUAL       : f = D3DCMP_NOTEQUAL; break;
        case CML_GREATEREQUAL   : f = D3DCMP_GREATEREQUAL; break;
        case CMP_ALWAYS         : f = D3DCMP_ALWAYS; break;
    }

    return f;
}


//------------------------------------------------------------------------------

static DWORD
_StencilOp( StencilOperation op )
{
    DWORD   s = D3DSTENCILOP_KEEP;

    switch( op )
    {
        case STENCILOP_KEEP             : s = D3DSTENCILOP_KEEP; break;
        case STENCILOP_ZERO             : s = D3DSTENCILOP_ZERO; break;
        case STENCILOP_REPLACE          : s = D3DSTENCILOP_REPLACE; break;
        case STENCILOP_INVERT           : s = D3DSTENCILOP_INVERT; break;
        case STENCILOP_INCREMENT_CLAMP  : s = D3DSTENCILOP_INCRSAT; break;
        case STENCILOP_DECREMENT_CLAMP  : s = D3DSTENCILOP_DECRSAT; break;
        case STENCILOP_INCREMENT_WRAP   : s = D3DSTENCILOP_INCR; break;
        case STENCILOP_DECREMENT_WRAP   : s = D3DSTENCILOP_DECR; break;
    }

    return s;
}


//==============================================================================

static Handle
dx9_DepthStencilState_Create( const DepthStencilState::Descriptor& desc )
{
    Handle                  handle = DepthStencilStateDX9Pool::Alloc();
    DepthStencilStateDX9_t* state  = DepthStencilStateDX9Pool::Get( handle );
    
    state->depthTestEnabled          = desc.depthTestEnabled;
    state->depthWriteEnabled         = desc.depthWriteEnabled;
    state->depthFunc                 = _CmpFunc( CmpFunc(desc.depthFunc) );

    state->stencilFunc               = _CmpFunc( CmpFunc(desc.stencilFunc) );
    state->stencilReadMask           = desc.stencilReadMask;
    state->stencilWriteMask          = desc.stencilWriteMask;
    state->stencilRefValue           = desc.stencilRefValue;
    state->stencilFailOperation      = _StencilOp( StencilOperation(desc.stencilFailOperation) );
    state->depthFailOperation        = _StencilOp( StencilOperation(desc.stencilFailOperation) );
    state->depthStencilPassOperation = _StencilOp( StencilOperation(desc.depthStencilPassOperation) );

    return handle;
}


//------------------------------------------------------------------------------

static void
dx9_DepthStencilState_Delete( Handle state )
{
    DepthStencilStateDX9Pool::Free( state );
}


//==============================================================================

namespace DepthStencilStateDX9
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_DepthStencilState_Create = &dx9_DepthStencilState_Create;
    dispatch->impl_DepthStencilState_Delete = &dx9_DepthStencilState_Delete;
}

void
SetToRHI( Handle hstate )
{
    DepthStencilStateDX9_t* state  = DepthStencilStateDX9Pool::Get( hstate );
    
    _D3D9_Device->SetRenderState( D3DRS_ZENABLE, state->depthTestEnabled );
    _D3D9_Device->SetRenderState( D3DRS_ZWRITEENABLE, state->depthWriteEnabled );
    _D3D9_Device->SetRenderState( D3DRS_ZFUNC, state->depthFunc );

    _D3D9_Device->SetRenderState( D3DRS_STENCILFUNC, state->stencilFunc );
    _D3D9_Device->SetRenderState( D3DRS_STENCILREF, state->stencilRefValue );
    _D3D9_Device->SetRenderState( D3DRS_STENCILMASK, state->stencilReadMask );
    _D3D9_Device->SetRenderState( D3DRS_STENCILWRITEMASK, state->stencilWriteMask );
    _D3D9_Device->SetRenderState( D3DRS_STENCILZFAIL, state->depthFailOperation );
    _D3D9_Device->SetRenderState( D3DRS_STENCILFAIL, state->stencilFailOperation );
    _D3D9_Device->SetRenderState( D3DRS_STENCILPASS, state->depthStencilPassOperation );
}

}



//==============================================================================
} // namespace rhi

