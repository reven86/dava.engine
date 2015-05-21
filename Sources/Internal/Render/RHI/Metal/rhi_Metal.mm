
    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "../Common/dbg_StatSet.h"
    #include "rhi_Metal.h"

    #include "Platform/TemplateiOS/EAGLView.h"
    #include <QuartzCore/CAMetalLayer.h>

    #include "_metal.h"


    id<MTLDevice>                   _Metal_Device                   = nil;
    id<MTLCommandQueue>             _Metal_DefCmdQueue              = nil;
    MTLRenderPassDescriptor*        _Metal_DefRenderPassDescriptor  = nil;
    id<MTLTexture>                  _Metal_DefFrameBuf              = nil;
    id<MTLTexture>                  _Metal_DefDepthBuf              = nil;
    id<MTLDepthStencilState>        _Metal_DefDepthState            = nil;


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
    CAMetalLayer* layer = (CAMetalLayer*)(GetAppViewLayer());

    layer.device            = MTLCreateSystemDefaultDevice();
    layer.pixelFormat       = MTLPixelFormatBGRA8Unorm;
    layer.framebufferOnly   = YES;
    layer.drawableSize      = layer.bounds.size;

    _Metal_Device       = layer.device;
    _Metal_DefCmdQueue  = [_Metal_Device newCommandQueue];


    // create frame-buffer

    int     w = layer.bounds.size.width;
    int     h = layer.bounds.size.height;

    MTLTextureDescriptor*   colorDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm width:w height:h mipmapped:NO];
    MTLTextureDescriptor*   depthDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float width:w height:h mipmapped:NO];

    _Metal_DefFrameBuf = [_Metal_Device newTextureWithDescriptor:colorDesc];
    _Metal_DefDepthBuf = [_Metal_Device newTextureWithDescriptor:depthDesc];


    // create default render-pass desc

    MTLRenderPassDescriptor*    desc = [MTLRenderPassDescriptor renderPassDescriptor];
    
    desc.colorAttachments[0].texture        = _Metal_DefFrameBuf;
    desc.colorAttachments[0].loadAction     = MTLLoadActionClear;
    desc.colorAttachments[0].storeAction    = MTLStoreActionStore;
    desc.colorAttachments[0].clearColor     = MTLClearColorMake(0.3,0.3,0.6,1);

    desc.depthAttachment.texture            = _Metal_DefDepthBuf;
    desc.depthAttachment.loadAction         = MTLLoadActionClear;
    desc.depthAttachment.storeAction        = MTLStoreActionStore;
    desc.depthAttachment.clearDepth         = 1.0f;

    _Metal_DefRenderPassDescriptor = desc;


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