#pragma once

#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Classes/Qt/Scene/System/ParticleEffectDebugDrawSystem/ParticleEffectDebugDrawSystem.h"
#include "Base/FastName.h"

class ParticleDebugRenderPass : public DAVA::RenderPass
{

public:
    struct ParticleDebugRenderPassConfig
    {
        const DAVA::FastName& name;
        RenderSystem* renderSystem;
        NMaterial* wireframeMaterial;
        NMaterial* overdrawMaterial;
        NMaterial* showAlphaMaterial;
        const eParticleDebugDrawMode& drawMode;
        UnorderedMap<RenderObject*, ParticleEffectComponent*>* componentsMap;
    };

    ParticleDebugRenderPass(ParticleDebugRenderPassConfig config);
    void Draw(DAVA::RenderSystem* renderSystem) override;
    static const DAVA::FastName PASS_DEBUG_DRAW_PARTICLES;
    DAVA::Texture* GetTexture() const;

private:
    DAVA::Texture* debugTexture;
    NMaterial* wireframeMaterial;
    NMaterial* overdrawMaterial;
    NMaterial* showAlphaMaterial;
    RenderBatchArray particleBatches;
    DAVA::UnorderedMap<RenderObject*, ParticleEffectComponent*>* componentsMap;
    const eParticleDebugDrawMode& drawMode;

    void DrawBatches(Camera* camera);
    void PrepareParticlesVisibilityArray(Camera* camera, RenderSystem* renderSystem);
    void PrepareParticlesBatchesArray(const Vector<RenderObject*> objectsArray, Camera* camera);
    void MakePacket(Camera* camera);
    NMaterial* SelectMaterial(RenderBatch* batch);
};
