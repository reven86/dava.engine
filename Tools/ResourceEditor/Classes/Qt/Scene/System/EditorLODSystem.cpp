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
#include "Commands2/CopyLastLODCommand.h"
#include "Commands2/CreatePlaneLODCommand.h"
#include "Commands2/RemoveComponentCommand.h"

#include "Main/Guards.h"

#include "Scene/EntityGroup.h"
#include "Scene/SceneEditor2.h"
#include "Scene/System/EditorLODSystem.h"
#include "Scene/System/SelectionSystem.h"

using namespace DAVA;

void LODComponentHolder::BindToSystem(EditorLODSystem* system_, SceneEditor2* scene_)
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

    uint32 count = static_cast<uint32>(lodComponents.size());
    if (count > 0)
    {
        for (auto& lc : lodComponents)
        {
            maxLodLayerIndex = Max(maxLodLayerIndex, static_cast<int32>(GetLodLayersCount(lc)) - 1);

            for (uint32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
            {
                lodDistances[i] += lc->GetLodLayerDistance(i);
            }
        }

        for (uint32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
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
    for (auto& lc : lodComponents)
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
    for (auto& lc : lodComponents)
    {
        if ((GetLodLayersCount(lc) > 0) && (HasComponent(lc->GetEntity(), Component::PARTICLE_EFFECT_COMPONENT) == false))
        {
            scene->Exec(new DeleteLODCommand(lc, layer, -1));
            wasLayerRemoved = true;
        }
    }
    scene->EndBatch();

    return wasLayerRemoved;
}

bool LODComponentHolder::CopyLod(int32 from, int32 to)
{
    bool wasCopiedRemoved = false;

    scene->BeginBatch(Format("Copy lod layer %d to %d", from, to));
    for (auto& lc : lodComponents)
    {
        Entity* entity = lc->GetEntity();
        if (HasComponent(entity, Component::PARTICLE_EFFECT_COMPONENT))
        {
            continue;
        }

        if (GetLodLayersCount(entity) < LodComponent::MAX_LOD_LAYERS)
        {
            scene->Exec(new CopyLastLODToLod0Command(lc));
            wasCopiedRemoved = true;
        }
    }
    scene->EndBatch();
    return wasCopiedRemoved;
}

void LODComponentHolder::ApplyForce(const ForceValues& force)
{
    for (auto& lc : lodComponents)
    {
        if (force.flag & ForceValues::APPLY_LAYER)
        {
            lc->currentLod = LodComponent::INVALID_LOD_LAYER;
            lc->SetForceLodLayer(force.layer);
        }

        if (force.flag & ForceValues::APPLY_DISTANCE)
        {
            lc->currentLod = LodComponent::INVALID_LOD_LAYER;
            lc->SetForceDistance(force.distance);
        }
    }
}

int32 LODComponentHolder::GetMaxLODLayer() const
{
    return maxLodLayerIndex;
}

uint32 LODComponentHolder::GetLODLayersCount() const
{
    return (maxLodLayerIndex + 1);
}

const LodComponent& LODComponentHolder::GetLODComponent() const
{
    return mergedComponent;
}

//SYSTEM

EditorLODSystem::EditorLODSystem(Scene* scene)
    : SceneSystem(scene)
{
    for (uint32 m = 0; m < eEditorMode::MODE_COUNT; ++m)
    {
        lodData[m].BindToSystem(this, static_cast<SceneEditor2*>(GetScene()));
    }

    const bool allSceneModeEnabled = SettingsManager::GetValue(Settings::Internal_LODEditorMode).AsBool();
    mode = (allSceneModeEnabled) ? eEditorMode::MODE_ALL_SCENE : eEditorMode::MODE_SELECTION;

    activeLodData = &lodData[mode];
}

EditorLODSystem::~EditorLODSystem()
{
    activeLodData = nullptr;
}

void EditorLODSystem::Process(float32 timeElapsed)
{
    DispatchSignals();
    ProcessPlaneLODs();
}

void EditorLODSystem::AddEntity(Entity* entity)
{
    LodComponent* lc = GetLodComponent(entity);
    DVASSERT(lc != nullptr);

    AddComponent(entity, lc);
}

void EditorLODSystem::RemoveEntity(Entity* entity)
{
    LodComponent* lc = GetLodComponent(entity);
    DVASSERT(lc != nullptr);

    RemoveComponent(entity, lc);
}

void EditorLODSystem::AddComponent(Entity* entity, Component* component)
{
    DVASSERT(component->GetType() == Component::LOD_COMPONENT);

    lodData[eEditorMode::MODE_ALL_SCENE].lodComponents.push_back(static_cast<LodComponent*>(component));
    lodData[eEditorMode::MODE_ALL_SCENE].SummarizeValues();

    if (mode == eEditorMode::MODE_ALL_SCENE)
    {
        EmitInvalidateUI(FLAG_ALL);
    }
}

void EditorLODSystem::RemoveComponent(Entity* entity, Component* component)
{
    DVASSERT(component->GetType() == Component::LOD_COMPONENT);

    LodComponent* removedComponent = static_cast<LodComponent*>(component);
    for (uint32 m = 0; m < eEditorMode::MODE_COUNT; ++m)
    {
        bool removed = FindAndRemoveExchangingWithLast(lodData[m].lodComponents, removedComponent);
        if (removed)
        {
            lodData[m].SummarizeValues();
            if (m == mode)
            {
                EmitInvalidateUI(FLAG_ALL);
            }
        }
    }
}

void EditorLODSystem::SceneDidLoaded()
{
    lodData[eEditorMode::MODE_ALL_SCENE].SummarizeValues();
    if (mode == eEditorMode::MODE_ALL_SCENE)
    {
        EmitInvalidateUI(FLAG_ALL);
    }
}

eEditorMode EditorLODSystem::GetMode() const
{
    return mode;
}

void EditorLODSystem::SetMode(eEditorMode mode_)
{
    DVASSERT(activeLodData != nullptr);

    activeLodData->ApplyForce({ -1, -1, ForceValues::APPLY_ALL });
    mode = mode_;
    activeLodData = &lodData[mode];
    activeLodData->ApplyForce(forceValues);

    EmitInvalidateUI(FLAG_ALL);
}

const ForceValues& EditorLODSystem::GetForceValues() const
{
    return forceValues;
}

void EditorLODSystem::SetForceValues(const ForceValues& values)
{
    DVASSERT(activeLodData != nullptr);

    ForceValues distanceDiffValues;
    distanceDiffValues.flag = ForceValues::APPLY_ALL;

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

    EmitInvalidateUI(FLAG_FORCE);
}

bool EditorLODSystem::CanDeleteLOD() const
{
    DVASSERT(activeLodData != nullptr);

    bool canDeleteLod = (!activeLodData->lodComponents.empty()) && (activeLodData->GetLODLayersCount() > 1);
    for (auto& lc : activeLodData->lodComponents)
    {
        if (HasComponent(lc->GetEntity(), Component::PARTICLE_EFFECT_COMPONENT))
        {
            canDeleteLod = false;
            break;
        }
    }

    return canDeleteLod;
}

bool EditorLODSystem::CanCreateLOD() const
{
    DVASSERT(activeLodData != nullptr);
    if (activeLodData->lodComponents.size() == 1) // we can create lod only for one entity
    {
        bool canCreateLod = (activeLodData->GetLODLayersCount() < LodComponent::MAX_LOD_LAYERS);
        if (canCreateLod)
        {
            Entity* entity = activeLodData->lodComponents[0]->GetEntity();
            if (HasComponent(entity, Component::PARTICLE_EFFECT_COMPONENT))
            {
                return false;
            }

            return HasComponent(entity, Component::RENDER_COMPONENT);
        }
    }

    return false;
}

void EditorLODSystem::CreatePlaneLOD(int32 fromLayer, uint32 textureSize, const FilePath& texturePath)
{
    DVASSERT(activeLodData != nullptr);

    planeLODRequests.reserve(activeLodData->lodComponents.size());
    for (auto& lc : activeLodData->lodComponents)
    {
        auto request = CreatePlaneLODCommandHelper::RequestRenderToTexture(lc, fromLayer, textureSize, texturePath);
        planeLODRequests.push_back(request);
    }
}

void EditorLODSystem::DeleteFirstLOD()
{
    DVASSERT(activeLodData != nullptr);
    DeleteLOD(0);
}

void EditorLODSystem::DeleteLastLOD()
{
    DVASSERT(activeLodData != nullptr);
    DeleteLOD(activeLodData->GetMaxLODLayer());
}

void EditorLODSystem::DeleteLOD(DAVA::int32 layer)
{
    if (activeLodData->GetLODLayersCount() > 0)
    {
        Guard::ScopedBoolGuard guard(generateCommands, true);
        bool deleted = activeLodData->DeleteLOD(layer);
        if (deleted)
        {
            RecalculateData();
            EmitInvalidateUI(FLAG_ALL);
        }
    }
}

void EditorLODSystem::CopyLastLODToFirst()
{
    DVASSERT(activeLodData != nullptr);

    Guard::ScopedBoolGuard guard(generateCommands, true);
    bool copied = activeLodData->CopyLod(activeLodData->GetMaxLODLayer(), 0);
    if (copied)
    {
        RecalculateData();
        EmitInvalidateUI(FLAG_FORCE | FLAG_DISTANCE | FLAG_ACTION);
    }
}

const LODComponentHolder* EditorLODSystem::GetActiveLODData() const
{
    return activeLodData;
}

void EditorLODSystem::SetLODDistances(const Array<float32, LodComponent::MAX_LOD_LAYERS>& distances)
{
    DVASSERT(activeLodData != nullptr);

    for (int32 i = 0; i < static_cast<int32>(distances.size()); ++i)
    {
        activeLodData->mergedComponent.SetLodLayerDistance(i, distances[i]);
    }

    Guard::ScopedBoolGuard guard(generateCommands, true);
    activeLodData->PropagateValues();

    RecalculateData();
    EmitInvalidateUI(FLAG_DISTANCE);
}

void EditorLODSystem::SolidChanged(const Entity* entity, bool value)
{
    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    EntityGroup selection(sceneEditor->selectionSystem->GetSelection().CopyContentToVector());

    if (selection.ContainsEntity(const_cast<Entity*>(entity)) == false)
    {
        return;
    }

    SelectionChanged(&selection, nullptr);
}

void EditorLODSystem::SelectionChanged(const EntityGroup* selected, const EntityGroup* deselected)
{
    lodData[eEditorMode::MODE_SELECTION].lodComponents.clear();

    bool ignoreChildren = SettingsManager::GetValue(Settings::Scene_RefreshLodForNonSolid).AsBool();

    uint32 count = selected->Size();
    Vector<Entity*> lodEntities;
    lodEntities.reserve(count); //mostly we have less than 5 lods in hierarchy

    const auto& entitiesContent = selected->GetContent();
    for (auto& it : entitiesContent)
    {
        Entity* entity = it.first;
        if (entity->GetSolid() || !ignoreChildren)
        {
            entity->GetChildEntitiesWithComponent(lodEntities, Component::LOD_COMPONENT);
        }

        if (entity->GetComponentCount(Component::LOD_COMPONENT) > 0)
        {
            lodEntities.push_back(entity);
        }
    }

    for (auto& entity : lodEntities)
    {
        uint32 count = entity->GetComponentCount(Component::LOD_COMPONENT);
        for (uint32 i = 0; i < count; ++i)
        {
            lodData[eEditorMode::MODE_SELECTION].lodComponents.push_back(static_cast<LodComponent*>(entity->GetComponent(Component::LOD_COMPONENT, i)));
        }
    }

    lodData[eEditorMode::MODE_SELECTION].SummarizeValues();
    if (mode == eEditorMode::MODE_SELECTION)
    {
        lodData[eEditorMode::MODE_SELECTION].ApplyForce(forceValues);

        EmitInvalidateUI(FLAG_ALL);
    }
}

void EditorLODSystem::AddDelegate(EditorLODSystemUIDelegate* uiDelegate)
{
    DVASSERT(uiDelegate != nullptr);

    uiDelegates.push_back(uiDelegate);
    if (uiDelegate != nullptr)
    {
        uiDelegate->UpdateModeUI(this, mode);
        uiDelegate->UpdateForceUI(this, forceValues);
        uiDelegate->UpdateDistanceUI(this, activeLodData);
        uiDelegate->UpdateActionUI(this);
    }
}

void EditorLODSystem::RemoveDelegate(EditorLODSystemUIDelegate* uiDelegate)
{
    DVASSERT(uiDelegate != nullptr);

    FindAndRemoveExchangingWithLast(uiDelegates, uiDelegate);
}

void EditorLODSystem::EmitInvalidateUI(uint32 flags)
{
    invalidateUIFlag |= flags;
}

void EditorLODSystem::DispatchSignals()
{
    if (invalidateUIFlag == FLAG_NONE)
    {
        return;
    }

    for (auto& d : uiDelegates)
    {
        if (invalidateUIFlag & FLAG_MODE)
        {
            d->UpdateModeUI(this, mode);
        }

        if (invalidateUIFlag & FLAG_FORCE)
        {
            d->UpdateForceUI(this, forceValues);
        }

        if (invalidateUIFlag & FLAG_DISTANCE)
        {
            d->UpdateDistanceUI(this, activeLodData);
        }

        if (invalidateUIFlag & FLAG_ACTION)
        {
            d->UpdateActionUI(this);
        }
    }

    invalidateUIFlag = FLAG_NONE;
}

void EditorLODSystem::ProcessCommand(const Command2* command, bool redo)
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
        RecalculateData();
        EmitInvalidateUI(FLAG_DISTANCE);
        break;
    }

    case CMDID_COMPONENT_REMOVE:
    {
        const RemoveComponentCommand* removeCommand = static_cast<const RemoveComponentCommand*>(command);
        if (removeCommand->GetComponent()->GetType() == Component::RENDER_COMPONENT)
        {
            RecalculateData();
            activeLodData->ApplyForce(forceValues);

            EmitInvalidateUI(FLAG_ALL);
        }

        break;
    }

    case CMDID_DELETE_RENDER_BATCH: //could changed count of lods
    case CMDID_CLONE_LAST_BATCH: //could changed count of lods
    case CMDID_LOD_CREATE_PLANE:
    case CMDID_LOD_COPY_LAST_LOD:
    case CMDID_LOD_DELETE:
    {
        RecalculateData();
        activeLodData->ApplyForce(forceValues);

        EmitInvalidateUI(FLAG_ALL);
        break;
    }

    default:
        break;
    }
}

