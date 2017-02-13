#pragma once

#include "DAVAEngine.h"

namespace DAVA
{
class ParticleDebugRenderPass;
class ParticleDebugDrawQuadRenderPass;

enum eParticleDebugDrawMode
{
    WIREFRAME,
    LOW_ALPHA,
    OVERDRAW
};

class ParticleEffectDebugDrawSystem : public SceneSystem
{
public:
    ParticleEffectDebugDrawSystem(Scene* scene);

    ~ParticleEffectDebugDrawSystem();

    void Draw();

    void GenerateDebugMaterials();
    void GenerateQuadMaterials();

    DAVA::Texture* GenerateHeatTexture();
    Vector4 LerpColors(float normalizedWidth);

    inline eParticleDebugDrawMode GetDrawMode() const;
    inline void SetDrawMode(eParticleDebugDrawMode mode);

    inline bool GetIsDrawOnlySected() const;
    inline void SetIsDrawOnlySelected(bool showOnlySelected);

    inline const Vector<NMaterial*>* const GetMaterials() const;

    void SetSelectedParticles(DAVA::UnorderedSet<RenderObject*> selectedParticles);
    void SetAlphaThreshold(float32 threshold);

private:
    struct TextureKey
    {
        Vector4 color = {};
        float32 time = 0.0f;
        TextureKey(Vector4 color, float32 time) : color(color), time(time)
        {}
    };

    DAVA::UnorderedSet<RenderObject*> selectedParticles;
    ParticleDebugRenderPass* renderPass = nullptr;
    ParticleDebugDrawQuadRenderPass* drawQuadPass = nullptr;
    DAVA::RenderSystem* renderSystem = nullptr;

    bool isDrawOnlySelected = false;
    eParticleDebugDrawMode drawMode = WIREFRAME;

    NMaterial* wireframeMaterial = nullptr;
    NMaterial* overdrawMaterial = nullptr;
    NMaterial* showAlphaMaterial = nullptr;

    NMaterial* quadMaterial = nullptr;
    NMaterial* quadHeatMaterial = nullptr;

    Texture* heatTexture = nullptr;

    Vector<NMaterial*> materials;
};

eParticleDebugDrawMode ParticleEffectDebugDrawSystem::GetDrawMode() const
{
    return drawMode;
}

void ParticleEffectDebugDrawSystem::SetDrawMode(eParticleDebugDrawMode mode)
{
    drawMode = mode;
}

bool ParticleEffectDebugDrawSystem::GetIsDrawOnlySected() const
{
    return isDrawOnlySelected;
}

void ParticleEffectDebugDrawSystem::SetIsDrawOnlySelected(bool enable)
{
    isDrawOnlySelected = enable;
}

const Vector<NMaterial*>* const ParticleEffectDebugDrawSystem::GetMaterials() const
{
    return &materials;
}
}