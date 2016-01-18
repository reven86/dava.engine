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
#include "Entity/SceneSystem.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Utils/StringFormat.h"
#include "Utils/Utils.h"


#include "Commands2/DeleteLODCommand.h"
#include "Commands2/ChangeLODDistanceCommand.h"

#include "Scene/SceneEditor2.h"
#include "Scene/System/EditorLODSystemV2.h"

using namespace DAVA;

void LODComponentHolder::BindToSystem(EditorLODSystemV2 *system_, SceneEditor2 *scene_)
{
    DVASSERT(system_ != nullptr);
    DVASSERT(scene_ != nullptr);

    system = system_;
    scene = scene_;
}


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

void LODComponentHolder::PropagateValues()
{
    scene->BeginBatch("LOD Distance Changed");
    for (auto & lc : lodComponents)
    {
        const int32 layersCount = static_cast<int32>(GetLodLayersCount(lc));
        for (int32 i = 0; i < layersCount; ++i)
        {
            scene->Exec(new ChangeLODDistanceCommand(lc, i, mergedComponent.GetLodLayerDistance(i)));
        }
    }
    scene->EndBatch();
}

bool LODComponentHolder::DeleteLOD(DAVA::int32 layer)
{
    bool wasLayerRemoved = false;

    scene->BeginBatch(Format("Delete lod layer %", layer));
    for (auto & lc : lodComponents)
    {
        if (GetLodLayersCount(lc) > 1 && (GetEffectComponent(lc->GetEntity()) == nullptr))
        {
            scene->Exec(new DeleteLODCommand(lc, layer, -1));
            wasLayerRemoved = true;
        }
    }
    scene->EndBatch();

    return wasLayerRemoved;
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

const DAVA::LodComponent & LODComponentHolder::GetLODComponent() const
{
    return mergedComponent;
}

//SYSTEM

EditorLODSystemV2::EditorLODSystemV2(Scene* scene)
    : SceneSystem(scene)
{
    for (uint32 m = 0; m < MODE_COUNT; ++m)
    {
        lodData[m].BindToSystem(this, static_cast<SceneEditor2 *>(GetScene()));
    }

    const bool allSceneModeEnabled = SettingsManager::GetValue(Settings::Internal_LODEditorMode).AsBool();
    mode = (allSceneModeEnabled) ? MODE_ALL_SCENE : MODE_SELECTION;

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

    EmitUpdateForceUI();
    EmitUpdateDistanceUI();
    EmitUpdateActionsUI();
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
            EmitUpdateForceUI();
            EmitUpdateDistanceUI();
            EmitUpdateActionsUI();
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

    EmitUpdateForceUI();
    EmitUpdateDistanceUI();
    EmitUpdateActionsUI();
}

const ForceValues & EditorLODSystemV2::GetForceValues() const
{
    return forceValues;
}

void EditorLODSystemV2::SetForceValues(const ForceValues & values)
{
    DVASSERT(activeLodData != nullptr);

    ForceValues distanceDiffValues;
    distanceDiffValues.flag = ForceValues::APPLY_BOTH;

    if (values.flag != forceValues.flag)
    {
        if (values.flag & ForceValues::APPLY_DISTANCE)
        {
            distanceDiffValues.distance = values.distance;
            distanceDiffValues.layer = LodComponent::INVALID_LOD_LAYER;
        }
        if (values.flag & ForceValues::APPLY_LAYER)
        {
            distanceDiffValues.distance = LodComponent::INVALID_DISTANCE;
            distanceDiffValues.layer = values.layer;
        }
    }
    if (values.distance != forceValues.distance)
    {
        distanceDiffValues.distance = values.distance;
        distanceDiffValues.layer = LodComponent::INVALID_LOD_LAYER;
    }
    if (values.layer != forceValues.layer)
    {
        distanceDiffValues.distance = LodComponent::INVALID_DISTANCE;
        distanceDiffValues.layer = values.layer;
    }

    activeLodData->ApplyForce(distanceDiffValues);
    forceValues = values;

    EmitUpdateForceUI();
}


bool EditorLODSystemV2::CanDeleteLOD() const
{
    DVASSERT(activeLodData != nullptr);

    bool canDeleteLod = !activeLodData->lodComponents.empty();
    for (auto &lc : activeLodData->lodComponents)
    {
        if (GetEffectComponent(lc->GetEntity()) != nullptr)
        {
            canDeleteLod = false;
            break;
        }
    }

    return canDeleteLod;
}

bool EditorLODSystemV2::CanCreateLOD() const
{
    DVASSERT(activeLodData != nullptr);
    
    bool canCreateLod = (activeLodData->GetLODLayersCount() < LodComponent::MAX_LOD_LAYERS);
    for (auto &lc : activeLodData->lodComponents)
    {
        if (GetEffectComponent(lc->GetEntity()) != nullptr)
        {
            canCreateLod = false;
            break;
        }
    }

    return canCreateLod && !activeLodData->IsMultyComponent();
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
        bool deleted = activeLodData->DeleteLOD(0);
        if (deleted)
        {
            activeLodData->SummarizeValues();

            EmitUpdateForceUI();
            EmitUpdateDistanceUI();
            EmitUpdateActionsUI();
        }
    }
}

void EditorLODSystemV2::DeleteLastLOD()
{
    DVASSERT(activeLodData != nullptr);

    int32 lastLayer = activeLodData->GetMaxLODLayer();
    if (lastLayer >= 0)
    {
        bool deleted = activeLodData->DeleteLOD(lastLayer);
        if (deleted)
        {
            activeLodData->SummarizeValues();

            EmitUpdateForceUI();
            EmitUpdateDistanceUI();
            EmitUpdateActionsUI();
        }
    }
}

void EditorLODSystemV2::CopyLastLODToFirst()
{

}



const LODComponentHolder * EditorLODSystemV2::GetActiveLODData() const
{
    return activeLodData;
}

void EditorLODSystemV2::SetLODDistances(const Array<float32, LodComponent::MAX_LOD_LAYERS> &distances)
{
    DVASSERT(activeLodData != nullptr);

    for (int32 i = 0; i < static_cast<int32>(distances.size()); ++i)
    {
        activeLodData->mergedComponent.SetLodLayerDistance(i, distances[i]);
    }

    activeLodData->PropagateValues();
    EmitUpdateDistanceUI();
}

void EditorLODSystemV2::SolidChanged(const DAVA::Entity *entity, bool value)
{

}

void EditorLODSystemV2::SelectionChanged(const EntityGroup *selected, const EntityGroup *deselected)
{

}