FilePath EditorLODSystem::GetPathForPlaneEntity() const
{
    DVASSERT(activeLodData != nullptr);
    DVASSERT(!activeLodData->lodComponents.empty());

    SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());
    Entity* entity = activeLodData->lodComponents.front()->GetEntity();

    FilePath entityPath = editorScene->GetScenePath();
    KeyedArchive* properties = GetCustomPropertiesArchieve(entity);
    if (nullptr != properties && properties->IsKeyExists(ResourceEditor::EDITOR_REFERENCE_TO_OWNER))
    {
        entityPath = FilePath(properties->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, entityPath.GetAbsolutePathname()));
    }
    String entityName = entity->GetName().c_str();
    FilePath textureFolder = entityPath.GetDirectory() + "images/";

    String texturePostfix = "_planes.png";
    FilePath texturePath = textureFolder + entityName + texturePostfix;
    int32 i = 0;
    while (FileSystem::Instance()->Exists(texturePath))
    {
        i++;
        texturePath = textureFolder + Format("%s_%d%s", entityName.c_str(), i, texturePostfix.c_str());
    }

    return texturePath;
}

void EditorLODSystem::ProcessPlaneLODs()
{
    if (planeLODRequests.empty())
    {
        return;
    }

    bool allRequestsProcessed = true;
    for (const auto& req : planeLODRequests)
    {
        allRequestsProcessed = allRequestsProcessed && req->completed;
    }

    if (allRequestsProcessed)
    {
        Guard::ScopedBoolGuard guard(generateCommands, true);

        SceneEditor2* sceneEditor2 = static_cast<SceneEditor2*>(GetScene());
        sceneEditor2->BeginBatch("Create plane lods");
        for (const auto& req : planeLODRequests)
        {
            sceneEditor2->Exec(new CreatePlaneLODCommand(req));
        }
        sceneEditor2->EndBatch();
        planeLODRequests.clear();

        RecalculateData();
        EmitInvalidateUI(FLAG_FORCE | FLAG_DISTANCE | FLAG_ACTION);
    }
}

void EditorLODSystem::RecalculateData()
{
    for (uint32 m = 0; m < eEditorMode::MODE_COUNT; ++m)
    {
        lodData[m].SummarizeValues();
    }
}
