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

#include "Commands2/Command2.h"
#include "Commands2/DeleteLODCommand.h"
#include "Commands2/ChangeLODDistanceCommand.h"

#include "Scene/EntityGroup.h"
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
    trianglesCount.fill(0u);

    maxLodLayerIndex = LodComponent::INVALID_LOD_LAYER;

    uint32 count = static_cast<uint32> (lodComponents.size());
    if (count > 0)
    {
        UnorderedSet<RenderObject*> renderObjects;
        renderObjects.reserve(count);

        for (auto & lc : lodComponents)
        {
            maxLodLayerIndex = Max(maxLodLayerIndex, static_cast<int32>(GetLodLayersCount(lc)) - 1);

            for (uint32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
            {
                lodDistances[i] += lc->GetLodLayerDistance(i);
            }

            InsertObjectWithTriangles(lc->GetEntity(), renderObjects);
        }

        for (uint32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
        {
            lodDistances[i] /= count;
        }

        std::sort(lodDistances.begin(), lodDistances.end());

        CalculateTriangles(renderObjects);
    }

    for (uint32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
    {
        mergedComponent.SetLodLayerDistance(i, lodDistances[i]);
    }
}

void LODComponentHolder::InsertObjectWithTriangles(Entity *entity, UnorderedSet<RenderObject *> &renderObjects)
{
    RenderObject * ro = GetRenderObject(entity);
    if (ro && (ro->GetType() == RenderObject::TYPE_MESH || ro->GetType() == RenderObject::TYPE_SPEED_TREE || ro->GetType() == RenderObject::TYPE_SKINNED_MESH))
    {
        renderObjects.insert(ro);
    }

    int32 count = entity->GetChildrenCount();
    for (int32 i = 0; i < count; ++i)
    {
        InsertObjectWithTriangles(entity->GetChild(i), renderObjects);
    }
}

void LODComponentHolder::CalculateTriangles(const UnorderedSet<RenderObject*> &renderObjects)
{
    bool onlyVisibleBatches = false; // to save leagcy code

    for (auto & ro : renderObjects)
    {
        DAVA::uint32 batchCount = ro->GetRenderBatchCount();
        for (DAVA::uint32 b = 0; b < batchCount; ++b)
        {
            DAVA::int32 lodIndex = 0;
            DAVA::int32 switchIndex = 0;

            RenderBatch *rb = ro->GetRenderBatch(b, lodIndex, switchIndex);
            if (lodIndex < 0)
            {
                continue;
            }

            if (IsPointerToExactClass<RenderBatch>(rb))
            {
                if (onlyVisibleBatches)
                { //check batch visibility

                    bool batchIsVisible = false;
                    DAVA::uint32 activeBatchCount = ro->GetActiveRenderBatchCount();
                    for (DAVA::uint32 a = 0; a < activeBatchCount; ++a)
                    {
                        RenderBatch *visibleBatch = ro->GetActiveRenderBatch(a);
                        if (visibleBatch == rb)
                        {
                            batchIsVisible = true;
                            break;
                        }
                    }

                    if (batchIsVisible == false) // need to skip this render batch
                    {
                        continue;
                    }
                }

                PolygonGroup *pg = rb->GetPolygonGroup();
                if (nullptr != pg)
                {
                    DVASSERT(lodIndex < DAVA::LodComponent::MAX_LOD_LAYERS);
                    DVASSERT(lodIndex >= 0);
                    trianglesCount[lodIndex] += pg->GetIndexCount() / 3;
                }
            }
        }
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

bool LODComponentHolder::DeleteLOD(int32 layer)
{
    bool wasLayerRemoved = false;

    scene->BeginBatch(Format("Delete lod layer %", layer));
    for (auto & lc : lodComponents)
    {
        if (GetLodLayersCount(lc) > 1 && (HasComponent(lc->GetEntity(), Component::PARTICLE_EFFECT_COMPONENT) == false))
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

uint32 LODComponentHolder::GetLODLayersCount() const
{
    return (maxLodLayerIndex + 1);
}

const LodComponent & LODComponentHolder::GetLODComponent() const
{
    return mergedComponent;
}

const Array<uint32, LodComponent::MAX_LOD_LAYERS> &LODComponentHolder::GetTriangles() const
{
    return trianglesCount;
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
    DispatchSignals();
}

void EditorLODSystemV2::AddEntity(Entity * entity)
{
    LodComponent *lc = GetLodComponent(entity);
    DVASSERT(lc != nullptr);

    AddComponent(entity, lc);
}

void EditorLODSystemV2::RemoveEntity(Entity * entity)
{
    LodComponent *lc = GetLodComponent(entity);
    DVASSERT(lc != nullptr);

    RemoveComponent(entity, lc);
}


void EditorLODSystemV2::AddComponent(Entity * entity, Component * component)
{
    DVASSERT(component->GetType() == Component::LOD_COMPONENT);

    lodData[MODE_ALL_SCENE].lodComponents.push_back(static_cast<LodComponent *>(component));
    lodData[MODE_ALL_SCENE].SummarizeValues();

    if (mode == MODE_ALL_SCENE)
    {
        EmitUpdateModeUI();
        EmitUpdateForceUI();
        EmitUpdateDistanceUI();
        EmitUpdateActionsUI();
    }
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

            if (m == mode)
            {
                EmitUpdateModeUI();
                EmitUpdateForceUI();
                EmitUpdateDistanceUI();
                EmitUpdateActionsUI();
            }
        }
    }
}


EditorLODSystemV2::eMode EditorLODSystemV2::GetMode() const
{
    return mode;
}

void EditorLODSystemV2::SetMode(EditorLODSystemV2::eMode mode_)
{
    DVASSERT(activeLodData != nullptr);

    activeLodData->ApplyForce({ -1, -1, ForceValues::APPLY_BOTH});
    mode = mode_;
    activeLodData = &lodData[mode];
    activeLodData->ApplyForce(forceValues);

    EmitUpdateModeUI();
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
        if (HasComponent(lc->GetEntity(), Component::PARTICLE_EFFECT_COMPONENT))
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
        if (HasComponent(lc->GetEntity(), Component::PARTICLE_EFFECT_COMPONENT))
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
        generateCommands = true;
        bool deleted = activeLodData->DeleteLOD(0);
        generateCommands = false;
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
        generateCommands = true;
        bool deleted = activeLodData->DeleteLOD(lastLayer);
        generateCommands = false;
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

    generateCommands = true;
    activeLodData->PropagateValues();
    generateCommands = false;

    EmitUpdateDistanceUI();
}

void EditorLODSystemV2::SolidChanged(const Entity *entity, bool value)
{
    SceneEditor2 *sceneEditor = static_cast<SceneEditor2 *> (GetScene());
    EntityGroup selection = sceneEditor->selectionSystem->GetSelection();

    if (selection.ContainsEntity(entity) == false)
    {
        return;
    }

    SelectionChanged(&selection, nullptr);
}

void EditorLODSystemV2::SelectionChanged(const EntityGroup *selected, const EntityGroup *deselected)
{
    lodData[MODE_SELECTION].lodComponents.clear();

    bool ignoreChildren = SettingsManager::GetValue(Settings::Scene_RefreshLodForNonSolid).AsBool();

    uint32 count = selected->Size();
    Vector<Entity *>lodEntities;
    lodEntities.reserve(count);    //mostly we have less than 5 lods in hierarchy
    for (uint32 i = 0; i < count; ++i)
    {
        Entity *entity = selected->GetEntity(i);
        if (entity->GetSolid() || !ignoreChildren)
        {
            entity->GetChildEntitiesWithComponent(lodEntities, Component::LOD_COMPONENT);
        }

        if (entity->GetComponentCount(Component::LOD_COMPONENT) > 0)
        {
            lodEntities.push_back(entity);
        }
    }

    for (auto & entity : lodEntities)
    {
        uint32 count = entity->GetComponentCount(Component::LOD_COMPONENT);
        for (uint32 i = 0; i < count; ++i)
        {
            lodData[MODE_SELECTION].lodComponents.push_back(static_cast<LodComponent *> (entity->GetComponent(Component::LOD_COMPONENT, i)));
        }
    }

    lodData[MODE_SELECTION].SummarizeValues();
    if (mode == MODE_SELECTION)
    {
        lodData[MODE_SELECTION].ApplyForce(forceValues);

        EmitUpdateModeUI();
        EmitUpdateForceUI();
        EmitUpdateDistanceUI();
        EmitUpdateActionsUI();
    }
}

void EditorLODSystemV2::SetDelegate(EditorLODSystemV2UIDelegate *uiDelegate_)
{
    uiDelegate = uiDelegate_;
    if (uiDelegate != nullptr)
    {
        EmitUpdateModeUI();
        EmitUpdateForceUI();
        EmitUpdateDistanceUI();
        EmitUpdateActionsUI();
    }
}

void EditorLODSystemV2::DispatchSignals()
{
    if (uiDelegate == nullptr)
    {
        return;
    }

    if (updateModeUI)
    {
        updateModeUI = false;
        uiDelegate->UpdateModeUI(this, mode);
    }

    if (updateForceUI)
    {
        updateForceUI = false;
        uiDelegate->UpdateForceUI(this, forceValues);
    }

    if (updateDistanceUI)
    {
        updateDistanceUI = false;
        uiDelegate->UpdateDistanceUI(this, activeLodData);
    }

    if (updateActionUI)
    {
        updateActionUI = false;
        uiDelegate->UpdateActionUI(this);
    }
}

void EditorLODSystemV2::ProcessCommand(const Command2 *command, bool redo)
{
    if (generateCommands)
    {
        return;
    }

    //this code need to be refactored after commads-notofications-refactoring will be merged

    int32 commandID = command->GetId();
    switch (commandID)
    {
    case CMDID_LOD_DISTANCE_CHANGE:
    {
        for (uint32 m = 0; m < MODE_COUNT; ++m)
        {
            lodData[m].SummarizeValues();
        }

        EmitUpdateDistanceUI();
        break;
    }

    case CMDID_DELETE_RENDER_BATCH: //could changed count of lods
    case CMDID_CLONE_LAST_BATCH: //could changed count of lods
    case CMDID_LOD_CREATE_PLANE:
    case CMDID_LOD_COPY_LAST_LOD:
    case CMDID_LOD_DELETE:
    {
        for (uint32 m = 0; m < MODE_COUNT; ++m)
        {
            lodData[m].SummarizeValues();
        }
        activeLodData->ApplyForce(forceValues);

        EmitUpdateModeUI();
        EmitUpdateForceUI();
        EmitUpdateDistanceUI();
        EmitUpdateActionsUI();
        break;
    }

    default:
        break;
    }
}

