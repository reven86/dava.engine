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

#include "Renderer.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/RHI/rhi_ShaderCache.h"
#include "Render/RHI/Common/dbg_StatSet.h"
#include "Render/RHI/Common/rhi_Private.h"
#include "Render/ShaderCache.h"
#include "Render/Material/FXCache.h"
#include "Render/DynamicBufferAllocator.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Render/RenderCallbacks.h"

namespace DAVA
{
namespace Renderer
{
namespace //for private variables
{
bool ininialized = false;
rhi::Api api;
int32 desiredFPS = 60;

RenderOptions renderOptions;
DynamicBindings dynamicBindings;
RuntimeTextures runtimeTextures;
RenderStats stats;

int32 framebufferWidth;
int32 framebufferHeight;

ScreenShotCallbackDelegate* screenshotCallback = nullptr;
}

static Mutex renderCmdExecSync;

void Initialize(rhi::Api _api, rhi::InitParam& params)
{
    DVASSERT(!ininialized);

    api = _api;

    framebufferWidth = static_cast<int32>(params.width * params.scaleX);
    framebufferHeight = static_cast<int32>(params.height * params.scaleY);

    if (nullptr == params.FrameCommandExecutionSync)
    {
        params.FrameCommandExecutionSync = &renderCmdExecSync;
    }

    rhi::Initialize(api, params);
    rhi::ShaderCache::Initialize();
    ShaderDescriptorCache::Initialize();
    FXCache::Initialize();
    PixelFormatDescriptor::SetHardwareSupportedFormats();
    GPUFamilyDescriptor::SetupGPUParameters();

    ininialized = true;
}

void Uninitialize()
{
    DVASSERT(ininialized);

    FXCache::Uninitialize();
    ShaderDescriptorCache::Uninitialize();
    rhi::ShaderCache::Unitialize();
    rhi::Uninitialize();
    ininialized = false;
}

bool IsInitialized()
{
    return ininialized;
}

void Reset(const rhi::ResetParam& params)
{
    framebufferWidth = static_cast<int32>(params.width * params.scaleX);
    framebufferHeight = static_cast<int32>(params.height * params.scaleY);

    rhi::Reset(params);
}

bool IsDeviceLost()
{
    DVASSERT(ininialized);
    return false;
}

rhi::Api GetAPI()
{
    DVASSERT(ininialized);
    return api;
}

int32 GetDesiredFPS()
{
    return desiredFPS;
}

void SetDesiredFPS(int32 fps)
{
    desiredFPS = fps;
}

RenderOptions* GetOptions()
{
    DVASSERT(ininialized);
    return &renderOptions;
}

DynamicBindings& GetDynamicBindings()
{
    return dynamicBindings;
}

RuntimeTextures& GetRuntimeTextures()
{
    return runtimeTextures;
}

RenderStats& GetRenderStats()
{
    return stats;
}

int32 GetFramebufferWidth()
{
    return framebufferWidth;
}

int32 GetFramebufferHeight()
{
    return framebufferHeight;
}

void RequestGLScreenShot(ScreenShotCallbackDelegate* _screenShotCallback)
{
    screenshotCallback = _screenShotCallback;
    //RHI_COMPLETE
}

void BeginFrame()
{
    StatSet::ResetAll();

    RenderCallbacks::ProcessFrame();
    DynamicBufferAllocator::BeginFrame();
}

void EndFrame()
{
    DynamicBufferAllocator::EndFrame();
    rhi::Present();

    stats.drawIndexedPrimitive = StatSet::StatValue(rhi::stat_DIP);
    stats.drawPrimitive = StatSet::StatValue(rhi::stat_DP);

    stats.pipelineStateSet = StatSet::StatValue(rhi::stat_SET_PS);
    stats.samplerStateSet = StatSet::StatValue(rhi::stat_SET_SS);

    stats.constBufferSet = StatSet::StatValue(rhi::stat_SET_CB);
    stats.textureSet = StatSet::StatValue(rhi::stat_SET_TEX);

    stats.vertexBufferSet = StatSet::StatValue(rhi::stat_SET_VB);
    stats.indexBufferSet = StatSet::StatValue(rhi::stat_SET_IB);

    stats.primitiveTriangleListCount = StatSet::StatValue(rhi::stat_DTL);
    stats.primitiveTriangleStripCount = StatSet::StatValue(rhi::stat_DTS);
    stats.primitiveLineListCount = StatSet::StatValue(rhi::stat_DLL);
}
}

void RenderStats::Reset()
{
    drawIndexedPrimitive = 0U;
    drawPrimitive = 0U;

    pipelineStateSet = 0U;
    samplerStateSet = 0U;

    constBufferSet = 0U;
    textureSet = 0U;

    vertexBufferSet = 0U;
    indexBufferSet = 0U;

    primitiveTriangleListCount = 0U;
    primitiveTriangleStripCount = 0U;
    primitiveLineListCount = 0U;

    dynamicParamBindCount = 0U;
    materialParamBindCount = 0U;

    batches2d = 0U;
    packets2d = 0U;

    visibleRenderObjects = 0U;
}
}
