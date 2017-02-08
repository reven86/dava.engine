#include "Classes/Qt/Scene/System/ParticleEffectDebugDrawSystem/ParticleEffectDebugDrawSystem.h"
#include "Classes/Qt/Scene/System/ParticleEffectDebugDrawSystem/ParticleDebugRenderPass.h"
#include "Classes/Qt/Scene/System/ParticleEffectDebugDrawSystem/ParticleDebugDrawQuadRenderPass.h"


#include "Logger/Logger.h"

ParticleEffectDebugDrawSystem::ParticleEffectDebugDrawSystem(Scene* scene) : SceneSystem(scene)
{
    if (wireframeMaterial == nullptr)
    {
        wireframeMaterial = new NMaterial();
        wireframeMaterial->SetFXName(NMaterialName::DEBUG_DRAW_WIREFRAME);

        Color wireframeColor(1.0f, 1.0f, 1.0f, 1.0f);
        wireframeMaterial->AddProperty(FastName("color"), wireframeColor.color, rhi::ShaderProp::TYPE_FLOAT4);
    }
    if (overdrawMaterial == nullptr)
    {
        overdrawMaterial = new NMaterial();
        overdrawMaterial->SetFXName(NMaterialName::PARTICLES);
        overdrawMaterial->AddFlag(FastName("PARTICLE_DEBUG_SHOW_OVERDRAW"), true);
        overdrawMaterial->AddFlag(NMaterialFlagName::FLAG_BLENDING, eBlending::BLENDING_ADDITIVE);
    }
    if (showAlphaMaterial == nullptr)
    {
        showAlphaMaterial = new NMaterial();
        showAlphaMaterial->SetFXName(NMaterialName::PARTICLES);
        showAlphaMaterial->AddFlag(FastName("PARTICLE_DEBUG_SHOW_ALPHA"), true);
        showAlphaMaterial->AddFlag(NMaterialFlagName::FLAG_BLENDING, eBlending::BLENDING_ALPHABLEND);
        float32 threshold = 0.05f;
        showAlphaMaterial->AddProperty(FastName("particleAlphaThreshold"), &threshold, rhi::ShaderProp::TYPE_FLOAT1);
        
        //showAlphaMaterial->SetTexture(NMaterialTextureName::TEXTURE_ALBEDO, nullptr);
    }
    if (scene != nullptr)
    {

        scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::START_PARTICLE_EFFECT);
        scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::STOP_PARTICLE_EFFECT);
        renderSystem = scene->GetRenderSystem();
        renderPass = new ParticleDebugRenderPass(ParticleDebugRenderPass::PASS_DEBUG_DRAW_PARTICLES, renderSystem, wireframeMaterial, overdrawMaterial, showAlphaMaterial, &componentsMap); 
        drawQuadPass = new ParticleDebugDrawQuadRenderPass(ParticleDebugDrawQuadRenderPass::PASS_DEBUG_DRAW_QUAD, renderSystem, renderPass->GetTexture());
    }
}

ParticleEffectDebugDrawSystem::~ParticleEffectDebugDrawSystem()
{
    SafeDelete(renderPass);
    SafeRelease(wireframeMaterial);
    SafeRelease(overdrawMaterial);
    SafeRelease(showAlphaMaterial);
}

void ParticleEffectDebugDrawSystem::Draw()
{
    renderPass->Draw(renderSystem);
    drawQuadPass->Draw(renderSystem);
}

void ParticleEffectDebugDrawSystem::AddToActive(ParticleEffectComponent* effect)
{
    Vector<ParticleEffectComponent*>::iterator it = std::find(activeComponents.begin(), activeComponents.end(), effect);
    if (it == activeComponents.end())
        activeComponents.push_back(effect);
    componentsMap[effect->GetRenderObject()] = effect;
}

void ParticleEffectDebugDrawSystem::RemoveFromActive(ParticleEffectComponent* effect)
{
    Vector<ParticleEffectComponent*>::iterator it = std::find(activeComponents.begin(), activeComponents.end(), effect);
    DVASSERT(it != activeComponents.end());
    activeComponents.erase(it);
    componentsMap.erase(effect->GetRenderObject());
}


void ParticleEffectDebugDrawSystem::RemoveEntity(Entity* entity)
{
    ParticleEffectComponent* effect = static_cast<ParticleEffectComponent*>(entity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    if (effect != nullptr && effect->GetAnimationState() != ParticleEffectComponent::STATE_STOPPED)
        RemoveFromActive(effect);
}

void ParticleEffectDebugDrawSystem::RemoveComponent(Entity* entity, Component* component)
{
    ParticleEffectComponent* effect = static_cast<ParticleEffectComponent*>(component);
    if (effect != nullptr && effect->GetAnimationState() != ParticleEffectComponent::STATE_STOPPED)
        RemoveFromActive(effect);
}

void ParticleEffectDebugDrawSystem::ImmediateEvent(Component* component, uint32 event)
{
    DVASSERT(component->GetType() == Component::PARTICLE_EFFECT_COMPONENT);
    ParticleEffectComponent* effect = static_cast<ParticleEffectComponent*>(component);
    if (event == EventSystem::START_PARTICLE_EFFECT)
        AddToActive(effect);
    else if (event == EventSystem::STOP_PARTICLE_EFFECT)
        RemoveFromActive(effect);
}

void ParticleEffectDebugDrawSystem::Process(float32 timeElapsed)
{

}