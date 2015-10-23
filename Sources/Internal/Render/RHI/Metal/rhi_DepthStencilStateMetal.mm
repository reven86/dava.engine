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
    uint32                      stencilEnabled:1;
    id<MTLDepthStencilState>    uid;
    uint8                       stencilRefValue;
};

typedef ResourcePool<DepthStencilStateMetal_t,RESOURCE_DEPTHSTENCIL_STATE,DepthStencilState::Descriptor,false>  DepthStencilStateMetalPool;
RHI_IMPL_POOL(DepthStencilStateMetal_t,RESOURCE_DEPTHSTENCIL_STATE,DepthStencilState::Descriptor,false);

    
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
        case CMP_GREATEREQUAL   : f = MTLCompareFunctionGreaterEqual; break;
        case CMP_ALWAYS         : f = MTLCompareFunctionAlways; break;
    }
    
    return f;
}
    

//==============================================================================

static Handle
metal_DepthStencilState_Create( const DepthStencilState::Descriptor& desc )
{
    Handle                      handle      = DepthStencilStateMetalPool::Alloc();
    DepthStencilStateMetal_t*   state       = DepthStencilStateMetalPool::Get( handle );
    MTLDepthStencilDescriptor*  ds_desc     = [MTLDepthStencilDescriptor new];
    
    if( desc.stencilEnabled )
    {
        MTLStencilDescriptor*       front_desc  = [MTLStencilDescriptor new];
        MTLStencilDescriptor*       back_desc   = [MTLStencilDescriptor new];

        front_desc.readMask                     = desc.stencilFront.readMask;
        front_desc.writeMask                    = desc.stencilFront.writeMask;
        front_desc.stencilFailureOperation      = _StencilOp( StencilOperation(desc.stencilFront.failOperation) );
        front_desc.depthFailureOperation        = _StencilOp( StencilOperation(desc.stencilFront.depthFailOperation) );
        front_desc.depthStencilPassOperation    = _StencilOp( StencilOperation(desc.stencilFront.depthStencilPassOperation) );
        front_desc.stencilCompareFunction       = _CmpFunc( CmpFunc(desc.stencilFront.func) );
        
        back_desc.readMask                      = desc.stencilBack.readMask;
        back_desc.writeMask                     = desc.stencilBack.writeMask;
        back_desc.stencilFailureOperation       = _StencilOp( StencilOperation(desc.stencilBack.failOperation) );
        back_desc.depthFailureOperation         = _StencilOp( StencilOperation(desc.stencilBack.depthFailOperation) );
        back_desc.depthStencilPassOperation     = _StencilOp( StencilOperation(desc.stencilBack.depthStencilPassOperation) );
        back_desc.stencilCompareFunction        = _CmpFunc( CmpFunc(desc.stencilBack.func) );
        
        ds_desc.frontFaceStencil = front_desc;
        
        if( desc.stencilTwoSided )
            ds_desc.backFaceStencil = back_desc;
        
        state->stencilRefValue  = desc.stencilFront.refValue;
    }
    
    ds_desc.depthWriteEnabled       = (desc.depthWriteEnabled)  ? YES  : NO;
    ds_desc.depthCompareFunction    = _CmpFunc( CmpFunc(desc.depthFunc) );
    
    state->uid            = [_Metal_Device newDepthStencilStateWithDescriptor:ds_desc];
    state->stencilEnabled = desc.stencilEnabled;
    
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
    
    if( state->stencilEnabled )
        [ce setStencilReferenceValue:state->stencilRefValue];
}

}



//==============================================================================
} // namespace rhi

