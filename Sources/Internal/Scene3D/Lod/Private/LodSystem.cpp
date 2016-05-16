/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Scene3D/Lod/LodSystem.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Render/Highlevel/Camera.h"
#include "Platform/SystemTimer.h"
#include "Core/PerformanceSettings.h"
#include "Debug/Stats.h"
#include "..\LodSystem.h"

namespace DAVA
{
LodSystem::LodSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void LodSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("LodSystem::Process");
    if (timeElapsed == 0.f)
    {
        timeElapsed = 0.000001f;
    }

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

    for (LodComponentInternal& internal : fastVector)
    {
        LodComponent* lodComponent = internal.lod;
        ParticleEffectComponent* effect = internal.effect;
        if (effect && effect->IsStopped())
        {
            //do not update inactive effects
        }
        else
        {
            int32 newLod;
            if (LodComponent::INVALID_LOD_LAYER != lodComponent->forceLodLayer)
            {
                newLod = lodComponent->forceLodLayer;
            }
            else
            {
                int32 layersCount = LodComponent::MAX_LOD_LAYERS;
                float32 dst;
                if (lodComponent->forceDistance != LodComponent::INVALID_DISTANCE)
                {
                    dst = lodComponent->forceDistanceSq;
                }
                else
                {
                    dst = (cameraPos - internal.transform->GetWorldTransform().GetTranslationVector()).SquareLength();
                    dst *= cameraZoomFactorSq;
                }

                if (effect)
                {
                    if (dst > lodComponent->GetLodLayerFarSquare(0)) //preserve lod 0 from degrade
                        dst = dst * lodMult + lodOffset;
                }

                if ((lodComponent->currentLod != LodComponent::INVALID_LOD_LAYER) &&
                    (dst >= lodComponent->GetLodLayerNearSquare(lodComponent->currentLod)) &&
                    (dst <= lodComponent->GetLodLayerFarSquare(lodComponent->currentLod)))
                {
                    newLod = lodComponent->currentLod;
                }
                else
                {
                    newLod = LodComponent::INVALID_LOD_LAYER;
                    for (int32 i = layersCount - 1; i >= 0; --i)
                    {
                        if (dst < lodComponent->GetLodLayerFarSquare(i))
                        {
                            newLod = i;
                        }
                    }
                }
            }

            if (lodComponent->currentLod != newLod)
            {
                lodComponent->currentLod = newLod;

                if (effect)
                {
                    effect->SetDesiredLodLevel(lodComponent->currentLod);
                }
                else
                {
                    int32 layerNum = lodComponent->currentLod;
                    if (layerNum != LodComponent::LAST_LOD_LAYER)
                    {
                        DVASSERT(0 <= layerNum && layerNum < LodComponent::MAX_LOD_LAYERS);
                    }

                    SetEntityLod(internal.entity, layerNum);
                }
            }
        }
    }
}

void LodSystem::AddEntity(Entity* entity)
{
    lodComponents.push_back(GetLodComponent(entity));
    TransformComponent* transform = static_cast<TransformComponent*>(entity->GetComponent(Component::TRANSFORM_COMPONENT));
    LodComponent* lod = static_cast<LodComponent*>(entity->GetComponent(Component::LOD_COMPONENT));
    ParticleEffectComponent* effect = static_cast<ParticleEffectComponent*>(entity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    fastVector.emplace_back(entity, transform, lod, effect, static_cast<int32>(fastVector.size()));

    fastMap.emplace(entity, &fastVector.back());
}

void LodSystem::RemoveEntity(Entity* entity)
{
    LodComponent* component = GetLodComponent(entity);
    int32 size = static_cast<int32>(lodComponents.size());
    for (int32 i = 0; i < size; ++i)
    {
        if (lodComponents[i] == component)
        {
            lodComponents[i] = lodComponents[size - 1];
            lodComponents.pop_back();
            break;
        }
    }

    auto iter = fastMap.find(entity);
    DVASSERT(iter != fastMap.end());
    //TODO: delete from vector
    fastMap.erase(entity);
}

void LodSystem::RegisterComponent(Entity* entity, Component* component)
{
    if (component->GetType() == Component::PARTICLE_EFFECT_COMPONENT)
    {
        auto iter = fastMap.find(entity);
        if (iter != fastMap.end())
        {
            LodComponentInternal* internal = iter->second;
            DVASSERT(internal->effect == nullptr);
            internal->effect = static_cast<ParticleEffectComponent*>(component);
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
            LodComponentInternal* internal = iter->second;
            DVASSERT(internal->effect != nullptr);
            internal->effect = nullptr;
        }
    }
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

}
