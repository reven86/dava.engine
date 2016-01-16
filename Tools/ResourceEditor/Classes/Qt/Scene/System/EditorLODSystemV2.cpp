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


#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Utils/Utils.h"



#include "Scene/System/EditorLODSystemV2.h"

using namespace DAVA;


void LODComponentHolder::SummarizeValues()
{
    Array<float32, LodComponent::MAX_LOD_LAYERS> lodDistances;
    lodDistances.fill(0.f);

    maxLodLayerIndex = LodComponent::INVALID_LOD_LAYER;

    uint32 count = static_cast<uint32> (lodComponents.size());
    if (count > 0)
    {
        const uint32 firstActualIndex = 1;  //because lod0 == 0;

        for (auto & lc : lodComponents)
        {
            maxLodLayerIndex = Max(maxLodLayerIndex, static_cast<int32>(GetLodLayersCount(lc)) - 1);

            for (uint32 i = firstActualIndex; i < LodComponent::MAX_LOD_LAYERS; ++i)
            {
                lodDistances[i] += lc->GetLodLayerDistance(i);
            }
        }

        for (uint32 i = firstActualIndex; i < LodComponent::MAX_LOD_LAYERS; ++i)
        {
            lodDistances[i] /= count;
        }

        std::sort(lodDistances.begin(), lodDistances.end());
    }

    for (uint32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
    {
        mergedComponent.SetLodLayerDistance(i, lodDistances[i]);
    }
}

void LODComponentHolder::ApplyForce(const ForceValues &force)
{
    for (auto & lc : lodComponents)
    {
        if (force.flag & ForceValues::APPLY_LAYER)
        {
            lc->SetForceLodLayer(force.layer);
        }

        if (force.flag & ForceValues::APPLY_DISTANCE)
        {
            lc->SetForceDistance(force.distance);
        }
    }
}

bool LODComponentHolder::IsMultyComponent() const
{
    return (lodComponents.size() > 1);
}

int32 LODComponentHolder::GetMaxLODLayer() const
{
    return maxLodLayerIndex;
}

DAVA::uint32 LODComponentHolder::GetLODLayersCount() const
{
    return (maxLodLayerIndex + 1);
}



EditorLODSystemV2::EditorLODSystemV2(Scene* scene)
    : SceneSystem(scene)
{
    activeLodData = &lodData[mode];
}

EditorLODSystemV2::~EditorLODSystemV2()
{
    activeLodData = nullptr;
}


void EditorLODSystemV2::Process(float32 timeElapsed)
{
}

void EditorLODSystemV2::AddComponent(Entity * entity, Component * component)
{
    DVASSERT(component->GetType() == Component::LOD_COMPONENT);

    lodData[MODE_ALL_SCENE].lodComponents.push_back(static_cast<LodComponent *>(component));
    lodData[MODE_ALL_SCENE].SummarizeValues();
}

void EditorLODSystemV2::RemoveComponent(Entity * entity, Component * component)
{
    DVASSERT(component->GetType() == Component::LOD_COMPONENT);

    LodComponent * removedComponent = static_cast<LodComponent *>(component);
    for (uint32 m = 0; m < MODE_COUNT; ++m)
    {
        bool removed = FindAndRemoveExchangingWithLast(lodData[m].lodComponents, removedComponent);
        if (removed)
        {
            lodData[m].SummarizeValues();
        }
    }
}


EditorLODSystemV2::eMode EditorLODSystemV2::GetMode() const
{
    return mode;
}

void EditorLODSystemV2::SetMode(EditorLODSystemV2::eMode mode_)
{
    if (mode == mode_)
    {
        return;
    }

    DVASSERT(activeLodData != nullptr);

    activeLodData->ApplyForce({ -1, -1, ForceValues::APPLY_BOTH});
    mode = mode_;
    activeLodData = &lodData[mode];
    activeLodData->ApplyForce(forceValues);
}

void EditorLODSystemV2::CreatePlaneLOD()
{
    DVASSERT(activeLodData != nullptr);

}

void EditorLODSystemV2::DeleteFirstLOD()
{
    DVASSERT(activeLodData != nullptr);

    if (activeLodData->GetLODLayersCount() > 0)
    {
        activeLodData->DeleteLOD(0);
        activeLodData->SummarizeValues();
    }
}

void EditorLODSystemV2::DeleteLastLOD()
{
    DVASSERT(activeLodData != nullptr);

    int32 lastLayer = activeLodData->GetMaxLODLayer();
    if (lastLayer >= 0)
    {
        activeLodData->DeleteLOD(lastLayer);
        activeLodData->SummarizeValues();
    }
}

