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
    #include "../Common/dbg_StatSet.h"
    #include "../rhi_Public.h"
    #include "rhi_Metal.h"

    #include "_metal.h"

id<MTLDevice> _Metal_Device = nil;
id<MTLCommandQueue> _Metal_DefCmdQueue = nil;
MTLRenderPassDescriptor* _Metal_DefRenderPassDescriptor = nil;
id<MTLTexture> _Metal_DefFrameBuf = nil;
id<MTLTexture> _Metal_DefDepthBuf = nil;
id<MTLTexture> _Metal_DefStencilBuf = nil;
id<MTLDepthStencilState> _Metal_DefDepthState = nil;
CAMetalLayer* _Metal_Layer = nil;

namespace rhi
{
Dispatch DispatchMetal = { 0 };

RenderDeviceCaps _metal_DeviceCaps;

//------------------------------------------------------------------------------

static Api
metal_HostApi()
{
    return RHI_METAL;
}

//------------------------------------------------------------------------------

static bool
metal_TextureFormatSupported(TextureFormat format)
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

    case TEXTURE_FORMAT_PVRTC2_4BPP_RGB:
    case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA:
    case TEXTURE_FORMAT_PVRTC2_2BPP_RGB:
    case TEXTURE_FORMAT_PVRTC2_2BPP_RGBA:

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

static const RenderDeviceCaps&
metal_DeviceCaps()
{
    return _metal_DeviceCaps;
}

//------------------------------------------------------------------------------

static bool
metal_NeedRestoreResources()
{
    return false;
}

static void
metal_Suspend()
{
}

static void
metal_Resume()
{
}

//------------------------------------------------------------------------------

void metal_Initialize(const InitParam& param)
{
    _Metal_Layer = (CAMetalLayer*)param.window;

    _Metal_Layer.device = MTLCreateSystemDefaultDevice();
    _Metal_Layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    _Metal_Layer.framebufferOnly = YES;
    _Metal_Layer.drawableSize = CGSizeMake((CGFloat)param.width, (CGFloat)param.height);

    _Metal_Device = _Metal_Layer.device;
    _Metal_DefCmdQueue = [_Metal_Device newCommandQueue];

    // create frame-buffer

    int w = param.width;
    int h = param.height;

    MTLTextureDescriptor* colorDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm width:w height:h mipmapped:NO];
    MTLTextureDescriptor* depthDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float width:w height:h mipmapped:NO];
    MTLTextureDescriptor* stencilDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatStencil8 width:w height:h mipmapped:NO];

    _Metal_DefFrameBuf = [_Metal_Device newTextureWithDescriptor:colorDesc];
    _Metal_DefDepthBuf = [_Metal_Device newTextureWithDescriptor:depthDesc];
    _Metal_DefStencilBuf = [_Metal_Device newTextureWithDescriptor:stencilDesc];

    // create default render-pass desc

    MTLRenderPassDescriptor* desc = [MTLRenderPassDescriptor renderPassDescriptor];

    desc.colorAttachments[0].texture = _Metal_DefFrameBuf;
    desc.colorAttachments[0].loadAction = MTLLoadActionClear;
    desc.colorAttachments[0].storeAction = MTLStoreActionStore;
    desc.colorAttachments[0].clearColor = MTLClearColorMake(0.3, 0.3, 0.6, 1);

    desc.depthAttachment.texture = _Metal_DefDepthBuf;
    desc.depthAttachment.loadAction = MTLLoadActionClear;
    desc.depthAttachment.storeAction = MTLStoreActionStore;
    desc.depthAttachment.clearDepth = 1.0f;

    _Metal_DefRenderPassDescriptor = desc;

    // create default depth-state

    MTLDepthStencilDescriptor* depth_desc = [MTLDepthStencilDescriptor new];

    depth_desc.depthCompareFunction = MTLCompareFunctionLessEqual;
    depth_desc.depthWriteEnabled = YES;

    _Metal_DefDepthState = [_Metal_Device newDepthStencilStateWithDescriptor:depth_desc];

    int ringBufferSize = 4 * 1024 * 1024;
    if (param.shaderConstRingBufferSize)
        ringBufferSize = param.shaderConstRingBufferSize;
    ConstBufferMetal::InitializeRingBuffer(ringBufferSize);

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
    DispatchMetal.impl_DeviceCaps = &metal_DeviceCaps;
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

    _metal_DeviceCaps.is32BitIndicesSupported = true;
    _metal_DeviceCaps.isFramebufferFetchSupported = true;
    _metal_DeviceCaps.isVertexTextureUnitsSupported = true;
    _metal_DeviceCaps.isZeroBaseClipRange = true;
    _metal_DeviceCaps.isUpperLeftRTOrigin = true;
    _metal_DeviceCaps.isCenterPixelMapping = false;
}

} // namespace rhi