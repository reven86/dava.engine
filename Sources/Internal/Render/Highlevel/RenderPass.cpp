#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Highlevel/ShadowVolumeRenderLayer.h"
#include "Render/ShaderCache.h"

#include "Debug/Profiler.h"
#include "Concurrency/Thread.h"

#include "Render/Renderer.h"
#include "Render/Texture.h"

#include "Render/Image/ImageSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

namespace DAVA
{
RenderPass::RenderPass(const FastName& _name)
    : passName(_name)
{
    renderLayers.reserve(RenderLayer::RENDER_LAYER_ID_COUNT);

    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.colorBuffer[0].clearColor[0] = 0.0f;
    passConfig.colorBuffer[0].clearColor[1] = 0.0f;
    passConfig.colorBuffer[0].clearColor[2] = 0.0f;
    passConfig.colorBuffer[0].clearColor[3] = 1.0f;
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    passConfig.priority = PRIORITY_MAIN_3D;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;
    passConfig.viewport.width = Renderer::GetFramebufferWidth();
    passConfig.viewport.height = Renderer::GetFramebufferHeight();
}

RenderPass::~RenderPass()
{
    ClearLayersArrays();
    for (RenderLayer* layer : renderLayers)
    {
        SafeDelete(layer);
    }
    SafeRelease(multisampledTexture);
}

void RenderPass::AddRenderLayer(RenderLayer* layer, RenderLayer::eRenderLayerID afterLayer)
{
    if (RenderLayer::RENDER_LAYER_INVALID_ID != afterLayer)
    {
        uint32 size = static_cast<uint32>(renderLayers.size());
        for (uint32 i = 0; i < size; ++i)
        {
            RenderLayer::eRenderLayerID layerID = renderLayers[i]->GetRenderLayerID();
            if (afterLayer == layerID)
            {
                renderLayers.insert(renderLayers.begin() + i + 1, layer);
                layersBatchArrays[layerID].SetSortingFlags(layer->GetSortingFlags());
                return;
            }
        }
        DVASSERT(0 && "RenderPass::AddRenderLayer afterLayer not found");
    }
    else
    {
        renderLayers.push_back(layer);
        layersBatchArrays[layer->GetRenderLayerID()].SetSortingFlags(layer->GetSortingFlags());
    }
}

void RenderPass::RemoveRenderLayer(RenderLayer* layer)
{
    Vector<RenderLayer*>::iterator it = std::find(renderLayers.begin(), renderLayers.end(), layer);
    DVASSERT(it != renderLayers.end());

    renderLayers.erase(it);
}

void RenderPass::SetupCameraParams(Camera* mainCamera, Camera* drawCamera, Vector4* externalClipPlane)
{
    DVASSERT(drawCamera);
    DVASSERT(mainCamera);

    bool needInvertCamera = rhi::NeedInvertProjection(passConfig);
    passConfig.invertCulling = needInvertCamera ? 1 : 0;

    drawCamera->SetupDynamicParameters(needInvertCamera, externalClipPlane);
    if (mainCamera != drawCamera)
        mainCamera->PrepareDynamicParameters(needInvertCamera, externalClipPlane);
}

void RenderPass::Draw(RenderSystem* renderSystem)
{
    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();
    SetupCameraParams(mainCamera, drawCamera);

    PrepareVisibilityArrays(mainCamera, renderSystem);

    if (BeginRenderPass())
    {
        DrawLayers(mainCamera);
        EndRenderPass();
    }
}

void RenderPass::PrepareVisibilityArrays(Camera* camera, RenderSystem* renderSystem)
{
    uint32 currVisibilityCriteria = RenderObject::CLIPPING_VISIBILITY_CRITERIA;
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::ENABLE_STATIC_OCCLUSION))
        currVisibilityCriteria &= ~RenderObject::VISIBLE_STATIC_OCCLUSION;

    visibilityArray.clear();
    renderSystem->GetRenderHierarchy()->Clip(camera, visibilityArray, currVisibilityCriteria);

    ClearLayersArrays();
    PrepareLayersArrays(visibilityArray, camera);
}

