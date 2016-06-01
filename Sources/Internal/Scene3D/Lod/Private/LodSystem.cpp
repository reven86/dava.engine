#include "Scene3D/Lod/LodSystem.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Render/Highlevel/Camera.h"
#include "Platform/SystemTimer.h"
#include "Core/PerformanceSettings.h"
#include "Debug/Stats.h"
#include "Debug/Profiler.h"
#include "Scene3D/Systems/EventSystem.h"

namespace DAVA
{
LodSystem::LodSystem(Scene* scene)
    : SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::START_PARTICLE_EFFECT);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::STOP_PARTICLE_EFFECT);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::LOD_DISTANCE_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::LOD_RECURSIVE_UPDATE_ENABLED);
}

void LodSystem::Process(float32 timeElapsed)
{
    SCOPED_NAMED_TIMING("LodSystem::Process");
    if (timeElapsed == 0.f)
    {
        timeElapsed = 0.000001f;
    }

    //lod degrade
    float32 currFps = 1.0f / timeElapsed;
    float32 currPSValue = (currFps - PerformanceSettings::Instance()->GetPsPerformanceMinFPS()) / (PerformanceSettings::Instance()->GetPsPerformanceMaxFPS() - PerformanceSettings::Instance()->GetPsPerformanceMinFPS());
    currPSValue = Clamp(currPSValue, 0.0f, 1.0f);
    float32 lodOffset = PerformanceSettings::Instance()->GetPsPerformanceLodOffset() * (1 - currPSValue);
    float32 lodMult = 1.0f + (PerformanceSettings::Instance()->GetPsPerformanceLodMult() - 1.0f) * (1 - currPSValue);
    /*as we use square values - multiply it too*/
    lodOffset *= lodOffset;
    lodMult *= lodMult;

    Camera* camera = GetScene()->GetCurrentCamera();
    if (!camera)
    {
        return;
    }
    Vector3 cameraPos = camera->GetPosition();
    float32 cameraZoomFactorSq = camera->GetZoomFactor() * camera->GetZoomFactor();

    int32 size = static_cast<int32>(fastVector.size());
    for (int32 index = 0; index < size; ++index)
    {
        FastStruct& fast = fastVector[index];
        __builtin_prefetch(&fastVector[index + 1]);
        __builtin_prefetch(&fastVector[index + 2]);
        __builtin_prefetch(&fastVector[index + 3]);
        __builtin_prefetch(&fastVector[index + 4]);

        if (fast.effectStopped)
        {
            //do not update inactive effects
        }
        else
        {
            int32 newLod = 0;
            if (forceLodUsed)
            {
                SlowStruct& slow = slowVector[index];
                if (slow.forceLodLayer != LodComponent::INVALID_LOD_LAYER)
                {
                    newLod = slow.forceLodLayer;
                }
            }
            else
            {
                float32 dst;
                if (forceLodUsed && slowVector[index].forceLodDistance != LodComponent::INVALID_DISTANCE)
                {
                    SlowStruct& slow = slowVector[index];
                    dst = slow.forceLodDistance * slow.forceLodDistance;
                }
                else
                {
                    dst = (cameraPos - fast.position).SquareLength();
                    dst *= cameraZoomFactorSq;
                }

                if (fast.isEffect)
                {
                    if (dst > fast.farSquare0) //preserve lod 0 from degrade
                        dst = dst * lodMult + lodOffset;
                }

                if ((fast.currentLod != LodComponent::INVALID_LOD_LAYER) &&
                    (dst >= fast.nearSquare) &&
                    (dst <= fast.farSquare))
                {
                    newLod = fast.currentLod;
                }
                else
                {
                    newLod = LodComponent::INVALID_LOD_LAYER;
                    SlowStruct* slow = &slowVector[index];
                    for (int32 i = LodComponent::MAX_LOD_LAYERS - 1; i >= 0; --i)
                    {
                        if (dst < slow->farSquares[i])
                        {
                            newLod = i;
                        }
                    }
                    //DVASSERT(newLod != LodComponent::INVALID_LOD_LAYER);
                }
            }

            //switch lod
            if (fast.currentLod != newLod)
            {
                fast.currentLod = newLod;
                SlowStruct& slow = slowVector[index];
                fast.nearSquare = slow.nearSquares[fast.currentLod];
                fast.farSquare = slow.farSquares[fast.currentLod];
                slow.lod->SetCurrentLod(fast.currentLod);

                ParticleEffectComponent* effect = slow.effect;
                if (effect)
                {
                    effect->SetDesiredLodLevel(fast.currentLod);
                }
                else
                {
                    if (fast.currentLod != LodComponent::LAST_LOD_LAYER)
                    {
                        //DVASSERT(0 <= fast.currentLod && fast.currentLod < LodComponent::MAX_LOD_LAYERS);
                    }

                    if (slow.recursiveUpdate)
                    {
                        SetEntityLodRecursive(slow.entity, fast.currentLod);
                    }
                    else
                    {
                        SetEntityLod(slow.entity, fast.currentLod);
                    }
                }
            }
        }
    }
}

