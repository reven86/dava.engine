#pragma once

#include "Base/FastName.h"
#include "Render/RenderBase.h"

namespace DAVA
{
class RenderSystem;
class NMaterial;
class Texture;

enum eParticleDebugDrawMode;

class ParticleDebugRenderPass : public RenderPass
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
        const bool& drawOnlySelected;
        UnorderedSet<RenderObject*>* selectedParticles;
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
    DAVA::UnorderedSet<RenderObject*>* selectedParticles;
    const eParticleDebugDrawMode& drawMode;
    const bool& drawOnlySelected;

    void DrawBatches(Camera* camera);
    void PrepareParticlesVisibilityArray(Camera* camera, RenderSystem* renderSystem);
    void PrepareParticlesBatchesArray(const Vector<RenderObject*> objectsArray, Camera* camera);
    void MakePacket(Camera* camera);
    NMaterial* SelectMaterial(RenderBatch* batch);
};
}