
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
SamplerStateDX9_t
{
    struct
    {
        DWORD   addrU;
        DWORD   addrV;
        DWORD   addrW;
        DWORD   minFilter;
        DWORD   magFilter;
        DWORD   mipFilter;
    }       sampler[MAX_TEXTURE_SAMPLER_COUNT];
    uint32  count;
};

typedef Pool<SamplerStateDX9_t,RESOURCE_SAMPLER_STATE>  SamplerStateDX9Pool;
RHI_IMPL_POOL(SamplerStateDX9_t,RESOURCE_SAMPLER_STATE);


//------------------------------------------------------------------------------

static DWORD
_AddrMode( TextureAddrMode mode )
{
    DWORD   m = D3DTADDRESS_WRAP;

    switch( mode )
    {
        case TEXADDR_WRAP   : m = D3DTADDRESS_WRAP; break;
        case TEXADDR_CLAMP  : m = D3DTADDRESS_CLAMP; break;
        case TEXADDR_MIRROR : m = D3DTADDRESS_MIRROR; break;
    }

    return m;
}


//------------------------------------------------------------------------------

static DWORD
_TextureFilter( TextureFilter filter )
{
    DWORD   f = 0;

    switch( filter )
    {
        case TEXFILTER_NEAREST  : f = D3DTEXF_POINT; break;
        case TEXFILTER_LINEAR   : f = D3DTEXF_LINEAR; break;
    }

    return f;
}


//------------------------------------------------------------------------------

static DWORD
_TextureMipFilter( TextureMipFilter filter )
{
    DWORD   f = 0;

    switch( filter )
    {
        case TEXMIPFILTER_NONE      : f = D3DTEXF_NONE; break;
        case TEXMIPFILTER_NEAREST   : f = D3DTEXF_POINT; break;
        case TEXMIPFILTER_LINEAR    : f = D3DTEXF_LINEAR; break;
    }

    return f;
}



//==============================================================================

static Handle
dx9_SamplerState_Create( const SamplerState::Descriptor& desc )
{
    Handle              handle = SamplerStateDX9Pool::Alloc();
    SamplerStateDX9_t*  state  = SamplerStateDX9Pool::Get( handle );
    
    state->count = desc.count;
    for( unsigned i=0; i!=desc.count; ++i )    
    {
        state->sampler[i].addrU     = _AddrMode( TextureAddrMode(desc.sampler[i].addrU) );
        state->sampler[i].addrV     = _AddrMode( TextureAddrMode(desc.sampler[i].addrV) );
        state->sampler[i].addrW     = _AddrMode( TextureAddrMode(desc.sampler[i].addrW) );
        state->sampler[i].minFilter = _TextureFilter( TextureFilter(desc.sampler[i].minFilter) );
        state->sampler[i].magFilter = _TextureFilter( TextureFilter(desc.sampler[i].magFilter) );
        state->sampler[i].mipFilter = _TextureMipFilter( TextureMipFilter(desc.sampler[i].mipFilter) );
    }

    return handle;
}


//------------------------------------------------------------------------------

static void
dx9_SamplerState_Delete( Handle state )
{
    SamplerStateDX9Pool::Free( state );
}


//==============================================================================

namespace SamplerStateDX9
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_SamplerState_Create = &dx9_SamplerState_Create;
    dispatch->impl_SamplerState_Delete = &dx9_SamplerState_Delete;
}

void
SetToRHI( Handle hstate )
{
    SamplerStateDX9_t* state = SamplerStateDX9Pool::Get( hstate );
    
    for( unsigned i=0; i!=state->count; ++i )
    {
        _D3D9_Device->SetSamplerState( i, D3DSAMP_ADDRESSU, state->sampler[i].addrU );
        _D3D9_Device->SetSamplerState( i, D3DSAMP_ADDRESSV, state->sampler[i].addrV );
        _D3D9_Device->SetSamplerState( i, D3DSAMP_ADDRESSW, state->sampler[i].addrW );
        _D3D9_Device->SetSamplerState( i, D3DSAMP_MINFILTER, state->sampler[i].minFilter );
        _D3D9_Device->SetSamplerState( i, D3DSAMP_MAGFILTER, state->sampler[i].magFilter );
        _D3D9_Device->SetSamplerState( i, D3DSAMP_MIPFILTER, state->sampler[i].mipFilter );
    }
}

}



//==============================================================================
} // namespace rhi

