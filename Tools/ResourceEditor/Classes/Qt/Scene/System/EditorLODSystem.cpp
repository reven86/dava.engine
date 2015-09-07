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


#include "DAVAEngine.h"
#include "EditorLODSystem.h"
#include "Scene/EntityGroup.h"
#include "Entity/SceneSystem.h"
#include "Scene/SceneSignals.h"
#include "Commands2/ChangeLODDistanceCommand.h"
#include "Commands2/CreatePlaneLODCommand.h"
#include "Commands2/DeleteLODCommand.h"
#include "Commands2/CopyLastLODCommand.h"

EditorLODSystem::ForceData::ForceData(DAVA::int32 newForceLayer /* = -1 */, DAVA::float32 newDistance /* = -1 */)
    : forceLayer(newForceLayer)
    , forceDistance(newDistance)
{
}

EditorLODSystem::EditorLODSystem(DAVA::Scene *scene) 
	: DAVA::SceneSystem(scene)
{
}

EditorLODSystem::~EditorLODSystem()
{
}

void EditorLODSystem::AddEntity(DAVA::Entity * entity)
{
    DVASSERT(entity);
    DAVA::LodComponent *tmpComponent = GetLodComponent(entity);
    DVASSERT(tmpComponent);

    sceneLODs.insert(std::make_pair(tmpComponent, ForceData()));
}

void EditorLODSystem::RemoveEntity(DAVA::Entity * entity)
{
    DVASSERT(entity);
    DAVA::LodComponent *tmpComponent = GetLodComponent(entity);
    DVASSERT(tmpComponent);

    sceneLODs.erase(tmpComponent);
}

void EditorLODSystem::AddSelectedLODsRecursive(DAVA::Entity *entity)
{
    DVASSERT(entity);
    DAVA::LodComponent *tmpComponent = GetLodComponent(entity);
    if (tmpComponent)
    {
        selectedLODs.push_back(tmpComponent);
    }
    if (entity->GetSolid() || !SettingsManager::GetValue(Settings::Scene_RefreshLodForNonSolid).AsBool())
    {

        DAVA::int32 count = entity->GetChildrenCount();
        for (DAVA::int32 i = 0; i < count; ++i)
        {
            AddSelectedLODsRecursive(entity->GetChild(i));
        }
    }
}

void EditorLODSystem::RemoveSelectedLODsRecursive(DAVA::Entity *entity)
{
    DVASSERT(entity);
    DAVA::LodComponent *tmpComponent = GetLodComponent(entity);
    if (tmpComponent)
    {
        selectedLODs.remove(tmpComponent);
    }
    DAVA::int32 count = entity->GetChildrenCount();
    for (DAVA::int32 i = 0; i < count; ++i)
    {
        RemoveSelectedLODsRecursive(entity->GetChild(i));
    }
}

void EditorLODSystem::UpdateDistances(const DAVA::Map<DAVA::uint32, DAVA::float32> & newDistances)
{
    if (GetCurrentLODs().empty() || newDistances.empty())
    {
        return;
    }

    for (auto &newDistance : newDistances)
    {
        SetLayerDistance(newDistance.first, newDistance.second);
    }
}

void EditorLODSystem::SceneSelectionChanged(const EntityGroup *selected, const EntityGroup *deselected)
{
    if (!allSceneModeEnabled)
    {
        size_t deselectedSize = deselected->Size();
        for (size_t i = 0; i < deselectedSize; ++i)
        {
            ResetForceState(deselected->GetEntity(i));
        }
    }
    selectedLODs.clear();
    size_t selectedSize = selected->Size();

    if (selectedSize == 0)
    {
        forceDistance = LodComponent::INVALID_DISTANCE;
        forceLayer = LodComponent::INVALID_LOD_LAYER;
    }

    for (size_t i = 0; i < selectedSize; ++i)
    {
        AddSelectedLODsRecursive(selected->GetEntity(i));
    }

    if (allSceneModeEnabled)
    {
        return;
    }

    CollectLODDataFromScene();
    UpdateForceData();
}

void EditorLODSystem::ResetForceState(DAVA::Entity *entity)
{
    DVASSERT(entity);
    DAVA::LodComponent *tmpComponent = GetLodComponent(entity);
    if (tmpComponent)
    {
        ResetForceState(tmpComponent);
    }
    DAVA::int32 count = entity->GetChildrenCount();
    for (DAVA::int32 i = 0; i < count; ++i)
    {
        ResetForceState(entity->GetChild(i));
    }
}

