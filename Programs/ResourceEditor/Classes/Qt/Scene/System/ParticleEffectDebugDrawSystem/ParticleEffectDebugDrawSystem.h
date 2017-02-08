#pragma once

#include "DAVAEngine.h"
#include "Classes/Qt/Scene/System/EditorSceneSystem.h"

class ParticleDebugRenderPass;
class ParticleDebugDrawQuadRenderPass;

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

private:
    void AddToActive(ParticleEffectComponent* effect);
    void RemoveFromActive(ParticleEffectComponent* effect);

    DAVA::Vector<ParticleEffectComponent*> activeComponents;
    DAVA::UnorderedMap<RenderObject*, ParticleEffectComponent*> componentsMap;
    ParticleDebugRenderPass* renderPass = nullptr;
    ParticleDebugDrawQuadRenderPass* drawQuadPass = nullptr;
    DAVA::RenderSystem* renderSystem = nullptr;

    NMaterial* wireframeMaterial = nullptr;
    NMaterial* overdrawMaterial = nullptr;
    NMaterial* showAlphaMaterial = nullptr;
};