void RenderPass::PrepareLayersArrays(const Vector<RenderObject*> objectsArray, Camera* camera)
{
    size_t size = objectsArray.size();
    for (size_t ro = 0; ro < size; ++ro)
    {
        RenderObject* renderObject = objectsArray[ro];
        if (renderObject->GetFlags() & RenderObject::CUSTOM_PREPARE_TO_RENDER)
        {
            renderObject->PrepareToRender(camera);
        }

        uint32 batchCount = renderObject->GetActiveRenderBatchCount();
        for (uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
        {
            RenderBatch* batch = renderObject->GetActiveRenderBatch(batchIndex);

            NMaterial* material = batch->GetMaterial();
            DVASSERT(material);
            if (material->PreBuildMaterial(passName))
            {
                layersBatchArrays[material->GetRenderLayerID()].AddRenderBatch(batch);
            }
        }
    }
}

void RenderPass::DrawLayers(Camera* camera)
{
    ShaderDescriptorCache::ClearDynamicBindigs();

    //per pass viewport bindings
    viewportSize = Vector2(viewport.dx, viewport.dy);
    rcpViewportSize = Vector2(1.0f / viewport.dx, 1.0f / viewport.dy);
    viewportOffset = Vector2(viewport.x, viewport.y);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_SIZE, &viewportSize, reinterpret_cast<pointer_size>(&viewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_RCP_VIEWPORT_SIZE, &rcpViewportSize, reinterpret_cast<pointer_size>(&rcpViewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_OFFSET, &viewportOffset, reinterpret_cast<pointer_size>(&viewportOffset));

    size_t size = renderLayers.size();
    for (size_t k = 0; k < size; ++k)
    {
        RenderLayer* layer = renderLayers[k];
        RenderBatchArray& batchArray = layersBatchArrays[layer->GetRenderLayerID()];
        batchArray.Sort(camera);

        layer->Draw(camera, batchArray, packetList);
    }
}

void RenderPass::DrawDebug(Camera* camera, RenderSystem* renderSystem)
{
    if (!renderSystem->GetDebugDrawer()->IsEmpty())
    {
        renderSystem->GetDebugDrawer()->Present(packetList, &camera->GetMatrix(), &camera->GetProjectionMatrix());
        renderSystem->GetDebugDrawer()->Clear();
    }
}

#if __DAVAENGINE_RENDERSTATS__
void RenderPass::ProcessVisibilityQuery()
{
    DVASSERT(queryBuffers.size() < 128);

    while (queryBuffers.size() && rhi::QueryBufferIsReady(queryBuffers.front()))
    {
        RenderStats& stats = Renderer::GetRenderStats();
        for (uint32 i = 0; i < static_cast<uint32>(RenderLayer::RENDER_LAYER_ID_COUNT); ++i)
        {
            FastName layerName = RenderLayer::GetLayerNameByID(static_cast<RenderLayer::eRenderLayerID>(i));
            stats.queryResults[layerName] += rhi::QueryValue(queryBuffers.front(), i);
        }

        rhi::DeleteQueryBuffer(queryBuffers.front());
        queryBuffers.pop_front();
    }
}
#endif

void RenderPass::ValidateMultisampledTextures(const rhi::RenderPassConfig& config)
{
    Size2i rtSize(config.viewport.width - config.viewport.x, config.viewport.height - config.viewport.y);

    if ((multisampledTexture == nullptr) || (multisampledTexture->width != rtSize.dx) || (multisampledTexture->height != rtSize.dy))
    {
        SafeRelease(multisampledTexture);

        Texture::FBODescriptor desc;
        desc.width = rtSize.dx;
        desc.height = rtSize.dy;
        desc.format = PixelFormat::FORMAT_RGBA8888;
        desc.needDepth = true;
        desc.needPixelReadback = false;
        desc.ensurePowerOf2 = false;
        desc.samples = config.samples;
        multisampledTexture = Texture::CreateFBO(desc);
    }
}

bool RenderPass::BeginRenderPass()
{
    bool success = false;

#if __DAVAENGINE_RENDERSTATS__
    ProcessVisibilityQuery();
    rhi::HQueryBuffer qBuffer = rhi::CreateQueryBuffer(RenderLayer::RENDER_LAYER_ID_COUNT);
    passConfig.queryBuffer = qBuffer;
    queryBuffers.push_back(qBuffer);
#endif

    rhi::RenderPassConfig localConfig = passConfig;

    if (localConfig.samples > 1)
    {
        ValidateMultisampledTextures(localConfig);
        localConfig.colorBuffer[0].resolveTexture = localConfig.colorBuffer[0].texture;
        localConfig.colorBuffer[0].texture = multisampledTexture->handle;

        localConfig.depthStencilBuffer.texture = multisampledTexture->handleDepthStencil;
    }

    renderPass = rhi::AllocateRenderPass(localConfig, 1, &packetList);
    if (renderPass != rhi::InvalidHandle)
    {
        rhi::BeginRenderPass(renderPass);
        rhi::BeginPacketList(packetList);
        success = true;
    }

    return success;
}

void RenderPass::EndRenderPass()
{
    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(renderPass);
}

void RenderPass::ClearLayersArrays()
{
    for (uint32 id = 0; id < static_cast<uint32>(RenderLayer::RENDER_LAYER_ID_COUNT); ++id)
    {
        layersBatchArrays[id].Clear();
    }
}

MainForwardRenderPass::MainForwardRenderPass(const FastName& name)
    : RenderPass(name)
    , reflectionPass(nullptr)
    , refractionPass(nullptr)
{
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_OPAQUE));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_AFTER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_OPAQUE));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_VEGETATION_ID, RenderLayer::LAYER_SORTING_FLAGS_VEGETATION));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_ALPHA_TEST_LAYER_ID, RenderLayer::LAYER_SORTING_FLAGS_ALPHA_TEST_LAYER));
    AddRenderLayer(new ShadowVolumeRenderLayer(RenderLayer::RENDER_LAYER_SHADOW_VOLUME_ID, RenderLayer::LAYER_SORTING_FLAGS_SHADOW_VOLUME));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_WATER_ID, RenderLayer::LAYER_SORTING_FLAGS_WATER));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_TRANSLUCENT));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_AFTER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_TRANSLUCENT));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_DEBUG_DRAW_ID, RenderLayer::LAYER_SORTING_FLAGS_DEBUG_DRAW));

    passConfig.priority = PRIORITY_MAIN_3D;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_RESOLVE;
    passConfig.samples = 4;
}

