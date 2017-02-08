#pragma once

#include "DAVAEngine.h"
#include "Classes/Qt/Scene/System/EditorSceneSystem.h"

class ParticleDebugRenderPass;
class ParticleDebugDrawQuadRenderPass;

enum eParticleDebugDrawMode
{
    WIREFRAME,
    LOW_ALPHA,
    OVERDRAW
};

class ParticleEffectDebugDrawSystem : public DAVA::SceneSystem, public EditorSceneSystem
{
public:
    ParticleEffectDebugDrawSystem(Scene* scene);

    ~ParticleEffectDebugDrawSystem();

    void RemoveEntity(Entity* entity) override;
    void RemoveComponent(Entity* entity, Component* component) override;
    void ImmediateEvent(Component* component, uint32 event) override;
    void Process(float32 timeElapsed) override;
    void Draw() override;

    void GenerateDebugMaterials();
    void GenerateQuadMaterials();

    DAVA::Texture* GenerateHeatTexture();
    Vector4 LerpColors(float normalizedWidth);

    inline eParticleDebugDrawMode GetDrawMode() const;
    inline void SetDrawMode(eParticleDebugDrawMode mode);

    inline bool GetIsEnabled() const;
    inline void SetIsEnabled(bool enable);
    inline const Vector<NMaterial*>* const GetMaterials() const;


private:
    struct TextureKey
    {
        Vector4 color = {};
        float32 time = 0.0f;
        TextureKey(Vector4 color, float32 time) : color(color), time(time)
        {}
    };

    void AddToActive(ParticleEffectComponent* effect);
    void RemoveFromActive(ParticleEffectComponent* effect);

    DAVA::Vector<ParticleEffectComponent*> activeComponents;
    DAVA::UnorderedMap<RenderObject*, ParticleEffectComponent*> componentsMap;
    ParticleDebugRenderPass* renderPass = nullptr;
    ParticleDebugDrawQuadRenderPass* drawQuadPass = nullptr;
    DAVA::RenderSystem* renderSystem = nullptr;

    bool isEnabled = false;
    eParticleDebugDrawMode drawMode = WIREFRAME;

    NMaterial* wireframeMaterial = nullptr;
    NMaterial* overdrawMaterial = nullptr;
    NMaterial* showAlphaMaterial = nullptr;

    NMaterial* quadMaterial = nullptr;
    NMaterial* quadHeatMaterial = nullptr;

    Texture* heatTexture;

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

bool ParticleEffectDebugDrawSystem::GetIsEnabled() const 
{
    return isEnabled;
}

void ParticleEffectDebugDrawSystem::SetIsEnabled(bool enable)
{
    isEnabled = enable;
}

const Vector<NMaterial*>* const ParticleEffectDebugDrawSystem::GetMaterials() const
{
    return &materials;
}
 