#include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "../Common/dbg_StatSet.h"
    #include "../rhi_Public.h"
    #include "rhi_Metal.h"

    #include "_metal.h"

#import <UIKit/UIKit.h>

#if !(TARGET_IPHONE_SIMULATOR == 1)

id<MTLDevice> _Metal_Device = nil;
id<MTLCommandQueue> _Metal_DefCmdQueue = nil;
id<MTLTexture> _Metal_DefFrameBuf = nil;
id<MTLTexture> _Metal_DefDepthBuf = nil;
id<MTLTexture> _Metal_DefStencilBuf = nil;
id<MTLDepthStencilState> _Metal_DefDepthState = nil;
CAMetalLayer* _Metal_Layer = nil;

DAVA::Atomic<bool> _Metal_Suspended(false);

namespace rhi
{
Dispatch DispatchMetal = { 0 };

//------------------------------------------------------------------------------

static Api
metal_HostApi()
{
    return RHI_METAL;
}

//------------------------------------------------------------------------------

static bool
metal_TextureFormatSupported(TextureFormat format, ProgType)
{
    bool supported = false;

    switch (format)
    {
    case TEXTURE_FORMAT_R8G8B8A8:
    case TEXTURE_FORMAT_R5G5B5A1:
    case TEXTURE_FORMAT_R5G6B5:
    case TEXTURE_FORMAT_R4G4B4A4:
    case TEXTURE_FORMAT_R8:

    case TEXTURE_FORMAT_PVRTC_4BPP_RGBA:
    case TEXTURE_FORMAT_PVRTC_2BPP_RGBA:
    /*
    case TEXTURE_FORMAT_PVRTC2_4BPP_RGB:
    case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA:
    case TEXTURE_FORMAT_PVRTC2_2BPP_RGB:
    case TEXTURE_FORMAT_PVRTC2_2BPP_RGBA:
*/
    case TEXTURE_FORMAT_ETC2_R8G8B8:
    case TEXTURE_FORMAT_ETC2_R8G8B8A1:
    case TEXTURE_FORMAT_EAC_R11_UNSIGNED:
    case TEXTURE_FORMAT_EAC_R11_SIGNED:

    case TEXTURE_FORMAT_D24S8:
    case TEXTURE_FORMAT_D16:
        supported = true;
        break;

    default:
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
metal_Reset(const ResetParam& param)
{
}

//------------------------------------------------------------------------------

static bool
metal_NeedRestoreResources()
{
    static bool lastNeedRestore = false;
    bool needRestore = TextureMetal::NeedRestoreCount();

    if (needRestore)
        DAVA::Logger::Debug("NeedRestore %d TEX", TextureMetal::NeedRestoreCount());

    if (lastNeedRestore && !needRestore)
        DAVA::Logger::Debug("all RHI-resources restored");

    lastNeedRestore = needRestore;

    return needRestore;
}

//------------------------------------------------------------------------------

static void
metal_Suspend()
{
    _Metal_Suspended.Set(true);
    DAVA::Logger::Info("mtl.render-suspended");
}

//------------------------------------------------------------------------------

static void
metal_Resume()
{
    //    TextureMetal::MarkAllNeedRestore();
    //TextureMetal::ReCreateAll();
    _Metal_Suspended.Set(false);
    DAVA::Logger::Info("mtl.render-resumed");
}

//------------------------------------------------------------------------------

bool
rhi_MetalIsSupported()
{
    if (!_Metal_Device)
    {
        NSString* currSysVer = [[UIDevice currentDevice] systemVersion];
        if ([currSysVer compare:@"8.0" options:NSNumericSearch] != NSOrderedAscending)
        {
            _Metal_Device = MTLCreateSystemDefaultDevice();
            [_Metal_Device retain];
        }
    }

    return (_Metal_Device) ? true : false;
    //    return [[UIDevice currentDevice].systemVersion floatValue] >= 8.0;
}

//------------------------------------------------------------------------------

void metal_Initialize(const InitParam& param)
{
    _Metal_Layer = (CAMetalLayer*)param.window;
    [_Metal_Layer retain];

    if (!_Metal_Device)
    {
        _Metal_Device = MTLCreateSystemDefaultDevice();
        [_Metal_Device retain];
    }

    _Metal_Layer.device = _Metal_Device;
    _Metal_Layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    _Metal_Layer.framebufferOnly = YES;
    _Metal_Layer.drawableSize = CGSizeMake((CGFloat)param.width, (CGFloat)param.height);

    _Metal_DefCmdQueue = [_Metal_Device newCommandQueue];

    // create frame-buffer

    int w = param.width;
    int h = param.height;

    MTLTextureDescriptor* colorDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm width:w height:h mipmapped:NO];
    MTLTextureDescriptor* depthDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float width:w height:h mipmapped:NO];
    MTLTextureDescriptor* stencilDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatStencil8 width:w height:h mipmapped:NO];

