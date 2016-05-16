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

    //update lod
    int32 objectCount = static_cast<int32>(lodComponents.size());
    for (int32 i = 0; i < objectCount; ++i)
    {
        LodComponent* lodComponent = lodComponents[i];
        Entity* entity = lodComponent->GetEntity();
        int32 newLod = RecheckLod(lodComponent, lodOffset, lodMult, GetScene()->GetCurrentCamera());
        if (lodComponent->currentLod != newLod)
        {
            lodComponent->currentLod = newLod;
            ParticleEffectComponent* effect = GetEffectComponent(entity);
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

                SetEntityLod(entity, layerNum);
            }
        }
    }
}

void LodSystem::AddEntity(Entity* entity)
{
    lodComponents.push_back(GetLodComponent(entity));
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
            return;
        }
    }

    DVASSERT(0);
}

int32 LodSystem::RecheckLod(LodComponent* lodComponent, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera)
{
    bool usePsSettings = (GetEffectComponent(lodComponent->GetEntity()) != NULL);

    if (LodComponent::INVALID_LOD_LAYER != lodComponent->forceLodLayer)
    {
        return lodComponent->forceLodLayer;
    }

    int32 layersCount = LodComponent::MAX_LOD_LAYERS;
    float32 dst = CalculateDistanceToCamera(lodComponent->GetEntity(), lodComponent, camera);

    if (usePsSettings)
    {
        if (dst > lodComponent->GetLodLayerFarSquare(0)) //preserve lod 0 from degrade
            dst = dst * psLodMultSq + psLodOffsetSq;
    }

    int32 layer = FindProperLayer(dst, lodComponent, layersCount);
    return layer;
}

float32 LodSystem::CalculateDistanceToCamera(const Entity* entity, const LodComponent* lodComponent, Camera* camera)
{
    if (lodComponent->forceDistance != LodComponent::INVALID_DISTANCE) //LodComponent::INVALID_DISTANCE
    {
        return lodComponent->forceDistanceSq;
    }

    if (camera)
    {
        float32 dst = (camera->GetPosition() - entity->GetWorldTransform().GetTranslationVector()).SquareLength();
        dst *= camera->GetZoomFactor() * camera->GetZoomFactor();

        return dst;
    }

    return 0.f;
}

int32 LodSystem::FindProperLayer(float32 distance, const LodComponent* lodComponent, int32 requestedLayersCount)
{
    if (lodComponent->currentLod != LodComponent::INVALID_LOD_LAYER)
    {
        if ((distance >= lodComponent->GetLodLayerNearSquare(lodComponent->currentLod)) && (distance <= lodComponent->GetLodLayerFarSquare(lodComponent->currentLod)))
        {
            return lodComponent->currentLod;
        }
    }

    int32 layer = LodComponent::INVALID_LOD_LAYER;
    for (int32 i = requestedLayersCount - 1; i >= 0; --i)
    {
        if (distance < lodComponent->GetLodLayerFarSquare(i))
        {
            layer = i;
        }
    }

    return layer;
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
