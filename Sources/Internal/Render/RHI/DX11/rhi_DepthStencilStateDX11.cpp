
    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_DX11.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_dx11.h"


namespace rhi
{
//==============================================================================

struct
DepthStencilStateDX11_t
{
    ID3D11DepthStencilState*    _ds11;
    UINT                        _stencilRef;

                                DepthStencilStateDX11_t()
                                  : _ds11(nullptr)
                                {}
};

typedef ResourcePool<DepthStencilStateDX11_t,RESOURCE_DEPTHSTENCIL_STATE>   DepthStencilStateDX11Pool;
RHI_IMPL_POOL(DepthStencilStateDX11_t,RESOURCE_DEPTHSTENCIL_STATE);


//------------------------------------------------------------------------------

static D3D11_COMPARISON_FUNC
_CmpFuncDX11( CmpFunc func )
{
    D3D11_COMPARISON_FUNC   f = D3D11_COMPARISON_ALWAYS;

    switch( func )
    {
        case CMP_NEVER          : f = D3D11_COMPARISON_NEVER; break;
        case CMP_LESS           : f = D3D11_COMPARISON_LESS; break;
        case CMP_EQUAL          : f = D3D11_COMPARISON_EQUAL; break;
        case CMP_LESSEQUAL      : f = D3D11_COMPARISON_LESS_EQUAL; break;
        case CMP_GREATER        : f = D3D11_COMPARISON_GREATER; break;
        case CMP_NOTEQUAL       : f = D3D11_COMPARISON_NOT_EQUAL; break;
        case CMP_GREATEREQUAL   : f = D3D11_COMPARISON_GREATER_EQUAL; break;
        case CMP_ALWAYS         : f = D3D11_COMPARISON_ALWAYS; break;
    }

    return f;
}


//------------------------------------------------------------------------------

static D3D11_STENCIL_OP
_StencilOpDX11( StencilOperation op )
{
    D3D11_STENCIL_OP   s = D3D11_STENCIL_OP_KEEP;

    switch( op )
    {
        case STENCILOP_KEEP             : s = D3D11_STENCIL_OP_KEEP; break;
        case STENCILOP_ZERO             : s = D3D11_STENCIL_OP_ZERO; break;
        case STENCILOP_REPLACE          : s = D3D11_STENCIL_OP_REPLACE; break;
        case STENCILOP_INVERT           : s = D3D11_STENCIL_OP_INVERT; break;
        case STENCILOP_INCREMENT_CLAMP  : s = D3D11_STENCIL_OP_INCR_SAT; break;
        case STENCILOP_DECREMENT_CLAMP  : s = D3D11_STENCIL_OP_DECR_SAT; break;
        case STENCILOP_INCREMENT_WRAP   : s = D3D11_STENCIL_OP_INCR; break;
        case STENCILOP_DECREMENT_WRAP   : s = D3D11_STENCIL_OP_DECR; break;
    }

    return s;
}


//==============================================================================

static Handle
dx11_DepthStencilState_Create( const DepthStencilState::Descriptor& desc )
{
    Handle                   handle = DepthStencilStateDX11Pool::Alloc();
    DepthStencilStateDX11_t* state  = DepthStencilStateDX11Pool::Get( handle );
    D3D11_DEPTH_STENCIL_DESC ds_desc;
    DX11Command              cmd    = { DX11Command::CREATE_DEPTHSTENCIL_STATE, { uint64(&ds_desc), uint64(&(state->_ds11)) } };

    ds_desc.DepthEnable                  = desc.depthTestEnabled;
    ds_desc.DepthWriteMask               = (desc.depthWriteEnabled) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    ds_desc.DepthFunc                    = _CmpFuncDX11( CmpFunc(desc.depthFunc) );    
    ds_desc.StencilEnable                = (desc.stencilEnabled) ? TRUE : FALSE;
    ds_desc.StencilReadMask              = desc.stencilFront.readMask;
    ds_desc.StencilWriteMask             = desc.stencilFront.writeMask;
    ds_desc.FrontFace.StencilFailOp      = _StencilOpDX11( StencilOperation(desc.stencilFront.failOperation) );
    ds_desc.FrontFace.StencilDepthFailOp = _StencilOpDX11( StencilOperation(desc.stencilFront.depthFailOperation) );
    ds_desc.FrontFace.StencilPassOp      = _StencilOpDX11( StencilOperation(desc.stencilFront.depthStencilPassOperation) );
    ds_desc.FrontFace.StencilFunc        = _CmpFuncDX11( CmpFunc(desc.stencilFront.func) );
    ds_desc.BackFace.StencilFailOp       = _StencilOpDX11( StencilOperation(desc.stencilBack.failOperation) );
    ds_desc.BackFace.StencilDepthFailOp  = _StencilOpDX11( StencilOperation(desc.stencilBack.depthFailOperation) );
    ds_desc.BackFace.StencilPassOp       = _StencilOpDX11( StencilOperation(desc.stencilBack.depthStencilPassOperation) );
    ds_desc.BackFace.StencilFunc         = _CmpFuncDX11( CmpFunc(desc.stencilBack.func) );


    ExecDX11( &cmd, 1 );

    if( SUCCEEDED(cmd.retval) )
    {
        state->_stencilRef = 0;
    }
    else
    {
        state->_ds11 = nullptr;
        DepthStencilStateDX11Pool::Free( handle );
        handle = InvalidHandle;
    }

    return handle;
}


//------------------------------------------------------------------------------

static void
dx11_DepthStencilState_Delete( Handle hstate )
{
    DepthStencilStateDX11_t* state  = DepthStencilStateDX11Pool::Get( hstate );
    
    if( state )
    {
        DX11Command cmd = { DX11Command::RELEASE, { uint64(&(state->_ds11)) } };
        
        ExecDX11( &cmd, 1 );
        state->_ds11 = nullptr;
    }

    DepthStencilStateDX11Pool::Free( hstate );
}


//==============================================================================

namespace DepthStencilStateDX11
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_DepthStencilState_Create = &dx11_DepthStencilState_Create;
    dispatch->impl_DepthStencilState_Delete = &dx11_DepthStencilState_Delete;
}

void
SetToRHI( Handle hstate )
{
    DepthStencilStateDX11_t* state  = DepthStencilStateDX11Pool::Get( hstate );
    
    _D3D11_ImmediateContext->OMSetDepthStencilState( state->_ds11, state->_stencilRef );
}

}



//==============================================================================
} // namespace rhi