void MainForwardRenderPass::InitReflectionRefraction()
{
    DVASSERT(!reflectionPass);

    reflectionPass = new WaterReflectionRenderPass(PASS_REFLECTION_REFRACTION);
    reflectionPass->GetPassConfig().colorBuffer[0].texture = Renderer::GetRuntimeTextures().GetDynamicTexture(RuntimeTextures::TEXTURE_DYNAMIC_REFLECTION);
    reflectionPass->GetPassConfig().colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    reflectionPass->GetPassConfig().colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    reflectionPass->GetPassConfig().depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetDynamicTexture(RuntimeTextures::TEXTURE_DYNAMIC_RR_DEPTHBUFFER);
    reflectionPass->GetPassConfig().depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    reflectionPass->GetPassConfig().depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    reflectionPass->SetViewport(Rect(0, 0, static_cast<float32>(RuntimeTextures::REFLECTION_TEX_SIZE), static_cast<float32>(RuntimeTextures::REFLECTION_TEX_SIZE)));

    refractionPass = new WaterRefractionRenderPass(PASS_REFLECTION_REFRACTION);
    refractionPass->GetPassConfig().colorBuffer[0].texture = Renderer::GetRuntimeTextures().GetDynamicTexture(RuntimeTextures::TEXTURE_DYNAMIC_REFRACTION);
    refractionPass->GetPassConfig().colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    refractionPass->GetPassConfig().colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    refractionPass->GetPassConfig().depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetDynamicTexture(RuntimeTextures::TEXTURE_DYNAMIC_RR_DEPTHBUFFER);
    refractionPass->GetPassConfig().depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    refractionPass->GetPassConfig().depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    refractionPass->SetViewport(Rect(0, 0, static_cast<float32>(RuntimeTextures::REFRACTION_TEX_SIZE), static_cast<float32>(RuntimeTextures::REFRACTION_TEX_SIZE)));
}

