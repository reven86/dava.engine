#include "Particles/ParticleEffectDebug/ParticleDebugRenderPass.h"

#include "Render/RHI/rhi_Type.h"
#include "Render/RHI/rhi_Public.h"
#include "Render/ShaderCache.h"

#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"

namespace DAVA
{
const FastName ParticleDebugRenderPass::PASS_DEBUG_DRAW_PARTICLES("ForwardPass");

Texture* ParticleDebugRenderPass::GetTexture() const
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
                                             // Remove obj if obj is not a particle or if showOnlySelected flag is set
                                             // if obj is not a particle or it is particle but not in selection set
                                             bool isParticle = obj->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER;
                                             bool inSet = drawOnlySelected ? selectedParticles->count(obj) > 0 : true;

                                             return !(isParticle && inSet);
                                         }),
                          visibilityArray.end());

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
                particleBatches.AddRenderBatch(batch);
        }
    }
}

ParticleDebugRenderPass::ParticleDebugRenderPass(ParticleDebugRenderPassConfig config)
    : RenderPass(config.name)
    , wireframeMaterial(config.wireframeMaterial)
    , overdrawMaterial(config.overdrawMaterial)
    , selectedParticles(config.selectedParticles)
    , showAlphaMaterial(config.showAlphaMaterial)
    , drawMode(config.drawMode)
    , drawOnlySelected(config.drawOnlySelected)
{
    passConfig.priority = DAVA::PRIORITY_MAIN_3D;

    static const int width = 1024;
    static const int height = 1024;
    debugTexture = Texture::CreateFBO(width, height, PixelFormat::FORMAT_RGBA8888);
    SetRenderTargetProperties(width, height, PixelFormat::FORMAT_RGBA8888);

    passConfig.colorBuffer[0].texture = debugTexture->handle;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.colorBuffer[0].clearColor[0] = 0.0f;
    passConfig.colorBuffer[0].clearColor[1] = 0.0f;
    passConfig.colorBuffer[0].clearColor[2] = 0.0f;
    passConfig.colorBuffer[0].clearColor[3] = 0.0f;
    SetViewport(Rect(0, 0, static_cast<float32>(height), static_cast<float32>(width)));
}

void ParticleDebugRenderPass::Draw(DAVA::RenderSystem* renderSystem)
{
    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();
    SetupCameraParams(mainCamera, drawCamera);

    if (BeginRenderPass())
    {
        PrepareParticlesVisibilityArray(mainCamera, renderSystem);
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
        NMaterial* mat = SelectMaterial(batch);

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

NMaterial* ParticleDebugRenderPass::SelectMaterial(RenderBatch* batch)
{
    switch (drawMode)
    {
    case eParticleDebugDrawMode::WIREFRAME:
        if (wireframeMaterial->PreBuildMaterial(passName))
            return wireframeMaterial;
        break;
    case eParticleDebugDrawMode::OVERDRAW:
        if (overdrawMaterial->PreBuildMaterial(passName))
            return overdrawMaterial;
        break;
    case eParticleDebugDrawMode::LOW_ALPHA:
        NMaterial* mat = nullptr;
        if (showAlphaMaterial->PreBuildMaterial(passName))
            mat = showAlphaMaterial;
        if (mat->HasLocalTexture(NMaterialTextureName::TEXTURE_ALBEDO))
            mat->SetTexture(NMaterialTextureName::TEXTURE_ALBEDO, batch->GetMaterial()->GetLocalTexture(NMaterialTextureName::TEXTURE_ALBEDO));
        else
            mat->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, batch->GetMaterial()->GetLocalTexture(NMaterialTextureName::TEXTURE_ALBEDO));
        return mat;
        break;
    }
    return nullptr;
}
}