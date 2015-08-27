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
    sampler_t
    {
        DWORD   addrU;
        DWORD   addrV;
        DWORD   addrW;
        DWORD   minFilter;
        DWORD   magFilter;
        DWORD   mipFilter;
    };
    
    sampler_t   fragmentSampler[MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT];
    uint32      fragmentSamplerCount;

    sampler_t   vertexSampler[MAX_VERTEX_TEXTURE_SAMPLER_COUNT];
    uint32      vertexSamplerCount;
};

typedef ResourcePool<SamplerStateDX9_t,RESOURCE_SAMPLER_STATE,SamplerState::Descriptor,false>   SamplerStateDX9Pool;
RHI_IMPL_POOL(SamplerStateDX9_t,RESOURCE_SAMPLER_STATE,SamplerState::Descriptor,false);


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
    
    state->fragmentSamplerCount = desc.fragmentSamplerCount;
    for( unsigned i=0; i!=desc.fragmentSamplerCount; ++i )
    {
        state->fragmentSampler[i].addrU     = _AddrMode( TextureAddrMode(desc.fragmentSampler[i].addrU) );
        state->fragmentSampler[i].addrV     = _AddrMode( TextureAddrMode(desc.fragmentSampler[i].addrV) );
        state->fragmentSampler[i].addrW     = _AddrMode( TextureAddrMode(desc.fragmentSampler[i].addrW) );
        state->fragmentSampler[i].minFilter = _TextureFilter( TextureFilter(desc.fragmentSampler[i].minFilter) );
        state->fragmentSampler[i].magFilter = _TextureFilter( TextureFilter(desc.fragmentSampler[i].magFilter) );
        state->fragmentSampler[i].mipFilter = _TextureMipFilter( TextureMipFilter(desc.fragmentSampler[i].mipFilter) );
    }

    state->vertexSamplerCount = desc.vertexSamplerCount;
    for( unsigned i=0; i!=desc.vertexSamplerCount; ++i )
    {
        state->vertexSampler[i].addrU     = _AddrMode( TextureAddrMode(desc.vertexSampler[i].addrU) );
        state->vertexSampler[i].addrV     = _AddrMode( TextureAddrMode(desc.vertexSampler[i].addrV) );
        state->vertexSampler[i].addrW     = _AddrMode( TextureAddrMode(desc.vertexSampler[i].addrW) );
        state->vertexSampler[i].minFilter = _TextureFilter( TextureFilter(desc.vertexSampler[i].minFilter) );
        state->vertexSampler[i].magFilter = _TextureFilter( TextureFilter(desc.vertexSampler[i].magFilter) );
        state->vertexSampler[i].mipFilter = _TextureMipFilter( TextureMipFilter(desc.vertexSampler[i].mipFilter) );
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
    
    for( unsigned i=0; i!=state->fragmentSamplerCount; ++i )
    {
        _D3D9_Device->SetSamplerState( i, D3DSAMP_ADDRESSU, state->fragmentSampler[i].addrU );
        _D3D9_Device->SetSamplerState( i, D3DSAMP_ADDRESSV, state->fragmentSampler[i].addrV );
        _D3D9_Device->SetSamplerState( i, D3DSAMP_ADDRESSW, state->fragmentSampler[i].addrW );
        _D3D9_Device->SetSamplerState( i, D3DSAMP_MINFILTER, state->fragmentSampler[i].minFilter );
        _D3D9_Device->SetSamplerState( i, D3DSAMP_MAGFILTER, state->fragmentSampler[i].magFilter );
        _D3D9_Device->SetSamplerState( i, D3DSAMP_MIPFILTER, state->fragmentSampler[i].mipFilter );
    }

    for( unsigned i=0; i!=state->vertexSamplerCount; ++i )
    {
        _D3D9_Device->SetSamplerState( D3DDMAPSAMPLER+1+i, D3DSAMP_ADDRESSU, state->vertexSampler[i].addrU );
        _D3D9_Device->SetSamplerState( D3DDMAPSAMPLER+1+i, D3DSAMP_ADDRESSV, state->vertexSampler[i].addrV );
        _D3D9_Device->SetSamplerState( D3DDMAPSAMPLER+1+i, D3DSAMP_ADDRESSW, state->vertexSampler[i].addrW );
        _D3D9_Device->SetSamplerState( D3DDMAPSAMPLER+1+i, D3DSAMP_MINFILTER, state->vertexSampler[i].minFilter );
        _D3D9_Device->SetSamplerState( D3DDMAPSAMPLER+1+i, D3DSAMP_MAGFILTER, state->vertexSampler[i].magFilter );
        _D3D9_Device->SetSamplerState( D3DDMAPSAMPLER+1+i, D3DSAMP_MIPFILTER, state->vertexSampler[i].mipFilter );
    }
}

}



//==============================================================================
} // namespace rhi

