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

namespace DAVA
{
LodSystem::LodSystem(Scene* scene)
    : SceneSystem(scene)
{
    camera = 0;
    forceUpdateAll = true;

    partialUpdateIndices.reserve(UPDATE_PART_PER_FRAME + 1);
    UpdatePartialUpdateIndices();
}

void LodSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("LodSystem::Process");
    if (timeElapsed == 0.f)
    {
        timeElapsed = 0.000001f;
    }

    SetCamera(GetScene()->GetCurrentCamera());

    float32 currFps = 1.0f / timeElapsed;

    float32 currPSValue = (currFps - PerformanceSettings::Instance()->GetPsPerformanceMinFPS()) / (PerformanceSettings::Instance()->GetPsPerformanceMaxFPS() - PerformanceSettings::Instance()->GetPsPerformanceMinFPS());
    currPSValue = Clamp(currPSValue, 0.0f, 1.0f);
    float32 lodOffset = PerformanceSettings::Instance()->GetPsPerformanceLodOffset() * (1 - currPSValue);
    float32 lodMult = 1.0f + (PerformanceSettings::Instance()->GetPsPerformanceLodMult() - 1.0f) * (1 - currPSValue);
    /*as we use square values - multiply it too*/
    lodOffset *= lodOffset;
    lodMult *= lodMult;

    if (forceUpdateAll)
    {
        int32 objectCount = static_cast<int32>(entities.size());
        for (int32 i = 0; i < objectCount; ++i)
        {
            Entity* entity = entities[i];

            ProcessEntity(entity, lodOffset, lodMult, camera);
        }

        if (objectCount > 0)
        {
            forceUpdateAll = false;
        }
    }
    else
    {
        for (int32 i = partialUpdateIndices[currentPartialUpdateIndex]; i < partialUpdateIndices[currentPartialUpdateIndex + 1]; ++i)
        {
            Entity* entity = entities[i];
            ProcessEntity(entity, lodOffset, lodMult, camera);
        }

        currentPartialUpdateIndex = currentPartialUpdateIndex < UPDATE_PART_PER_FRAME - 1 ? currentPartialUpdateIndex + 1 : 0;
    }
}

void LodSystem::ForceUpdate(Entity* entity, Camera* camera, float32 timeElapsed)
{
    float32 currFps = 1.0f / timeElapsed;

    float32 currPSValue = (currFps - PerformanceSettings::Instance()->GetPsPerformanceMinFPS()) / (PerformanceSettings::Instance()->GetPsPerformanceMaxFPS() - PerformanceSettings::Instance()->GetPsPerformanceMinFPS());
    currPSValue = Clamp(currPSValue, 0.0f, 1.0f);
    float32 lodOffset = PerformanceSettings::Instance()->GetPsPerformanceLodOffset() * (1 - currPSValue);
    float32 lodMult = 1.0f + (PerformanceSettings::Instance()->GetPsPerformanceLodMult() - 1.0f) * (1 - currPSValue);
    /*as we use square values - multiply it too*/
    lodOffset *= lodOffset;
    lodMult *= lodMult;

    PorcessEntityRecursive(entity, lodOffset, lodMult, camera);
}

void LodSystem::PorcessEntityRecursive(Entity* entity, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera)
{
    LodComponent* lod = GetLodComponent(entity);
    if (lod)
    {
        ProcessEntity(entity, psLodOffsetSq, psLodMultSq, camera);
    }

    int32 childrenCount = entity->GetChildrenCount();
    for (int32 i = 0; i < childrenCount; ++i)
    {
        PorcessEntityRecursive(entity->GetChild(i), psLodOffsetSq, psLodMultSq, camera);
    }
}


void LodSystem::AddEntity(Entity* entity)
{
    entities.push_back(entity);
    UpdatePartialUpdateIndices();
}

void LodSystem::RemoveEntity(Entity* entity)
{
    uint32 size = static_cast<uint32>(entities.size());
    for (uint32 i = 0; i < size; ++i)
    {
        if (entities[i] == entity)
        {
            entities[i] = entities[size - 1];
            entities.pop_back();
            UpdatePartialUpdateIndices();
            return;
        }
    }

    DVASSERT(0);
}

void LodSystem::UpdatePartialUpdateIndices()
{
    currentPartialUpdateIndex = 0;

    int32 size = static_cast<int32>(entities.size());
    int32 partSize = size / UPDATE_PART_PER_FRAME;

    partialUpdateIndices.clear();
    int32 currentIndex = 0;
    partialUpdateIndices.push_back(currentIndex);
    for (int32 i = 0; i < UPDATE_PART_PER_FRAME; ++i)
    {
        currentIndex += partSize;
        partialUpdateIndices.push_back(currentIndex);
    }

    int32& LastSlot = partialUpdateIndices[partialUpdateIndices.size() - 1];
    LastSlot = Max(LastSlot, size);
}

void LodSystem::UpdateLod(Entity* entity, LodComponent* lodComponent, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera)
{
    int32 oldLod = lodComponent->currentLod;
    if (!RecheckLod(entity, lodComponent, psLodOffsetSq, psLodMultSq, camera))
    {
        //		if (oldLod != LodComponent::INVALID_LOD_LAYER)
        //		{
        //			lodComponent->SetLayerVisibility(oldLod, false);
        //		}

        lodComponent->currentLod = LodComponent::INVALID_LOD_LAYER;
        return;
    }

    if (oldLod != lodComponent->currentLod)
    {
        ParticleEffectComponent* effect = GetEffectComponent(entity);
        if (effect)
        {
            effect->SetDesiredLodLevel(lodComponent->currentLod);
            return;
        }

        int32 layerNum = lodComponent->currentLod;
        if (layerNum != LodComponent::LAST_LOD_LAYER)
        {
            DVASSERT(0 <= layerNum && layerNum < LodComponent::MAX_LOD_LAYERS);
        }

        SetEntityLod(entity, layerNum);
    }
}

bool LodSystem::RecheckLod(Entity* entity, LodComponent* lodComponent, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera)
{
    bool usePsSettings = (GetEffectComponent(entity) != NULL);

    if (LodComponent::INVALID_LOD_LAYER != lodComponent->forceLodLayer)
    {
        lodComponent->currentLod = lodComponent->forceLodLayer;
        return true;
    }

    int32 layersCount = LodComponent::MAX_LOD_LAYERS;
    float32 dst = CalculateDistanceToCamera(entity, lodComponent, camera);

    if (usePsSettings)
    {
        if (dst > lodComponent->GetLodLayerFarSquare(0)) //preserv lod 0 from degrade
            dst = dst * psLodMultSq + psLodOffsetSq;
    }

    int32 layer = FindProperLayer(dst, lodComponent, layersCount);
    if (layer != lodComponent->currentLod)
    {
        lodComponent->currentLod = layer;
    }

    return (lodComponent->currentLod != LodComponent::INVALID_LOD_LAYER);
}

DAVA::float32 LodSystem::CalculateDistanceToCamera(const Entity* entity, const LodComponent* lodComponent, Camera* camera)
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

void LodSystem::SetCamera(Camera* _camera)
{
    camera = _camera;
}

}