void EditorLODSystem::ResetForceState(DAVA::LodComponent *lodComponent)
{
    DVASSERT(lodComponent);
    lodComponent->SetForceDistance(DAVA::LodComponent::INVALID_DISTANCE);
    lodComponent->SetForceLodLayer(DAVA::LodComponent::INVALID_LOD_LAYER);
    lodComponent->currentLod = DAVA::LodComponent::INVALID_LOD_LAYER;
}

void EditorLODSystem::CollectLODDataFromScene()
{
    currentLodsLayersCount = 0;
    std::fill(lodDistances.begin(), lodDistances.end(), 0.0f);
    std::fill(lodTrianglesCount.begin(), lodTrianglesCount.end(), 0);
    std::array<DAVA::int32, DAVA::LodComponent::MAX_LOD_LAYERS> lodsComponentsCount = { 0 };

    auto lods = GetCurrentLODs();
    for (auto &lod : lods)
    {
        DAVA::int32 layersCount = GetLodLayersCount(lod);
        DVASSERT(layersCount <= DAVA::LodComponent::MAX_LOD_LAYERS);
        for (auto layer = 0; layer < layersCount; ++layer)
        {
            lodDistances[layer] += lod->GetLodLayerDistance(layer);
            lodsComponentsCount[layer]++;
        }
        //triangles
        AddTrianglesInfo(lodTrianglesCount, lod, false);
    }
    //distances
    for (auto i = 0; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
    {
        if(0 != lodsComponentsCount[i])
        {
            lodDistances[i] /= lodsComponentsCount[i];
            ++currentLodsLayersCount;
        }
    }

    if (!SettingsManager::GetValue(Settings::Scene_RememberForceParameters).AsBool()
        && !forceDistanceEnabled
        && forceLayer >= currentLodsLayersCount)
    {
        SetForceLayer(currentLodsLayersCount - 1);
        return;
    }
}

void EditorLODSystem::AddTrianglesInfo(std::array<DAVA::uint32, DAVA::LodComponent::MAX_LOD_LAYERS> &triangles, DAVA::LodComponent *lod, bool onlyVisibleBatches)
{
    Entity * en = lod->GetEntity();
    if (nullptr != GetEffectComponent(en))
    {
        return;
    }

    RenderObject * ro = GetRenderObject(en);
    if (nullptr == ro)
    {
        return;
    }

    DAVA::uint32 batchCount = ro->GetRenderBatchCount();
    for (DAVA::uint32 i = 0; i < batchCount; ++i)
    {
        DAVA::int32 lodIndex = 0;
        DAVA::int32 switchIndex = 0;

        RenderBatch *rb = ro->GetRenderBatch(i, lodIndex, switchIndex);
        if (lodIndex < 0 || lodIndex >= DAVA::LodComponent::MAX_LOD_LAYERS)
        {
            Logger::Error("got unexpected lod index (%d) when collecting triangles on entitie %s. Correct values for lod index is %d", lodIndex, en->GetName().c_str(), DAVA::LodComponent::MAX_LOD_LAYERS);
            continue;
        }
    
        if(IsPointerToExactClass<RenderBatch>(rb))
        {
            if(onlyVisibleBatches)
            { //check batch visibility

                bool batchIsVisible = false;
                DAVA::uint32 activeBatchCount = ro->GetActiveRenderBatchCount();
                for (DAVA::uint32 a = 0; a < activeBatchCount && !batchIsVisible; ++a)
                {
                    RenderBatch *visibleBatch = ro->GetActiveRenderBatch(a);
                    batchIsVisible = (visibleBatch == rb);
                }

                if (batchIsVisible == false) // need to skip this render batch
                {
                    continue;
                }
            }

            PolygonGroup *pg = rb->GetPolygonGroup();
            if(nullptr != pg)
            {
                DVASSERT(lodIndex < DAVA::LodComponent::MAX_LOD_LAYERS);
                DVASSERT(lodIndex >= 0);
                triangles[lodIndex] += pg->GetIndexCount() / 3; 
            }
        }
    }
}

bool EditorLODSystem::CheckSelectedContainsEntity(const DAVA::Entity *arg) const
{
    DVASSERT(arg);
    const EntityGroup &selection = static_cast<SceneEditor2*>(GetScene())->selectionSystem->GetSelection();
    for (size_t i = 0;  i < selection.Size();  ++i)
    {
        if (selection.GetEntity(i) == arg)
        {
            return true;
        }
    }
    return false;
}

void EditorLODSystem::SolidChanged(const Entity *entity, bool value)
{
    DVASSERT(entity);
    if (!CheckSelectedContainsEntity(entity))
    {
        return;
    }

    if (value)
    {
        DAVA::int32 count = entity->GetChildrenCount();
        for (DAVA::int32 i = 0; i < count; ++i)
        {
            AddSelectedLODsRecursive(entity->GetChild(i));
        }
    }
    else
    {
        DAVA::int32 count = entity->GetChildrenCount();
        for (DAVA::int32 i = 0; i < count; ++i)
        {
            RemoveSelectedLODsRecursive(entity->GetChild(i));
        }
    }
    CollectLODDataFromScene();
}


bool EditorLODSystem::CanCreatePlaneLOD() const
{
    if (1 != GetCurrentLODs().size())
    {
        return false;
    }

    auto lods = GetCurrentLODs();
    for (auto &lod : lods)
    {
        if (lod->GetEntity()->GetComponent(Component::PARTICLE_EFFECT_COMPONENT))
        {
            return false;
        }
    }

    return (GetLodLayersCount(GetCurrentLODs().front()->GetEntity()) < LodComponent::MAX_LOD_LAYERS);
}

bool EditorLODSystem::CreatePlaneLOD(DAVA::int32 fromLayer, DAVA::uint32 textureSize, const DAVA::FilePath & texturePath)
{
    if (GetCurrentLODs().empty())
	{
        return false;
	}

    SceneEditor2* sceneEditor2 = static_cast<SceneEditor2*>(GetScene());

    auto lods = GetCurrentLODs();
    for (auto& lod : lods)
	{
		auto request = CreatePlaneLODCommandHelper::RequestRenderToTexture(lod, fromLayer, textureSize, texturePath);
		planeLODRequests.push_back(request);
	}

    return true;
}

void EditorLODSystem::Process(DAVA::float32 elapsedTime)
{
	bool allRequestsProcessed = !planeLODRequests.empty();

	for (const auto& req : planeLODRequests)
		allRequestsProcessed = allRequestsProcessed && req->completed;

	if (allRequestsProcessed)
	{
		SceneEditor2* sceneEditor2 = static_cast<SceneEditor2*>(GetScene());
		sceneEditor2->BeginBatch("LOD Added");
		for (const auto& req : planeLODRequests)
		{
			sceneEditor2->Exec(new CreatePlaneLODCommand(req));
		}
		sceneEditor2->EndBatch();

		planeLODRequests.clear();
	}
}

bool EditorLODSystem::CopyLastLodToLod0()
{
    if (GetCurrentLODs().empty())
    {
        return false;
    }    
    SceneEditor2* sceneEditor2 = static_cast<SceneEditor2*>(GetScene());

    sceneEditor2->BeginBatch("LOD Added");

    auto lods = GetCurrentLODs();
    for (auto &lod : lods)
    {
        sceneEditor2->Exec(new CopyLastLODToLod0Command(lod));
    }
    sceneEditor2->EndBatch();
    return true;
}

FilePath EditorLODSystem::GetDefaultTexturePathForPlaneEntity() const
{
    DVASSERT(GetCurrentLODs().size() == 1)
    Entity *entity = GetCurrentLODs().back()->GetEntity();

    FilePath entityPath = static_cast<SceneEditor2*>(GetScene())->GetScenePath();
    KeyedArchive * properties = GetCustomPropertiesArchieve(entity);
    if (nullptr != properties && properties->IsKeyExists(ResourceEditor::EDITOR_REFERENCE_TO_OWNER))
    {
        entityPath = FilePath(properties->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, entityPath.GetAbsolutePathname()));
    }
    String entityName = entity->GetName().c_str();
    FilePath textureFolder = entityPath.GetDirectory() + "images/";

    String texturePostfix = "_planes.png";
    FilePath texturePath = textureFolder + entityName + texturePostfix;
    int32 i = 0;
    while(texturePath.Exists())
    {
        i++;
        texturePath = textureFolder + Format("%s_%d%s", entityName.c_str(), i, texturePostfix.c_str());
    }

    return texturePath;
}