void MainForwardRenderPass::PrepareReflectionRefractionTextures(RenderSystem* renderSystem)
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::WATER_REFLECTION_REFRACTION_DRAW))
        return;

    if (!reflectionPass)
        InitReflectionRefraction();

    const RenderBatchArray& waterLayerBatches = layersBatchArrays[RenderLayer::RENDER_LAYER_WATER_ID];
    uint32 waterBatchesCount = waterLayerBatches.GetRenderBatchCount();
    if (waterBatchesCount)
    {
        waterBox.Empty();
        for (uint32 i = 0; i < waterBatchesCount; ++i)
        {
            RenderBatch* batch = waterLayerBatches.Get(i);
            waterBox.AddAABBox(batch->GetRenderObject()->GetWorldBoundingBox());
        }
    }

    const float32* clearColor = Color::Black.color;
    if (QualitySettingsSystem::Instance()->GetAllowMetalFeatures())
        clearColor = static_cast<const float32*>(Renderer::GetDynamicBindings().GetDynamicParam(DynamicBindings::PARAM_WATER_CLEAR_COLOR));

    for (int32 i = 0; i < 4; ++i)
    {
        reflectionPass->GetPassConfig().colorBuffer[0].clearColor[i] = clearColor[i];
        refractionPass->GetPassConfig().colorBuffer[0].clearColor[i] = clearColor[i];
    }

    reflectionPass->SetWaterLevel(waterBox.max.z);
    reflectionPass->GetPassConfig().priority = passConfig.priority + PRIORITY_SERVICE_3D;
    reflectionPass->Draw(renderSystem);

    refractionPass->SetWaterLevel(waterBox.min.z);
    refractionPass->GetPassConfig().priority = passConfig.priority + PRIORITY_SERVICE_3D;
    refractionPass->Draw(renderSystem);
}

void MainForwardRenderPass::Draw(RenderSystem* renderSystem)
{
    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();

    /*    drawCamera->SetPosition(Vector3(5, 5, 5));
    drawCamera->SetTarget(Vector3(0, 0, 0));
    Vector4 clip(0, 0, 1, -1);*/
    SetupCameraParams(mainCamera, drawCamera);

    TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "PrepareVisibilityArrays")
    PrepareVisibilityArrays(mainCamera, renderSystem);
    TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "PrepareVisibilityArrays")

    passConfig.PerfQueryIndex0 = PERFQUERY__MAIN_PASS_T0;
    passConfig.PerfQueryIndex1 = PERFQUERY__MAIN_PASS_T1;

    if (BeginRenderPass())
    {
        TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "DrawLayers")
        DrawLayers(mainCamera);
        TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "DrawLayers")

        if (layersBatchArrays[RenderLayer::RENDER_LAYER_WATER_ID].GetRenderBatchCount() != 0)
            PrepareReflectionRefractionTextures(renderSystem);

        DrawDebug(drawCamera, renderSystem);

        EndRenderPass();
    }
}

MainForwardRenderPass::~MainForwardRenderPass()
{
    SafeDelete(reflectionPass);
    SafeDelete(refractionPass);
}

WaterPrePass::WaterPrePass(const FastName& name)
    : RenderPass(name)
    , passMainCamera(NULL)
    , passDrawCamera(NULL)
{
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_OPAQUE));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_AFTER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_OPAQUE));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_ALPHA_TEST_LAYER_ID, RenderLayer::LAYER_SORTING_FLAGS_ALPHA_TEST_LAYER));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_TRANSLUCENT));
    AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_AFTER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_TRANSLUCENT));

    passConfig.priority = PRIORITY_SERVICE_3D;
}
WaterPrePass::~WaterPrePass()
{
    SafeRelease(passMainCamera);
    SafeRelease(passDrawCamera);
}