    _Metal_DefDepthBuf = [_Metal_Device newTextureWithDescriptor:depthDesc];
    _Metal_DefStencilBuf = [_Metal_Device newTextureWithDescriptor:stencilDesc];

    // create default depth-state

    MTLDepthStencilDescriptor* depth_desc = [MTLDepthStencilDescriptor new];

    depth_desc.depthCompareFunction = MTLCompareFunctionLessEqual;
    depth_desc.depthWriteEnabled = YES;

    _Metal_DefDepthState = [_Metal_Device newDepthStencilStateWithDescriptor:depth_desc];

    int ringBufferSize = 4 * 1024 * 1024;
    if (param.shaderConstRingBufferSize)
        ringBufferSize = param.shaderConstRingBufferSize;
    ConstBufferMetal::InitializeRingBuffer(ringBufferSize * 2); //TODO: 2 is for release 3.1 only, in 3.2 we will decrease this in game configuration and set corresponding multiplier here (supposed 3)

    stat_DIP = StatSet::AddStat("rhi'dip", "dip");
    stat_DP = StatSet::AddStat("rhi'dp", "dp");
    stat_DTL = StatSet::AddStat("rhi'dtl", "dtl");
    stat_DTS = StatSet::AddStat("rhi'dts", "dts");
    stat_DLL = StatSet::AddStat("rhi'dll", "dll");
    stat_SET_PS = StatSet::AddStat("rhi'set-ps", "set-ps");
    stat_SET_SS = StatSet::AddStat("rhi'set-ss", "set-ss");
    stat_SET_TEX = StatSet::AddStat("rhi'set-tex", "set-tex");
    stat_SET_CB = StatSet::AddStat("rhi'set-cb", "set-cb");
    stat_SET_VB = StatSet::AddStat("rhi'set-vb", "set-vb");
    stat_SET_IB = StatSet::AddStat("rhi'set-ib", "set-ib");

    VertexBufferMetal::SetupDispatch(&DispatchMetal);
    IndexBufferMetal::SetupDispatch(&DispatchMetal);
    QueryBufferMetal::SetupDispatch(&DispatchMetal);
    PerfQuerySetMetal::SetupDispatch(&DispatchMetal);
    TextureMetal::SetupDispatch(&DispatchMetal);
    PipelineStateMetal::SetupDispatch(&DispatchMetal);
    ConstBufferMetal::SetupDispatch(&DispatchMetal);
    DepthStencilStateMetal::SetupDispatch(&DispatchMetal);
    SamplerStateMetal::SetupDispatch(&DispatchMetal);
    RenderPassMetal::SetupDispatch(&DispatchMetal);
    CommandBufferMetal::SetupDispatch(&DispatchMetal);

    DispatchMetal.impl_Reset = &metal_Reset;
    DispatchMetal.impl_Uninitialize = &metal_Uninitialize;
    DispatchMetal.impl_HostApi = &metal_HostApi;
    DispatchMetal.impl_TextureFormatSupported = &metal_TextureFormatSupported;
    DispatchMetal.impl_NeedRestoreResources = &metal_NeedRestoreResources;
    DispatchMetal.impl_NeedRestoreResources = &metal_NeedRestoreResources;
    DispatchMetal.impl_ResumeRendering = &metal_Resume;
    DispatchMetal.impl_SuspendRendering = &metal_Suspend;

    SetDispatchTable(DispatchMetal);

    if (param.maxVertexBufferCount)
        VertexBufferMetal::Init(param.maxVertexBufferCount);
    if (param.maxIndexBufferCount)
        IndexBufferMetal::Init(param.maxIndexBufferCount);
    if (param.maxConstBufferCount)
        ConstBufferMetal::Init(param.maxConstBufferCount);
    if (param.maxTextureCount)
        TextureMetal::Init(param.maxTextureCount);

    MutableDeviceCaps::Get().is32BitIndicesSupported = true;
    MutableDeviceCaps::Get().isFramebufferFetchSupported = true;
    MutableDeviceCaps::Get().isVertexTextureUnitsSupported = true;
    MutableDeviceCaps::Get().isZeroBaseClipRange = true;
    MutableDeviceCaps::Get().isUpperLeftRTOrigin = true;
    MutableDeviceCaps::Get().isCenterPixelMapping = false;
    MutableDeviceCaps::Get().isInstancingSupported = true;
    MutableDeviceCaps::Get().maxAnisotropy = 16;
    MutableDeviceCaps::Get().maxSamples = 4;
}

} // namespace rhi

#endif //#if !(TARGET_IPHONE_SIMULATOR==1)