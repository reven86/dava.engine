
    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "../Common/dbg_StatSet.h"
    #include "../rhi_Public.h"
    #include "rhi_Metal.h"

    #include "_metal.h"


    id<MTLDevice>                   _Metal_Device                   = nil;
    id<MTLCommandQueue>             _Metal_DefCmdQueue              = nil;
    id<MTLDepthStencilState>        _Metal_DefDepthState            = nil;
    DVMetalLayer*                   _Metal_Layer                    = nil;


namespace rhi
{
Dispatch        DispatchMetal       = {0};

    
//------------------------------------------------------------------------------

static Api
metal_HostApi()
{
    return RHI_METAL;
}


//------------------------------------------------------------------------------

static bool
metal_TextureFormatSupported( TextureFormat format )
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
            supported = true;
            break;
    }

    return supported;
}
    
    
//------------------------------------------------------------------------------

static void
metal_Uninitialize()
{
    
}


//------------------------------------------------------------------------------

static void
metal_Reset( const ResetParam& param )
{
}


//------------------------------------------------------------------------------

void
metal_Initialize( const InitParam& param )
{
    DVASSERT(param.context);
    
    _Metal_Layer = (DVMetalLayer*)param.context;
    
    _Metal_Device       = _Metal_Layer.device;
    _Metal_DefCmdQueue  = [_Metal_Device newCommandQueue];

    // create default depth-state

    MTLDepthStencilDescriptor*  depth_desc = [MTLDepthStencilDescriptor new];

    depth_desc.depthCompareFunction = MTLCompareFunctionLessEqual;
    depth_desc.depthWriteEnabled    = YES;
    
    _Metal_DefDepthState = [_Metal_Device newDepthStencilStateWithDescriptor:depth_desc];


    ConstBufferMetal::InitializeRingBuffer( 8*1024*1024 );

    stat_DIP        = StatSet::AddStat( "rhi'dip", "dip" );
    stat_DP         = StatSet::AddStat( "rhi'dp", "dp" );
    stat_SET_PS     = StatSet::AddStat( "rhi'set-ps", "set-ps" );
    stat_SET_TEX    = StatSet::AddStat( "rhi'set-tex", "set-tex" );
    stat_SET_CB     = StatSet::AddStat( "rhi'set-cb", "set-cb" );


    VertexBufferMetal::SetupDispatch( &DispatchMetal );
    IndexBufferMetal::SetupDispatch( &DispatchMetal );
    TextureMetal::SetupDispatch( &DispatchMetal );
    PipelineStateMetal::SetupDispatch( &DispatchMetal );
    ConstBufferMetal::SetupDispatch( &DispatchMetal );
    DepthStencilStateMetal::SetupDispatch( &DispatchMetal );
    SamplerStateMetal::SetupDispatch( &DispatchMetal );
    RenderPassMetal::SetupDispatch( &DispatchMetal );
    CommandBufferMetal::SetupDispatch( &DispatchMetal );
    
    DispatchMetal.impl_Reset                    = &metal_Reset;
    DispatchMetal.impl_Uninitialize             = &metal_Uninitialize;
    DispatchMetal.impl_HostApi                  = &metal_HostApi;
    DispatchMetal.impl_TextureFormatSupported   = &metal_TextureFormatSupported;
    
    SetDispatchTable( DispatchMetal );
}


} // namespace rhi