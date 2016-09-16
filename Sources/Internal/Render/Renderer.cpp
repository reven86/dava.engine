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
#include "Render/Image/Image.h"
#include "Render/Texture.h"
#include "Platform/DeviceInfo.h"

namespace DAVA
{
namespace Renderer
{
namespace //for private variables
{
bool initialized = false;
rhi::Api api;
int32 desiredFPS = 60;

RenderOptions renderOptions;
DynamicBindings dynamicBindings;
RuntimeTextures runtimeTextures;
RenderStats stats;

rhi::ResetParam resetParams;
}

void Initialize(rhi::Api _api, rhi::InitParam& params)
{
    DVASSERT(!initialized);

    api = _api;

    rhi::Initialize(api, params);
    rhi::ShaderCache::Initialize();
    ShaderDescriptorCache::Initialize();
    FXCache::Initialize();
    PixelFormatDescriptor::SetHardwareSupportedFormats();

    resetParams.width = params.width;
    resetParams.height = params.height;
    resetParams.vsyncEnabled = params.vsyncEnabled;
    resetParams.window = params.window;
    resetParams.fullScreen = params.fullScreen;

    initialized = true;
    //must be called after setting ininialized in true
    Texture::SetDefaultGPU(DeviceInfo::GetGPUFamily());
}

void Uninitialize()
{
    DVASSERT(initialized);

    FXCache::Uninitialize();
    ShaderDescriptorCache::Uninitialize();
    rhi::ShaderCache::Unitialize();
    rhi::Uninitialize();
    initialized = false;
}

bool IsInitialized()
{
    return initialized;
}

void Reset(const rhi::ResetParam& params)
{
    resetParams = params;

    rhi::Reset(params);
}

bool IsDeviceLost()
{
    DVASSERT(initialized);
    return false;
}

rhi::Api GetAPI()
{
    DVASSERT(initialized);
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

void SetVSyncEnabled(bool enable)
{
    if (resetParams.vsyncEnabled != enable)
    {
        resetParams.vsyncEnabled = enable;
        rhi::Reset(resetParams);
    }
}

bool IsVSyncEnabled()
{
    return resetParams.vsyncEnabled;
}

RenderOptions* GetOptions()
{
    DVASSERT(initialized);
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
    return static_cast<int32>(resetParams.width);
}

int32 GetFramebufferHeight()
{
    return static_cast<int32>(resetParams.height);
}

void BeginFrame()
{
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
    occludedRenderObjects = 0U;

    queryResults.clear();
}
}