WaterReflectionRenderPass::WaterReflectionRenderPass(const FastName& name)
    : WaterPrePass(name)
{
}

void WaterReflectionRenderPass::UpdateCamera(Camera* camera)
{
    Vector3 v;
    v = camera->GetPosition();
    v.z = waterLevel - (v.z - waterLevel);
    camera->SetPosition(v);
    v = camera->GetTarget();
    v.z = waterLevel - (v.z - waterLevel);
    camera->SetTarget(v);
}

void WaterReflectionRenderPass::Draw(RenderSystem* renderSystem)
{
    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();

    if (!passDrawCamera)
    {
        passMainCamera = new Camera();
        passDrawCamera = new Camera();
    }

    passMainCamera->CopyMathOnly(*mainCamera);
    UpdateCamera(passMainCamera);

    Vector4 clipPlane(0, 0, 1, -(waterLevel - 0.1f));
    Camera* currMainCamera = passMainCamera;
    Camera* currDrawCamera;

    if (drawCamera == mainCamera)
    {
        currDrawCamera = currMainCamera;
    }
    else
    {
        passDrawCamera->CopyMathOnly(*drawCamera);
        UpdateCamera(passDrawCamera);
        currDrawCamera = passDrawCamera;
    }

    SetupCameraParams(currMainCamera, currDrawCamera, &clipPlane);

    visibilityArray.clear();
    renderSystem->GetRenderHierarchy()->Clip(currMainCamera, visibilityArray, RenderObject::CLIPPING_VISIBILITY_CRITERIA | RenderObject::VISIBLE_REFLECTION);

    //[METAL_COMPLETE] THIS IS TEMPORARY SOLUTION TO ENNABLE IT FOR METAL ONLY
    if (QualitySettingsSystem::Instance()->GetAllowMetalFeatures())
        passName = PASS_REFLECTION_REFRACTION;
    else
        passName = PASS_FORWARD;

    ClearLayersArrays();
    PrepareLayersArrays(visibilityArray, currMainCamera);

    if (BeginRenderPass())
    {
        DrawLayers(currMainCamera);
        EndRenderPass();
    }
}

WaterRefractionRenderPass::WaterRefractionRenderPass(const FastName& name)
    : WaterPrePass(name)
{
    /*const RenderLayerManager * renderLayerManager = RenderLayerManager::Instance();
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_SHADOW_VOLUME), LAST_LAYER);*/
}

void WaterRefractionRenderPass::Draw(RenderSystem* renderSystem)
{
    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();

    if (!passDrawCamera)
    {
        passMainCamera = new Camera();
        passDrawCamera = new Camera();
    }

    passMainCamera->CopyMathOnly(*mainCamera);

    //-0.1f ?
    //Vector4 clipPlane(0,0, -1, waterLevel*3);
    Vector4 clipPlane(0, 0, -1, waterLevel + 0.1f);

    Camera* currMainCamera = passMainCamera;
    Camera* currDrawCamera;

    if (drawCamera == mainCamera)
    {
        currDrawCamera = currMainCamera;
    }
    else
    {
        passDrawCamera->CopyMathOnly(*drawCamera);
        currDrawCamera = passDrawCamera;
    }

    SetupCameraParams(currMainCamera, currDrawCamera, &clipPlane);

    visibilityArray.clear();
    renderSystem->GetRenderHierarchy()->Clip(currMainCamera, visibilityArray, RenderObject::CLIPPING_VISIBILITY_CRITERIA | RenderObject::VISIBLE_REFRACTION);

    //[METAL_COMPLETE] THIS IS TEMPORARY SOLUTION TO ENNABLE IT FOR METAL ONLY
    if (QualitySettingsSystem::Instance()->GetAllowMetalFeatures())
        passName = PASS_REFLECTION_REFRACTION;
    else
        passName = PASS_FORWARD;

    ClearLayersArrays();
    PrepareLayersArrays(visibilityArray, currMainCamera);

    if (BeginRenderPass())
    {
        DrawLayers(currMainCamera);
        EndRenderPass();
    }
}
};
