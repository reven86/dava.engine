//==============================================================================
//
//  
//
//==============================================================================
//
//  externals:

    #include "rhi_DX9.h"
    #include "../Common/rhi_Impl.h"
    
    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    #include "Core/Core.h"
    using DAVA::Logger;

    #include "_dx9.h"
    #include "../rhi_Type.h"
    #include "../Common/dbg_StatSet.h"

    #include <vector>

namespace rhi
{
//==============================================================================

Dispatch    DispatchDX9 = {0};


//==============================================================================

static Api
dx9_HostApi()
{
    return RHI_DX9;
}


//------------------------------------------------------------------------------

static bool
dx9_TextureFormatSupported( TextureFormat format )
{
    bool    supported = false;
    
    switch( format )
    {
        case TEXTURE_FORMAT_A8R8G8B8 :
        case TEXTURE_FORMAT_A1R5G5B5 :
        case TEXTURE_FORMAT_R5G6B5 :
        case TEXTURE_FORMAT_A4R4G4B4 :
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

static void
dx9_Uninitialize()
{
}


//------------------------------------------------------------------------------

static void
dx9_Reset( const ResetParam& param )
{
}


//------------------------------------------------------------------------------

void
dx9_Initialize( const InitParam& param )
{
    _D3D9_Device = (IDirect3DDevice9 *)param.context;
    _End_Frame = param.endFrameFunc;

    DVASSERT(_D3D9_Device);
    DVASSERT(_End_Frame);

    VertexBufferDX9::SetupDispatch( &DispatchDX9 );
    IndexBufferDX9::SetupDispatch( &DispatchDX9 );
    TextureDX9::SetupDispatch( &DispatchDX9 );
    PipelineStateDX9::SetupDispatch( &DispatchDX9 );
    ConstBufferDX9::SetupDispatch( &DispatchDX9 );
    DepthStencilStateDX9::SetupDispatch( &DispatchDX9 );
    SamplerStateDX9::SetupDispatch( &DispatchDX9 );
    RenderPassDX9::SetupDispatch( &DispatchDX9 );
    CommandBufferDX9::SetupDispatch( &DispatchDX9 );

    DispatchDX9.impl_Uninitialize           = &dx9_Uninitialize;
    DispatchDX9.impl_Reset                  = &dx9_Reset;
    DispatchDX9.impl_HostApi                = &dx9_HostApi;
    DispatchDX9.impl_TextureFormatSupported = &dx9_TextureFormatSupported;

    SetDispatchTable( DispatchDX9 );

    ConstBufferDX9::InitializeRingBuffer( 4*1024*1024 ); // CRAP: hardcoded const ring-buf size

    stat_DIP        = StatSet::AddStat( "rhi'dip", "dip" );
    stat_DP         = StatSet::AddStat( "rhi'dp", "dp" );
    stat_SET_PS     = StatSet::AddStat( "rhi'set-ps", "set-ps" );
    stat_SET_TEX    = StatSet::AddStat( "rhi'set-tex", "set-tex" );
    stat_SET_CB     = StatSet::AddStat( "rhi'set-cb", "set-cb" );
}


//==============================================================================
} // namespace rhi