void LodSystem::UpdateDistances(LodComponent* from, LodSystem::SlowStruct* to)
{
    //lods will overlap +- 5%
    to->nearSquares[0] = 0.f;
    to->farSquares[0] = from->GetLodLayerDistance(0) * 1.05f;
    to->farSquares[0] *= to->farSquares[0];

    for (int32 i = 1; i < LodComponent::MAX_LOD_LAYERS - 1; ++i)
    {
        to->nearSquares[i] = from->GetLodLayerDistance(i - 1) * 0.95f;
        to->nearSquares[i] *= to->nearSquares[i];

        to->farSquares[i] = from->GetLodLayerDistance(i) * 1.05f;
        to->farSquares[i] *= to->farSquares[i];
    }

    to->nearSquares[LodComponent::MAX_LOD_LAYERS - 1] = from->GetLodLayerDistance(LodComponent::MAX_LOD_LAYERS - 2) * 0.95f;
    to->nearSquares[LodComponent::MAX_LOD_LAYERS - 1] *= to->nearSquares[LodComponent::MAX_LOD_LAYERS - 1];
    to->farSquares[LodComponent::MAX_LOD_LAYERS - 1] = std::numeric_limits<float32>::max();
}

void LodSystem::AddEntity(Entity* entity)
{
    TransformComponent* transform = static_cast<TransformComponent*>(entity->GetComponent(Component::TRANSFORM_COMPONENT));
    LodComponent* lod = static_cast<LodComponent*>(entity->GetComponent(Component::LOD_COMPONENT));
    ParticleEffectComponent* effect = static_cast<ParticleEffectComponent*>(entity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    Vector3 position = transform->GetWorldTransform().GetTranslationVector();
    bool effectStopped = effect ? effect->IsStopped() : false;

    SlowStruct slow;
    slow.entity = entity;
    slow.lod = lod;
    slow.effect = effect;
    UpdateDistances(lod, &slow);
    slowVector.push_back(slow);

    FastStruct fast{
        slow.farSquares[0],
        position,
        lod->currentLod,
        lod->currentLod == LodComponent::INVALID_LOD_LAYER ? -1.f : slow.nearSquares[lod->currentLod],
        lod->currentLod == LodComponent::INVALID_LOD_LAYER ? -1.f : slow.farSquares[lod->currentLod],
        effectStopped,
        effect != nullptr
    };

    fastVector.push_back(fast);
    fastMap.insert(std::make_pair(entity, static_cast<int32>(fastVector.size() - 1)));
}

void LodSystem::RemoveEntity(Entity* entity)
{
    //find in fastMap
    auto iter = fastMap.find(entity);
    DVASSERT(iter != fastMap.end());
    int32 index = iter->second;

    //delete from slow
    SlowStruct& slowLast = slowVector.back();
    slowVector[index] = slowLast;
    slowVector.pop_back();

    //delete from fast
    FastStruct& fastLast = fastVector.back();
    fastVector[index] = fastLast;
    fastVector.pop_back();

    //delete in fastMap
    fastMap.erase(entity);
    Entity* movedEntity = slowLast.entity;
    if (entity != movedEntity)
    {
        fastMap[movedEntity] = index;
    }
}

void LodSystem::RegisterComponent(Entity* entity, Component* component)
{
    if (component->GetType() == Component::PARTICLE_EFFECT_COMPONENT)
    {
        auto iter = fastMap.find(entity);
        if (iter != fastMap.end())
        {
            int32 index = iter->second;
            SlowStruct* slow = &slowVector[index];
            DVASSERT(slow->effect == nullptr);
            slow->effect = static_cast<ParticleEffectComponent*>(component);
            FastStruct* fast = &fastVector[index];
            fast->isEffect = slow->effect != nullptr;
        }
    }
}

void LodSystem::UnregisterComponent(Entity* entity, Component* component)
{
    if (component->GetType() == Component::PARTICLE_EFFECT_COMPONENT)
    {
        auto iter = fastMap.find(entity);
        if (iter != fastMap.end())
        {
            int32 index = iter->second;
            SlowStruct* slow = &slowVector[index];
            DVASSERT(slow->effect == nullptr);
            slow->effect = nullptr;
            FastStruct* fast = &fastVector[index];
            fast->isEffect = false;
        }
    }
}

void LodSystem::ImmediateEvent(Component* component, uint32 event)
{
    switch (event)
    {
    case EventSystem::WORLD_TRANSFORM_CHANGED:
    {
        DVASSERT(component->GetType() == Component::TRANSFORM_COMPONENT);
        TransformComponent* transform = static_cast<TransformComponent*>(component);
        auto iter = fastMap.find(component->GetEntity());
        if (iter != fastMap.end())
        {
            int32 index = iter->second;
            FastStruct* fast = &fastVector[index];
            fast->position = transform->GetWorldTransform().GetTranslationVector();
        }
    }
    break;

    case EventSystem::START_PARTICLE_EFFECT:
    case EventSystem::STOP_PARTICLE_EFFECT:
    {
        DVASSERT(component->GetType() == Component::PARTICLE_EFFECT_COMPONENT);
        ParticleEffectComponent* effect = static_cast<ParticleEffectComponent*>(component);
        auto iter = fastMap.find(component->GetEntity());
        if (iter != fastMap.end())
        {
            int32 index = iter->second;
            FastStruct* fast = &fastVector[index];
            fast->effectStopped = event == EventSystem::STOP_PARTICLE_EFFECT ? true : false;
        }
    }
    break;

    case EventSystem::LOD_DISTANCE_CHANGED:
    {
        DVASSERT(component->GetType() == Component::LOD_COMPONENT);
        LodComponent* lod = static_cast<LodComponent*>(component);
        auto iter = fastMap.find(component->GetEntity());
        if (iter != fastMap.end())
        {
            int32 index = iter->second;
            SlowStruct* slow = &slowVector[index];
            UpdateDistances(lod, slow);
        }
    }
    break;

    case EventSystem::LOD_RECURSIVE_UPDATE_ENABLED:
    {
        DVASSERT(component->GetType() == Component::LOD_COMPONENT);
        LodComponent* lod = static_cast<LodComponent*>(component);
        auto iter = fastMap.find(component->GetEntity());
        DVASSERT(iter != fastMap.end());
        int32 index = iter->second;
        SlowStruct* slow = &slowVector[index];
        slow->recursiveUpdate = true;
    }
    break;

    default:
        break;
    }
}

void LodSystem::SetForceLodLayer(LodComponent* forComponent, int32 layer)
{
    auto iter = fastMap.find(forComponent->GetEntity());
    DVASSERT(iter != fastMap.end());
    int32 index = iter->second;
    SlowStruct* slow = &slowVector[index];
    slow->forceLodLayer = layer;

    if (layer != LodComponent::INVALID_LOD_LAYER)
    {
        forceLodUsed = true;
    }
}

int32 LodSystem::GetForceLodLayer(LodComponent* forComponent)
{
    auto iter = fastMap.find(forComponent->GetEntity());
    DVASSERT(iter != fastMap.end());
    int32 index = iter->second;
    SlowStruct* slow = &slowVector[index];
    return slow->forceLodLayer;
}

void LodSystem::SetForceLodDistance(LodComponent* forComponent, float32 distance)
{
    auto iter = fastMap.find(forComponent->GetEntity());
    DVASSERT(iter != fastMap.end());
    int32 index = iter->second;
    SlowStruct* slow = &slowVector[index];
    slow->forceLodDistance = distance;

    if (distance != LodComponent::INVALID_DISTANCE)
    {
        forceLodUsed = true;
    }
}

DAVA::float32 LodSystem::GetForceLodDistance(LodComponent* forComponent)
{
    auto iter = fastMap.find(forComponent->GetEntity());
    DVASSERT(iter != fastMap.end());
    int32 index = iter->second;
    SlowStruct* slow = &slowVector[index];
    return slow->forceLodDistance;
}

void LodSystem::SetEntityLod(Entity* entity, int32 currentLod)
{
    RenderObject* ro = GetRenderObject(entity);
    if (ro)
    {
        if (currentLod == LodComponent::LAST_LOD_LAYER)
        {
            ro->SetLodIndex(ro->GetMaxLodIndex());
        }
        else
        {
            ro->SetLodIndex(currentLod);
        }
    }
}

void LodSystem::SetEntityLodRecursive(Entity* entity, int32 currentLod)
{
    SetEntityLod(entity, currentLod);

    int32 count = entity->GetChildrenCount();
    for (int32 i = 0; i < count; ++i)
    {
        SetEntityLodRecursive(entity->GetChild(i), currentLod);
    }
}
}
