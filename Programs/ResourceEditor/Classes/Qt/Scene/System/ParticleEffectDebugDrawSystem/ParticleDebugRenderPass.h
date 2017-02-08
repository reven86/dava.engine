#pragma once

#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Base/FastName.h"

class ParticleDebugRenderPass : public DAVA::RenderPass
{

public:
    ParticleDebugRenderPass(const DAVA::FastName& name, RenderSystem* renderSystem, NMaterial* wireframeMaterial, NMaterial* overdrawMaterial, NMaterial* showAlphaMaterial, DAVA::UnorderedMap<RenderObject*, ParticleEffectComponent*>* componentsMap);
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

    void DrawBatches(Camera* camera);
    void PrepareParticlesVisibilityArray(Camera* camera, RenderSystem* renderSystem);
    void PrepareParticlesBatchesArray(const Vector<RenderObject*> objectsArray, Camera* camera);
    void MakePacket(Camera* camera);
};