bool EditorLODSystem::CanDeleteLod() const
{
    if (GetCurrentLODs().empty())
    {
        return false;
    }
    bool containLayers = false;

    auto lods = GetCurrentLODs();
    for (auto &lod : lods)
    {
        if (nullptr != lod->GetEntity()->GetComponent(Component::PARTICLE_EFFECT_COMPONENT))
        {
            return false;
        }
        else if (!containLayers && GetLodLayersCount(lod) > 1)
        {
            containLayers = true;
        }
    }
    return containLayers;
}

bool EditorLODSystem::DeleteFirstLOD()
{
    if (false == CanDeleteLod())
    {
        return false;
    }
    SceneEditor2* sceneEditor2 = static_cast<SceneEditor2*>(GetScene());
    sceneEditor2->BeginBatch("Delete First LOD");

    auto lods = GetCurrentLODs();
    for (auto &lod : lods)
    {
        if (GetLodLayersCount(lod) > 1)
        {
            sceneEditor2->Exec(new DeleteLODCommand(lod, 0, -1));
        }
    }
    sceneEditor2->EndBatch();
    return true;
}

bool EditorLODSystem::DeleteLastLOD()
{
    if (false == CanDeleteLod())
    {
        return false;
    }
    SceneEditor2* sceneEditor2 = static_cast<SceneEditor2*>(GetScene());
    sceneEditor2->BeginBatch("Delete Last LOD");

    auto lods = GetCurrentLODs();
    for (auto &lod : lods)
    {
        if (GetLodLayersCount(lod) > 1)
        {
            sceneEditor2->Exec(new DeleteLODCommand(lod, GetLodLayersCount(lod) - 1, -1));
        }
    }
    sceneEditor2->EndBatch();
    return true;
}

