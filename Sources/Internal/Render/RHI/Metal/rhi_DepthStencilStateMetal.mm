
    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_Metal.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_metal.h"


namespace rhi
{
//==============================================================================

struct
DepthStencilStateMetal_t
{
    id<MTLDepthStencilState>    uid;
    uint8                       stencilRefValue;
};

typedef Pool<DepthStencilStateMetal_t,RESOURCE_DEPTHSTENCIL_STATE>    DepthStencilStateMetalPool;
RHI_IMPL_POOL(DepthStencilStateMetal_t,RESOURCE_DEPTHSTENCIL_STATE);

    
static MTLStencilOperation
_StencilOp( StencilOperation op )
{
    MTLStencilOperation   s = MTLStencilOperationKeep;
    
    switch( op )
    {
        case STENCILOP_KEEP             : s = MTLStencilOperationKeep; break;
        case STENCILOP_ZERO             : s = MTLStencilOperationZero; break;
        case STENCILOP_REPLACE          : s = MTLStencilOperationReplace; break;
        case STENCILOP_INVERT           : s = MTLStencilOperationInvert; break;
        case STENCILOP_INCREMENT_CLAMP  : s = MTLStencilOperationIncrementClamp; break;
        case STENCILOP_DECREMENT_CLAMP  : s = MTLStencilOperationDecrementClamp; break;
        case STENCILOP_INCREMENT_WRAP   : s = MTLStencilOperationIncrementWrap; break;
        case STENCILOP_DECREMENT_WRAP   : s = MTLStencilOperationDecrementWrap; break;
    }
    
    return s;
}
 
static MTLCompareFunction
_CmpFunc( CmpFunc func )
{
    MTLCompareFunction  f = MTLCompareFunctionLessEqual;
    
    switch( func )
    {
        case CMP_NEVER          : f = MTLCompareFunctionNever; break;
        case CMP_LESS           : f = MTLCompareFunctionLess; break;
        case CMP_EQUAL          : f = MTLCompareFunctionEqual; break;
        case CMP_LESSEQUAL      : f = MTLCompareFunctionLessEqual; break;
        case CMP_GREATER        : f = MTLCompareFunctionGreater; break;
        case CMP_NOTEQUAL       : f = MTLCompareFunctionNotEqual; break;
        case CML_GREATEREQUAL   : f = MTLCompareFunctionGreaterEqual; break;
        case CMP_ALWAYS         : f = MTLCompareFunctionAlways; break;
    }
    
    return f;
}
    

//==============================================================================

static Handle
metal_DepthStencilState_Create( const DepthStencilState::Descriptor& desc )
{
    Handle                      handle  = DepthStencilStateMetalPool::Alloc();
    DepthStencilStateMetal_t*   state   = DepthStencilStateMetalPool::Get( handle );
    MTLDepthStencilDescriptor*  ds_desc = [MTLDepthStencilDescriptor new];
    MTLStencilDescriptor*       s_desc  = [MTLStencilDescriptor new];

    s_desc.readMask                     = desc.stencilReadMask;
    s_desc.writeMask                    = desc.stencilWriteMask;
    s_desc.stencilFailureOperation      = _StencilOp( StencilOperation(desc.stencilFailOperation) );
    s_desc.depthFailureOperation        = _StencilOp( StencilOperation(desc.depthFailOperation) );
    s_desc.depthStencilPassOperation    = _StencilOp( StencilOperation(desc.depthStencilPassOperation) );
    s_desc.stencilCompareFunction       = _CmpFunc( CmpFunc(desc.stencilFunc) );

    ds_desc.depthWriteEnabled       = (desc.depthWriteEnabled)  ? YES  : NO;
    ds_desc.depthCompareFunction    = _CmpFunc( CmpFunc(desc.depthFunc) );
    ds_desc.frontFaceStencil        = s_desc;
    
    state->uid              = [_Metal_Device newDepthStencilStateWithDescriptor:ds_desc];
    state->stencilRefValue  = desc.stencilRefValue;
    
    return handle;
}


static void
metal_DepthStencilState_Delete( Handle state )
{
    DepthStencilStateMetalPool::Free( state );
}




namespace DepthStencilStateMetal
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_DepthStencilState_Create = &metal_DepthStencilState_Create;
    dispatch->impl_DepthStencilState_Delete = &metal_DepthStencilState_Delete;
}

void
SetToRHI( Handle hstate, id<MTLRenderCommandEncoder> ce )
{
    DepthStencilStateMetal_t* state  = DepthStencilStateMetalPool::Get( hstate );

    [ce setDepthStencilState:state->uid];
    [ce setStencilReferenceValue:state->stencilRefValue];
}

}



//==============================================================================
} // namespace rhi

