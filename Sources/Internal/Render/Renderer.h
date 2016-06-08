#ifndef __DAVAENGINE_RENDERER_H__
#define __DAVAENGINE_RENDERER_H__

#include "Core/Core.h"
#include "RenderBase.h"
#include "RenderOptions.h"
#include "DynamicBindings.h"
#include "RuntimeTextures.h"
#include "RHI/rhi_Public.h"
#include "RHI/rhi_Type.h"

namespace DAVA
{
struct ScreenShotCallbackDelegate;
struct RenderStats;

namespace Renderer
{
//init
void Initialize(rhi::Api api, rhi::InitParam& params);
void Uninitialize();
bool IsInitialized();

void Reset(const rhi::ResetParam& params);

rhi::Api GetAPI();

bool IsDeviceLost();

void SetDesiredFPS(int32 fps);
int32 GetDesiredFPS();

void SetVSyncEnabled(bool enable);
bool IsVSyncEnabled();

//frame management
void BeginFrame();
void EndFrame();

//misc
int32 GetFramebufferWidth();
int32 GetFramebufferHeight();
void RequestGLScreenShot(ScreenShotCallbackDelegate* screenShotCallback);

//options
RenderOptions* GetOptions();

//dynamic params
DynamicBindings& GetDynamicBindings();

//runtime textures
RuntimeTextures& GetRuntimeTextures();

//render stats
RenderStats& GetRenderStats();
}

class Image;
struct ScreenShotCallbackDelegate
{
    virtual ~ScreenShotCallbackDelegate() = default;

    void operator()(Image* image)
    {
        return OnScreenShot(image);
    }

protected:
    virtual void OnScreenShot(Image* image) = 0;
};

struct RenderStats
{
    void Reset();

    uint32 drawPrimitive = 0U;
    uint32 drawIndexedPrimitive = 0U;

    uint32 pipelineStateSet = 0U;
    uint32 samplerStateSet = 0U;

    uint32 constBufferSet = 0U;
    uint32 textureSet = 0U;

    uint32 vertexBufferSet = 0U;
    uint32 indexBufferSet = 0U;

    uint32 primitiveTriangleListCount = 0U;
    uint32 primitiveTriangleStripCount = 0U;
    uint32 primitiveLineListCount = 0U;

    uint32 dynamicParamBindCount = 0U;
    uint32 materialParamBindCount = 0U;

    uint32 batches2d = 0U;
    uint32 packets2d = 0U;

    uint32 visibleRenderObjects = 0U;
};
}

#endif