void EditorLODSystem::SetLayerDistance(DAVA::int32 layerNum, DAVA::float32 distance)
{
    DVASSERT(layerNum < currentLodsLayersCount);
    lodDistances[layerNum] = distance;
    if (GetCurrentLODs().empty())
    {
        return;
    }
    SceneEditor2* sceneEditor2 = static_cast<SceneEditor2*>(GetScene());
    sceneEditor2->BeginBatch("LOD Distance Changed");
    
    auto lods = GetCurrentLODs();
    for (auto &lod : lods)
    {
        sceneEditor2->Exec(new ChangeLODDistanceCommand(lod, layerNum, distance));
    }
    sceneEditor2->EndBatch();
    CollectLODDataFromScene();
}

void EditorLODSystem::SetForceDistanceEnabled(bool enable)
{
    if (forceDistanceEnabled == enable)
    {
        return;
    }
    forceDistanceEnabled = enable;
    UpdateForceData();
}

void EditorLODSystem::UpdateForceData()
{
    if (forceDistanceEnabled)
    {
        UpdateForceDistance();
    }
    else
    {
        UpdateForceLayer();
    }
}

void EditorLODSystem::SetForceDistance(DAVA::float32 distance)
{
    if (allSceneModeEnabled)
    {
        allSceneForceDistance = distance;
    }
    forceDistance = distance;

    auto lods = GetCurrentLODs();
    for (auto &lod : lods)
    {
        lod->SetForceLodLayer(LodComponent::INVALID_LOD_LAYER);
        lod->currentLod = DAVA::LodComponent::INVALID_LOD_LAYER;
        lod->SetForceDistance(distance);

        if (!allSceneModeEnabled && SettingsManager::GetValue(Settings::Scene_RememberForceParameters).AsBool())
        {
            ForceData force = sceneLODs[lod];
            force.forceDistance = distance;
            sceneLODs[lod] = force;
        }
    }
}

DAVA::float32 EditorLODSystem::GetCurrentDistance() const
{
    if (!SettingsManager::GetValue(Settings::Scene_RememberForceParameters).AsBool())
    {
        return forceDistance;
    }
    else if (GetCurrentLODs().empty())
    {
        return DAVA::LodComponent::MIN_LOD_DISTANCE;
    }
    else if (allSceneModeEnabled)
    {
        return allSceneForceDistance;
    }
    else
    {
        return CalculateForceDistance();
    }
}

void EditorLODSystem::UpdateForceDistance()
{
    DAVA::Map<int, int> m;

    if (!SettingsManager::GetValue(Settings::Scene_RememberForceParameters).AsBool())
    {
        auto lods = GetCurrentLODs();
        for (auto &lod : lods)
        {
            lod->SetForceLodLayer(LodComponent::INVALID_LOD_LAYER);
            lod->currentLod = DAVA::LodComponent::INVALID_LOD_LAYER;
            lod->SetForceDistance(forceDistance);
        }
        return;
    }

    if (allSceneModeEnabled)
    {
        return;
    }

    auto lods = GetCurrentLODs();
    for (auto &lod : lods)
    {
        lod->SetForceLodLayer(LodComponent::INVALID_LOD_LAYER);
        lod->currentLod = DAVA::LodComponent::INVALID_LOD_LAYER;
        lod->SetForceDistance(sceneLODs[lod].forceDistance);
    }
}

