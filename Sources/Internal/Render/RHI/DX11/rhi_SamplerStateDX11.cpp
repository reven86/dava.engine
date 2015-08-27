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
    #include "rhi_DX11.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    
    #include "_dx11.h"


namespace rhi
{
//==============================================================================

struct
SamplerStateDX11_t
{    
    uint32              fragmentSamplerCount;
    ID3D11SamplerState* fragmentSampler[MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT];
                        
                        SamplerStateDX11_t()
                          : fragmentSamplerCount(0)
                        {}
};

typedef ResourcePool<SamplerStateDX11_t,RESOURCE_SAMPLER_STATE,SamplerState::Descriptor,false>  SamplerStateDX11Pool;
RHI_IMPL_POOL(SamplerStateDX11_t,RESOURCE_SAMPLER_STATE,SamplerState::Descriptor,false);


//------------------------------------------------------------------------------

static D3D11_FILTER
_TextureFilterDX11( TextureFilter min_filter, TextureFilter mag_filter, TextureMipFilter mip_filter )
{
    D3D11_FILTER    f = D3D11_FILTER_MIN_MAG_MIP_POINT;

    switch( mip_filter )
    {
        case TEXMIPFILTER_NONE :
        case TEXMIPFILTER_NEAREST :
        {
            if( min_filter == TEXFILTER_NEAREST  &&  mag_filter == TEXFILTER_NEAREST )
                f = D3D11_FILTER_MIN_MAG_MIP_POINT;
            else if( min_filter == TEXFILTER_NEAREST  &&  mag_filter == TEXFILTER_LINEAR )
                f = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
            else if( min_filter == TEXFILTER_LINEAR  &&  mag_filter == TEXFILTER_NEAREST )            
                f = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            else if( min_filter == TEXFILTER_LINEAR  &&  mag_filter == TEXFILTER_LINEAR )  
                f = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        }   break;

        case TEXMIPFILTER_LINEAR :
        {
            if( min_filter == TEXFILTER_NEAREST  &&  mag_filter == TEXFILTER_NEAREST )
                f = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            else if( min_filter == TEXFILTER_NEAREST  &&  mag_filter == TEXFILTER_LINEAR )
                f = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
            else if( min_filter == TEXFILTER_LINEAR  &&  mag_filter == TEXFILTER_NEAREST )            
                f = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            else if( min_filter == TEXFILTER_LINEAR  &&  mag_filter == TEXFILTER_LINEAR )  
                f = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        }   break;
    }

    return f;
}


//------------------------------------------------------------------------------

D3D11_TEXTURE_ADDRESS_MODE
_TextureAddrModeDX11( TextureAddrMode mode )
{
    D3D11_TEXTURE_ADDRESS_MODE m = D3D11_TEXTURE_ADDRESS_WRAP;

    switch( mode )
    {
        case TEXADDR_WRAP   : m = D3D11_TEXTURE_ADDRESS_WRAP; break;
        case TEXADDR_CLAMP  : m = D3D11_TEXTURE_ADDRESS_CLAMP; break;
        case TEXADDR_MIRROR : m = D3D11_TEXTURE_ADDRESS_MIRROR; break;
    }
    
    return m;
}


//------------------------------------------------------------------------------

static void
dx11_SamplerState_Delete( Handle hstate )
{
    SamplerStateDX11_t* state  = SamplerStateDX11Pool::Get( hstate );

    if( state )
    {
        for( unsigned s=0; s!=state->fragmentSamplerCount; ++s )    
        {
            if( state->fragmentSampler[s] )
            {
                state->fragmentSampler[s]->Release();
            }
        }

        SamplerStateDX11Pool::Free( hstate );
    }    
}


//------------------------------------------------------------------------------

static Handle
dx11_SamplerState_Create( const SamplerState::Descriptor& desc )
{
    Handle              handle  = SamplerStateDX11Pool::Alloc();
    SamplerStateDX11_t* state   = SamplerStateDX11Pool::Get( handle );
    bool                success = true;
    
    memset( state->fragmentSampler, 0, sizeof(state->fragmentSampler) );
    
    state->fragmentSamplerCount = desc.fragmentSamplerCount;
    for( unsigned s=0; s!=desc.fragmentSamplerCount; ++s )    
    {        
        D3D11_SAMPLER_DESC  s_desc;
        HRESULT             hr;
        
        s_desc.Filter        = _TextureFilterDX11( TextureFilter(desc.fragmentSampler[s].minFilter), TextureFilter(desc.fragmentSampler[s].magFilter), TextureMipFilter(desc.fragmentSampler[s].mipFilter) );
        s_desc.AddressU      = _TextureAddrModeDX11( TextureAddrMode(desc.fragmentSampler[s].addrU) );
        s_desc.AddressV      = _TextureAddrModeDX11( TextureAddrMode(desc.fragmentSampler[s].addrV) );
        s_desc.AddressW      = _TextureAddrModeDX11( TextureAddrMode(desc.fragmentSampler[s].addrW) );
        s_desc.MipLODBias    = 0;
        s_desc.MaxAnisotropy = 0;
        s_desc.MinLOD        = -D3D11_FLOAT32_MAX;
        s_desc.MaxLOD        = D3D11_FLOAT32_MAX;
        
        hr = _D3D11_Device->CreateSamplerState( &s_desc, state->fragmentSampler+s );
        
        if( FAILED(hr) )
        {
            state->fragmentSampler[s] = nullptr;
            success = false;
        }
    }

    if( !success )
    {
        dx11_SamplerState_Delete( handle );
        handle = InvalidHandle;
    }

    return handle;
}


//==============================================================================

namespace SamplerStateDX11
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_SamplerState_Create = &dx11_SamplerState_Create;
    dispatch->impl_SamplerState_Delete = &dx11_SamplerState_Delete;
}

void
SetToRHI( Handle hstate, ID3D11DeviceContext* context )
{
    SamplerStateDX11_t* state = SamplerStateDX11Pool::Get( hstate );
    
      context->PSSetSamplers( 0, state->fragmentSamplerCount, state->fragmentSampler );
}

}



//==============================================================================
} // namespace rhi

