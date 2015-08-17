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

    #include "rhi_DX11.h"
    #include "../Common/rhi_Impl.h"
    
    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    #include "Core/Core.h"
    using DAVA::Logger;

    #include "_dx11.h"
    #include "../rhi_Type.h"
    #include "../Common/dbg_StatSet.h"

    #include <vector>

#if defined(__DAVAENGINE_WIN_UAP__)
	#include "uap_dx11.h"
#elif defined(__DAVAENGINE_WIN32__)
	#include "win_dx11.h"
#endif
	

namespace rhi
{
//==============================================================================

static Dispatch     DispatchDX11 = {0};



//------------------------------------------------------------------------------

static Api
dx11_HostApi()
{
    return RHI_DX11;
}


//------------------------------------------------------------------------------

static bool
dx11_TextureFormatSupported( TextureFormat format )
{
    bool    supported = false;
    
    switch( format )
    {
        case TEXTURE_FORMAT_R8G8B8A8 :
        case TEXTURE_FORMAT_R5G5B5A1 :
        case TEXTURE_FORMAT_R5G6B5 :
        case TEXTURE_FORMAT_R4G4B4A4 :
        case TEXTURE_FORMAT_R8 :
        case TEXTURE_FORMAT_R16 :
        case TEXTURE_FORMAT_DXT1 :
        case TEXTURE_FORMAT_DXT3 :
        case TEXTURE_FORMAT_DXT5 :
            supported = true;
            break;
    }

    return supported;
}


//------------------------------------------------------------------------------

static bool
_IsValidIntelCard( unsigned vendor_id, unsigned device_id )
{
    return ( ( vendor_id == 0x8086 ) &&  // Intel Architecture

             // These guys are prehistoric :)
             
             ( device_id == 0x2572 ) ||  // 865G
             ( device_id == 0x3582 ) ||  // 855GM
             ( device_id == 0x2562 ) ||  // 845G
             ( device_id == 0x3577 ) ||  // 830M

             // These are from 2005 and later
             
             ( device_id == 0x2A02 ) ||  // GM965 Device 0 
             ( device_id == 0x2A03 ) ||  // GM965 Device 1 
             ( device_id == 0x29A2 ) ||  // G965 Device 0 
             ( device_id == 0x29A3 ) ||  // G965 Device 1 
             ( device_id == 0x27A2 ) ||  // 945GM Device 0 
             ( device_id == 0x27A6 ) ||  // 945GM Device 1 
             ( device_id == 0x2772 ) ||  // 945G Device 0 
             ( device_id == 0x2776 ) ||  // 945G Device 1 
             ( device_id == 0x2592 ) ||  // 915GM Device 0 
             ( device_id == 0x2792 ) ||  // 915GM Device 1 
             ( device_id == 0x2582 ) ||  // 915G Device 0 
             ( device_id == 0x2782 )     // 915G Device 1 
           );
}


//------------------------------------------------------------------------------

static void
dx11_Uninitialize()
{
    UninitializeRenderThreadDX11();
}


//------------------------------------------------------------------------------

static void
dx11_Reset( const ResetParam& param )
{
}


//------------------------------------------------------------------------------

void
_InitDX11()
{
    HRESULT                 hr;

#if defined(__DAVAENGINE_WIN_UAP__)
	hr = init_device_and_swapchain_uap(_DX11_InitParam.window, _DX11_InitParam.width, _DX11_InitParam.height);
#else
	hr = init_device_and_swapchain_win32(_DX11_InitParam.window, _DX11_InitParam.width, _DX11_InitParam.height);
#endif
}


//------------------------------------------------------------------------------

void
dx11_Initialize( const InitParam& param )
{
    _DX11_InitParam = param;
    InitializeRenderThreadDX11();

    VertexBufferDX11::SetupDispatch( &DispatchDX11 );
    IndexBufferDX11::SetupDispatch( &DispatchDX11 );
    QueryBufferDX11::SetupDispatch( &DispatchDX11 );
    TextureDX11::SetupDispatch( &DispatchDX11 );
    PipelineStateDX11::SetupDispatch( &DispatchDX11 );
    ConstBufferDX11::SetupDispatch( &DispatchDX11 );
    DepthStencilStateDX11::SetupDispatch( &DispatchDX11 );
    SamplerStateDX11::SetupDispatch( &DispatchDX11 );
    RenderPassDX11::SetupDispatch( &DispatchDX11 );
    CommandBufferDX11::SetupDispatch( &DispatchDX11 );

    DispatchDX11.impl_Uninitialize           = &dx11_Uninitialize;
    DispatchDX11.impl_Reset                  = &dx11_Reset;
    DispatchDX11.impl_HostApi                = &dx11_HostApi;
    DispatchDX11.impl_TextureFormatSupported = &dx11_TextureFormatSupported;

    SetDispatchTable( DispatchDX11 );


    ConstBufferDX11::InitializeRingBuffer( 4*1024*1024 ); // CRAP: hardcoded const ring-buf size


    stat_DIP        = StatSet::AddStat( "rhi'dip", "dip" );
    stat_DP         = StatSet::AddStat( "rhi'dp", "dp" );
    stat_SET_PS     = StatSet::AddStat( "rhi'set-ps", "set-ps" );
    stat_SET_TEX    = StatSet::AddStat( "rhi'set-tex", "set-tex" );
    stat_SET_CB     = StatSet::AddStat( "rhi'set-cb", "set-cb" );
}


//==============================================================================
} // namespace rhi