void EditorLODSystem::SetForceLayer(DAVA::int32 layer)
{
    if (allSceneModeEnabled)
    {
        allSceneForceLayer = layer;
    }
    forceLayer = layer;

    auto lods = GetCurrentLODs();
    for (auto &lod : lods)
    {
        lod->SetForceDistance(DAVA::LodComponent::INVALID_DISTANCE);
        lod->currentLod = DAVA::LodComponent::INVALID_LOD_LAYER;
        lod->SetForceLodLayer(layer);

        if (!allSceneModeEnabled && SettingsManager::GetValue(Settings::Scene_RememberForceParameters).AsBool())
        {
            ForceData force = sceneLODs[lod];
            force.forceLayer = layer;
            sceneLODs[lod] = force;
        }
    }
}

DAVA::int32 EditorLODSystem::GetCurrentForceLayer() const
{
    if (!SettingsManager::GetValue(Settings::Scene_RememberForceParameters).AsBool())
    {
        return forceLayer;
    }
    else if (GetCurrentLODs().empty())
    {
        return DAVA::LodComponent::INVALID_LOD_LAYER;
    }
    else if (allSceneModeEnabled)
    {
        return allSceneForceLayer;
    }
    else
    {
        return CalculateForceLayer();
    }
}

void EditorLODSystem::UpdateForceLayer()
{
    if (!SettingsManager::GetValue(Settings::Scene_RememberForceParameters).AsBool())
    {
        auto lods = GetCurrentLODs();
        for (auto &lod : lods)
        {
            lod->SetForceDistance(DAVA::LodComponent::INVALID_DISTANCE);
            lod->currentLod = DAVA::LodComponent::INVALID_LOD_LAYER;
            lod->SetForceLodLayer(forceLayer);
        }
        return;
    }

    if (allSceneModeEnabled)
    {
        return;
    }

    auto lods = GetCurrentLODs();
    for (auto &lod : lods)
    {
        lod->SetForceDistance(DAVA::LodComponent::INVALID_DISTANCE);
        lod->currentLod = DAVA::LodComponent::INVALID_LOD_LAYER;
        lod->SetForceLodLayer(sceneLODs[lod].forceLayer);
    }
}

void EditorLODSystem::SetAllSceneModeEnabled(bool enabled)
{
    if (allSceneModeEnabled == enabled)
    {
        return;
    }
    allSceneModeEnabled = enabled;
    if (!allSceneModeEnabled)
    {
        allSceneForceLayer = DAVA::LodComponent::MAX_LOD_LAYERS;
        allSceneForceDistance = DAVA::LodComponent::INVALID_DISTANCE;
    }
    UpdateAllSceneModeEnabled();
}

void EditorLODSystem::UpdateAllSceneModeEnabled()
{
    DAVA::List<DAVA::LodComponent *> lods;
    for (auto it = sceneLODs.begin(); it != sceneLODs.end(); ++it)
    {
        lods.push_back(it->first);
    }

    for (auto lod : lods)
    {
        ResetForceState(lod);
    }
    CollectLODDataFromScene();
    UpdateForceData();
}

DAVA::int32 EditorLODSystem::CalculateForceLayer() const
{
    auto lods = GetCurrentLODs();
    DAVA::int32 compareLayer = sceneLODs.at(*(lods.begin())).forceLayer;
    for (auto &lod : lods)
    {
        if (sceneLODs.at(lod).forceLayer != compareLayer)
        {
            return DAVA::LodComponent::MAX_LOD_LAYERS;
        }
    }

    return compareLayer;
}

DAVA::float32 EditorLODSystem::CalculateForceDistance() const
{
    auto lods = GetCurrentLODs();
    DAVA::int32 compareDistance = sceneLODs.at(*(lods.begin())).forceDistance;
    for (auto &lod : lods)
    {
        if (sceneLODs.at(lod).forceDistance != compareDistance)
        {
            return DAVA::LodComponent::INVALID_DISTANCE;
        }
    }

    return compareDistance;
}
