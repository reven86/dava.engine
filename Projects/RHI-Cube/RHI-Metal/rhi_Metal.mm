
    #include "../rhi_Base.h"
    #include "../RHI/rhi_Pool.h"
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


//------------------------------------------------------------------------------

Api
HostApi()
{
    return RHI_METAL;
}


//------------------------------------------------------------------------------

void
Initialize()
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

    depth_desc.depthCompareFunction = MTLCompareFunctionLess;
    depth_desc.depthWriteEnabled    = YES;
    
    _Metal_DefDepthState = [_Metal_Device newDepthStencilStateWithDescriptor:depth_desc];


    ConstBufferMetal::InitializeRingBuffer( 4*1024*1024 );
}


//------------------------------------------------------------------------------

void
Uninitialize()
{
    
}


} // namespace rhi