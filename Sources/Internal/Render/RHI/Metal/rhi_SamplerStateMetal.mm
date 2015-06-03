
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
SamplerStateMetal_t
{
    uint32              count;
    id<MTLSamplerState> uid[MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT];
};

typedef Pool<SamplerStateMetal_t,RESOURCE_SAMPLER_STATE>    SamplerStateMetalPool;
RHI_IMPL_POOL(SamplerStateMetal_t,RESOURCE_SAMPLER_STATE);


static MTLSamplerAddressMode
_AddrMode( TextureAddrMode mode )
{
    MTLSamplerAddressMode   m = MTLSamplerAddressModeRepeat;
    
    switch( mode )
    {
        case TEXADDR_WRAP   : m = MTLSamplerAddressModeRepeat; break;
        case TEXADDR_CLAMP  : m = MTLSamplerAddressModeClampToEdge; break;
        case TEXADDR_MIRROR : m = MTLSamplerAddressModeMirrorRepeat; break;
    }
    
    return m;
}

static MTLSamplerMinMagFilter
_TextureFilter( TextureFilter filter )
{
    MTLSamplerMinMagFilter  f = MTLSamplerMinMagFilterNearest;
    
    switch( filter )
    {
        case TEXFILTER_NEAREST  : f = MTLSamplerMinMagFilterNearest; break;
        case TEXFILTER_LINEAR   : f = MTLSamplerMinMagFilterLinear; break;
    }
    
    return f;
}

static MTLSamplerMipFilter
_TextureMipFilter( TextureMipFilter filter )
{
    MTLSamplerMipFilter f = MTLSamplerMipFilterNearest;
    
    switch( filter )
    {
        case TEXMIPFILTER_NONE      : f = MTLSamplerMipFilterNotMipmapped ; break;
        case TEXMIPFILTER_NEAREST   : f = MTLSamplerMipFilterNearest; break;
        case TEXMIPFILTER_LINEAR    : f = MTLSamplerMipFilterLinear; break;
    }
    
    return f;
}
    
    
//==============================================================================

static Handle
metal_SamplerState_Create( const SamplerState::Descriptor& desc )
{
    Handle                  handle  = SamplerStateMetalPool::Alloc();
    SamplerStateMetal_t*    state   = SamplerStateMetalPool::Get( handle );
    
    state->count = desc.fragmentSamplerCount;
    for( unsigned s=0; s!=desc.fragmentSamplerCount; ++s )
    {
        MTLSamplerDescriptor*   s_desc = [MTLSamplerDescriptor new];
        
        s_desc.sAddressMode          = _AddrMode( TextureAddrMode(desc.fragmentSampler[s].addrU) );
        s_desc.tAddressMode          = _AddrMode( TextureAddrMode(desc.fragmentSampler[s].addrV) );
        s_desc.rAddressMode          = _AddrMode( TextureAddrMode(desc.fragmentSampler[s].addrW) );
        s_desc.minFilter             = _TextureFilter( TextureFilter(desc.fragmentSampler[s].minFilter) );
        s_desc.magFilter             = _TextureFilter( TextureFilter(desc.fragmentSampler[s].magFilter) );
        s_desc.mipFilter             = _TextureMipFilter( TextureMipFilter(desc.fragmentSampler[s].mipFilter) );
        s_desc.lodMinClamp           = 0.0f;
        s_desc.lodMaxClamp           = FLT_MAX;
        s_desc.maxAnisotropy         = 1;
        s_desc.normalizedCoordinates = YES;
        
        state->uid[s] = [_Metal_Device newSamplerStateWithDescriptor:s_desc];
    }
    
    return handle;
}


static void
metal_SamplerState_Delete( Handle state )
{
    SamplerStateMetalPool::Free( state );
}




namespace SamplerStateMetal
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_SamplerState_Create = &metal_SamplerState_Create;
    dispatch->impl_SamplerState_Delete = &metal_SamplerState_Delete;
}

void
SetToRHI( Handle hstate, id<MTLRenderCommandEncoder> ce )
{
    SamplerStateMetal_t* state  = SamplerStateMetalPool::Get( hstate );

    for( unsigned s=0; s!=state->count; ++s )
        [ce setFragmentSamplerState:state->uid[s] atIndex:s];
}

}



//==============================================================================
} // namespace rhi

