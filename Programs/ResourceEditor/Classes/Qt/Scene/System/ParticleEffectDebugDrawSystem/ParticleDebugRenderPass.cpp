#include "Classes/Qt/Scene/System/ParticleEffectDebugDrawSystem/ParticleDebugRenderPass.h"
#include "Render/RHI/rhi_Type.h"
#include "Render/RHI/rhi_Public.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"

using namespace DAVA;

const DAVA::FastName ParticleDebugRenderPass::PASS_DEBUG_DRAW_PARTICLES("ForwardPass");

DAVA::Texture* ParticleDebugRenderPass::GetTexture() const
{
    return debugTexture;
}


void ParticleDebugRenderPass::PrepareParticlesVisibilityArray(Camera* camera, RenderSystem* renderSystem)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::RENDER_PASS_PREPARE_ARRAYS)

    uint32 currVisibilityCriteria = RenderObject::CLIPPING_VISIBILITY_CRITERIA;
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::ENABLE_STATIC_OCCLUSION))
        currVisibilityCriteria &= ~RenderObject::VISIBLE_STATIC_OCCLUSION;

    visibilityArray.clear();
    renderSystem->GetRenderHierarchy()->Clip(camera, visibilityArray, currVisibilityCriteria);
    visibilityArray.erase(std::remove_if(visibilityArray.begin(), visibilityArray.end(),
        [this](RenderObject* obj) 
        {
        return (*componentsMap).count(obj) == 0 ||
            obj->GetType() != RenderObject::TYPE_PARTICLE_EMTITTER ||
            !(*componentsMap)[obj]->GetWireframeMode();            
        }), visibilityArray.end());

    particleBatches.Clear();
    PrepareParticlesBatchesArray(visibilityArray, camera);
}

void ParticleDebugRenderPass::PrepareParticlesBatchesArray(const Vector<RenderObject*> objectsArray, Camera* camera)
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
                particleBatches.AddRenderBatch(batch);
                //layersBatchArrays[material->GetRenderLayerID()].AddRenderBatch(batch);
            }
            
            //batch->SetMaterial(tmp);
        }
    }
}

ParticleDebugRenderPass::ParticleDebugRenderPass(const DAVA::FastName& name, RenderSystem* renderSystem, NMaterial* wireframeMaterial, NMaterial* overdrawMaterial, NMaterial* showAlphaMaterial, DAVA::UnorderedMap<RenderObject*, ParticleEffectComponent*>* componentsMap)
    : RenderPass(name), wireframeMaterial(wireframeMaterial), overdrawMaterial(overdrawMaterial), componentsMap(componentsMap), showAlphaMaterial(showAlphaMaterial)
{
    passConfig.priority = DAVA::PRIORITY_MAIN_3D;
    
    static const int width = 1024;
    static const int height = 1024;
    debugTexture = DAVA::Texture::CreateFBO(width, height, DAVA::PixelFormat::FORMAT_RGBA8888);
    SetRenderTargetProperties(width, height, DAVA::PixelFormat::FORMAT_RGBA8888);

    passConfig.colorBuffer[0].texture = debugTexture->handle;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.colorBuffer[0].clearColor[0] = 0.0f;
    passConfig.colorBuffer[0].clearColor[1] = 0.0f;
    passConfig.colorBuffer[0].clearColor[2] = 0.0f;
    passConfig.colorBuffer[0].clearColor[3] = 0.0f;
    SetViewport(Rect(0, 0, width, width));

//     AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_OPAQUE));
//     AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_AFTER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_OPAQUE));
//     AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_VEGETATION_ID, RenderLayer::LAYER_SORTING_FLAGS_VEGETATION));
//     AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_ALPHA_TEST_LAYER_ID, RenderLayer::LAYER_SORTING_FLAGS_ALPHA_TEST_LAYER));
// //     AddRenderLayer(new ShadowVolumeRenderLayer(RenderLayer::RENDER_LAYER_SHADOW_VOLUME_ID, RenderLayer::LAYER_SORTING_FLAGS_SHADOW_VOLUME));
//     AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_WATER_ID, RenderLayer::LAYER_SORTING_FLAGS_WATER));
//     AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_TRANSLUCENT));
//     AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_AFTER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_TRANSLUCENT));
//     AddRenderLayer(new RenderLayer(RenderLayer::RENDER_LAYER_DEBUG_DRAW_ID, RenderLayer::LAYER_SORTING_FLAGS_DEBUG_DRAW));
}

void ParticleDebugRenderPass::Draw(DAVA::RenderSystem* renderSystem)
{
    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();
    SetupCameraParams(mainCamera, drawCamera);


    if (BeginRenderPass())
    {
        PrepareParticlesVisibilityArray(mainCamera, renderSystem);
        //DrawLayers(mainCamera);
        DrawBatches(mainCamera);
        EndRenderPass();
    }
}

void ParticleDebugRenderPass::DrawBatches(Camera* camera)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::RENDER_PASS_DRAW_LAYERS)

        ShaderDescriptorCache::ClearDynamicBindigs();

    //per pass viewport bindings
    viewportSize = Vector2(viewport.dx, viewport.dy);
    rcpViewportSize = Vector2(1.0f / viewport.dx, 1.0f / viewport.dy);
    viewportOffset = Vector2(viewport.x, viewport.y);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_SIZE, &viewportSize, reinterpret_cast<pointer_size>(&viewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_RCP_VIEWPORT_SIZE, &rcpViewportSize, reinterpret_cast<pointer_size>(&rcpViewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_OFFSET, &viewportOffset, reinterpret_cast<pointer_size>(&viewportOffset));
            
    particleBatches.Sort(camera);

    MakePacket(camera);
}

void ParticleDebugRenderPass::MakePacket(Camera* camera)
{
    uint32 size = static_cast<uint32>(particleBatches.GetRenderBatchCount());
    rhi::Packet packet;
    for (uint32 i = 0; i < size; i++)
    {
        RenderBatch* batch = particleBatches.Get(i);
        RenderObject* renderObject = batch->GetRenderObject();
        renderObject->BindDynamicParameters(camera);
        NMaterial* mat = batch->GetMaterial();
        if (wireframeMaterial->PreBuildMaterial(passName))
            mat = wireframeMaterial;
        if (overdrawMaterial->PreBuildMaterial(passName))
            mat = overdrawMaterial;

        if (showAlphaMaterial->PreBuildMaterial(passName))
            mat = showAlphaMaterial;
        if (mat->HasLocalTexture(NMaterialTextureName::TEXTURE_ALBEDO))
            mat->SetTexture(NMaterialTextureName::TEXTURE_ALBEDO, batch->GetMaterial()->GetLocalTexture(NMaterialTextureName::TEXTURE_ALBEDO));
        else
            mat->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, batch->GetMaterial()->GetLocalTexture(NMaterialTextureName::TEXTURE_ALBEDO));

        if (mat != nullptr)
        {
            batch->BindGeometryData(packet);
            DVASSERT(packet.primitiveCount);
            mat->BindParams(packet);
            packet.debugMarker = mat->GetEffectiveFXName().c_str();
            packet.perfQueryStart = batch->perfQueryStart;
            packet.perfQueryEnd = batch->perfQueryEnd;

            rhi::AddPacket(packetList, packet);
        }
    }
}
