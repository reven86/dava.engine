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



#include "EditorLODData.h"

#include "Scene/SceneSignals.h"
#include "Commands2/ChangeLODDistanceCommand.h"
#include "Commands2/CreatePlaneLODCommand.h"
#include "Commands2/DeleteLODCommand.h"
#include "Commands2/CopyLastLODCommand.h"
#include "Commands2/LodIndexChangeCommand.h"

const DAVA::uint32 EditorLODData::EDITOR_LOD_DATA_COUNT = 4;

EditorLODData::EditorLODData() :
        forceDistanceEnabled(false)
    ,   forceDistance(0.f)
    ,   forceLayer(DAVA::LodComponent::INVALID_LOD_LAYER)
    ,   activeScene(NULL)
	,	allSceneModeEnabled(false)
{
    connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), SLOT(SceneActivated(SceneEditor2 *)));
    connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), SLOT(SceneDeactivated(SceneEditor2 *)));
    connect(SceneSignals::Instance(), SIGNAL(StructureChanged(SceneEditor2 *, DAVA::Entity *)), SLOT(SceneStructureChanged(SceneEditor2 *, DAVA::Entity *)));
    connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)), SLOT(SceneSelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)));

    connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2 *, const Command2*, bool)), SLOT(CommandExecuted(SceneEditor2 *, const Command2*, bool)));
    
    lodInfo.resize(DAVA::LodComponent::MAX_LOD_LAYERS);
}

EditorLODData::~EditorLODData()
{
    if(activeScene)
    {
        ResetForceState(activeScene);
        activeScene = NULL;
    }
}


void EditorLODData::ClearLODData()
{
    ResetLODData();
    
    emit DataChanged();
}

void EditorLODData::ResetLODData()
{
    ResetLODInfo();
    
    sortedLodIndices.clear();
    
    lodData.clear();
}

void EditorLODData::ResetLODInfo()
{
    size_t lodInfoSize = lodInfo.size();
    for(size_t i = 0; i < lodInfoSize; ++i)
    {
        lodInfo[i].MakeEmpty();
    }
}

void EditorLODData::ClearForceData()
{
    forceDistance = 0.f;
    forceLayer = DAVA::LodComponent::INVALID_LOD_LAYER;
}

DAVA::uint32 EditorLODData::GetLayersCount() const
{
    return lodInfo.size();
}

DAVA::float32 EditorLODData::GetLayerDistance(DAVA::uint32 layerNum) const
{
    DVASSERT(layerNum >= 0 && layerNum < lodInfo.size());
    return lodInfo[layerNum].lodDistance;
}

void EditorLODData::SetLayerDistance(DAVA::uint32 layerNum, DAVA::float32 distance)
{
    DVASSERT(layerNum >= 0 && layerNum < lodInfo.size());
    
    lodInfo[layerNum].lodDistance = distance;
    
    DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
    if(componentsCount && activeScene)
    {
        activeScene->BeginBatch("LOD Distance Changed");
        
        for(DAVA::uint32 i = 0; i < componentsCount; ++i)
        {
            activeScene->Exec(new ChangeLODDistanceCommand(lodData[i], layerNum, distance, currentLODQuality));
        }
        
        activeScene->EndBatch();
    }
}

DAVA::int32 EditorLODData::GetLayerLodIndex(DAVA::uint32 layerNum) const
{
    DVASSERT(layerNum >= 0 && layerNum < lodInfo.size());
    return lodInfo[layerNum].lodIndex;
}

void EditorLODData::SetLayerLodIndex(DAVA::uint32 layerNum, DAVA::int32 lodIndex)
{
    DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
	if(componentsCount && activeScene)
	{
		activeScene->BeginBatch("LOD Index Changed");
        
        DVASSERT(layerNum >= 0 && layerNum < lodInfo.size());
        
        lodInfo[layerNum].lodIndex = lodIndex;
        
        for(DAVA::uint32 i = 0; i < componentsCount; ++i)
        {
            activeScene->Exec(new LodIndexChangeCommand(layerNum, lodIndex, lodData[i], currentLODQuality));
        }
		
		activeScene->EndBatch();
	}
}


void EditorLODData::UpdateDistances(const DAVA::Map<DAVA::uint32, DAVA::float32> & newDistances)
{
	DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
	if(componentsCount && activeScene && newDistances.size() != 0)
	{
		activeScene->BeginBatch("LOD Distances Changed");

		DAVA::Map<DAVA::uint32, DAVA::float32>::const_iterator endIt = newDistances.end();
		for(auto it = newDistances.begin(); it != endIt; ++it)
		{
			DAVA::uint32 layerNum = it->first;
			DAVA::float32 distance = it->second;
            
            DVASSERT(layerNum >= 0 && layerNum < lodInfo.size());
            
            lodInfo[layerNum].lodDistance = distance;
            
            for(DAVA::uint32 i = 0; i < componentsCount; ++i)
            {
                activeScene->Exec(new ChangeLODDistanceCommand(lodData[i], layerNum, distance, currentLODQuality));
            }
		}

		activeScene->EndBatch();
	}
}

DAVA::uint32 EditorLODData::GetLayerTriangles(DAVA::uint32 layerNum) const
{
    DVASSERT(layerNum >= 0 && layerNum < lodInfo.size());
    return lodInfo[layerNum].lodTriangles;
}


void EditorLODData::SceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected)
{
    if(activeScene == scene && !allSceneModeEnabled)
    {
		for(size_t i = 0; i < deselected->Size(); ++i)
		{
			ResetForceState(deselected->GetEntity(i));
		}

        UpdateLODStateFromScene();
		GetLODDataFromScene();
        UpdateForceData();
    }
}

void EditorLODData::ResetForceState(DAVA::Entity *entity)
{
    if(!entity) return;
    
    DAVA::Vector<DAVA::LodComponent *> lods;
    EnumerateLODsRecursive(entity, lods);
    
    for(DAVA::uint32 i = 0; i < (DAVA::uint32)lods.size(); ++i)
    {
        lods[i]->SetForceDistance(DAVA::LodComponent::INVALID_DISTANCE);
        lods[i]->SetForceLodLayer(DAVA::LodComponent::INVALID_LOD_LAYER);
		lods[i]->currentLod = -1;
    }
}

void EditorLODData::SetForceDistance(DAVA::float32 distance)
{
    forceDistance = distance;

    if(forceDistanceEnabled)
    {
        DAVA::uint32 count = lodData.size();
        for(DAVA::uint32 i = 0; i < count; ++i)
        {
            lodData[i]->SetForceDistance(forceDistance);
            lodData[i]->SetForceLodLayer(DAVA::LodComponent::INVALID_LOD_LAYER);
        }
    }
	else
	{
		DAVA::uint32 count = lodData.size();
		for(DAVA::uint32 i = 0; i < count; ++i)
		{
			lodData[i]->SetForceDistance(DAVA::LodComponent::INVALID_DISTANCE);
		}
	}
}

DAVA::float32 EditorLODData::GetForceDistance() const
{
    return forceDistance;
}

void EditorLODData::EnableForceDistance(bool enable)
{
    forceDistanceEnabled = enable;
	SetForceDistance(forceDistance);
}

bool EditorLODData::GetForceDistanceEnabled() const
{
    return forceDistanceEnabled;
}


void EditorLODData::GetLODDataFromScene()
{
    DAVA::int32 lodComponentsSize = lodData.size();
    if(lodComponentsSize)
    {
        DAVA::Set<DAVA::int32> layers;
        DAVA::Set<DAVA::int32> currentQualityLayers;
        
        size_t lodInfoSize = lodInfo.size();
        
        DAVA::Vector<DAVA::uint32> triangleInfo;
        triangleInfo.resize(lodInfoSize, 0);
        
        DAVA::Vector<DAVA::uint32> lodComponentsCount;
        lodComponentsCount.resize(lodInfoSize, 0);
        
        for(DAVA::int32 i = 0; i < lodComponentsSize; ++i)
        {
            //distances
            
            lodData[i]->SetQuality(currentLODQuality);
            
            CollectLodLayers(lodData[i], layers);
            
            DVASSERT(lodInfoSize >= lodData[i]->lodLayersArray.size());
            
            size_t lodLayersArraySize = lodData[i]->lodLayersArray.size();
            for(size_t lodLayerIndex = 0; lodLayerIndex < lodLayersArraySize; ++lodLayerIndex)
            {
                lodInfo[lodLayerIndex].lodDistance += lodData[i]->GetLodLayerDistance(lodLayerIndex);
                
                if(1 == lodComponentsSize)
                {
                    lodInfo[lodLayerIndex].lodIndex = lodData[i]->GetLodLayerLodIndex(lodLayerIndex);
                }
                
                ++lodComponentsCount[lodLayerIndex];
                
                currentQualityLayers.insert((DAVA::int32)lodData[i]->GetLodLayerLodIndex(lodLayerIndex));
            }
            
            //triangles
            
            AddTrianglesInfo(triangleInfo, lodData[i], false);
        }
        
        size_t lodComponentCountSize = lodComponentsCount.size();
        for(size_t i = 0; i < lodComponentCountSize; ++i)
        {
            if(lodComponentsCount[i])
            {
                lodInfo[i].lodDistance /= lodComponentsCount[i];
                lodInfo[i].isEmpty = false;
            }
        }
        
        for(size_t i = 0; i < lodInfoSize; ++i)
        {
            lodInfo[i].lodTriangles = triangleInfo[i];
        }
        
        sortedLodIndices.clear();
        sortedLodIndices.insert(sortedLodIndices.end(), layers.begin(), layers.end());
        std::sort(sortedLodIndices.begin(), sortedLodIndices.end());
        
        sortedCurrentLodIndices.clear();
        sortedCurrentLodIndices.insert(sortedCurrentLodIndices.begin(), currentQualityLayers.begin(), currentQualityLayers.end());
        std::sort(sortedCurrentLodIndices.begin(), sortedCurrentLodIndices.end());

        emit DataChanged();
    }
}

void EditorLODData::AddTrianglesInfo(DAVA::Vector<DAVA::uint32>& triangles, DAVA::LodComponent *lod, bool onlyVisibleBatches)
{
    Entity * en = lod->GetEntity();
    if (GetEffectComponent(en))
        return;
    RenderObject * ro = GetRenderObject(en);
    if(ro)
    {
        DAVA::Vector<DAVA::int32> distanceIndices;
        
        uint32 batchCount = ro->GetRenderBatchCount();
        for(uint32 i = 0; i < batchCount; ++i)
        {
            int32 lodIndex = 0;
            int32 switchIndex = 0;
        
            RenderBatch *rb = ro->GetRenderBatch(i, lodIndex, switchIndex);
            if(IsPointerToExactClass<RenderBatch>(rb))
            {
                if(onlyVisibleBatches)
                { //check batch visibility
                
                    bool batchIsVisible = false;
                    uint32 activeBatchCount = ro->GetActiveRenderBatchCount();
                    for(uint32 a = 0; a < activeBatchCount && !batchIsVisible; ++a)
                    {
                        RenderBatch *visibleBatch = ro->GetActiveRenderBatch(a);
                        batchIsVisible = (visibleBatch == rb);
                    }
                
                    if(batchIsVisible == false) // need to skip this render batch
                        continue;
                }
            
                PolygonGroup *pg = rb->GetPolygonGroup();
                if(pg)
                {
                    distanceIndices.clear();
                    
                    MapLodIndexToDistanceIndex(lodIndex, lod->lodLayersArray, distanceIndices);
                    
                    size_t distanceIndexCount = distanceIndices.size();
                    for(size_t distanceIndex = 0; distanceIndex < distanceIndexCount; ++distanceIndex)
                    {
                        triangles[distanceIndices[distanceIndex]] += (pg->GetIndexCount() / 3);
                    }
                }
            }
        }
    }
}


void EditorLODData::EnumerateLODs()
{
	if(!activeScene) return;

	if(allSceneModeEnabled)
	{
		EnumerateLODsRecursive(activeScene, lodData);
	}
	else
	{
		EntityGroup selection = activeScene->selectionSystem->GetSelection();

		DAVA::uint32 count = selection.Size();
		for(DAVA::uint32 i = 0; i < count; ++i)
		{
			EnumerateLODsRecursive(selection.GetEntity(i), lodData);
		}
	}
}


void EditorLODData::EnumerateLODsRecursive(DAVA::Entity *entity, DAVA::Vector<DAVA::LodComponent *> & lods)
{
    DAVA::LodComponent *lod = GetLodComponent(entity);
    if(lod)
    {
        lods.push_back(lod);
        return;
    }
    
    DAVA::int32 count = entity->GetChildrenCount();
    for(DAVA::int32 i = 0; i < count; ++i)
    {
        EnumerateLODsRecursive(entity->GetChild(i), lods);
    }
}


void EditorLODData::SetForceLayer(DAVA::int32 layer)
{
    forceLayer = layer;
    
    DAVA::uint32 count = lodData.size();
    for(DAVA::uint32 i = 0; i < count; ++i)
    {
        lodData[i]->SetForceLodLayer(forceLayer);
    }
}

DAVA::int32 EditorLODData::GetForceLayer() const
{
    return forceLayer;
}

void EditorLODData::SceneActivated(SceneEditor2 *scene)
{
    activeScene = scene;
    
    UpdateLODStateFromScene();
    GetLODDataFromScene();
    ClearForceData();
}

void EditorLODData::SceneDeactivated(SceneEditor2 *scene)
{
    if(activeScene)
    {
        ResetForceState(activeScene);
        activeScene = NULL;
    }

    ClearLODData();
    ClearForceData();
}

void EditorLODData::SceneStructureChanged(SceneEditor2 *scene, DAVA::Entity *parent)
{
    ResetForceState(parent);

    if(activeScene == scene)
    {
        UpdateLODStateFromScene();
        GetLODDataFromScene();
    }

    UpdateForceData();
}

void EditorLODData::UpdateForceData()
{
    if(forceDistanceEnabled)
    {
        SetForceDistance(forceDistance);
    }
    else if(forceLayer != DAVA::LodComponent::INVALID_LOD_LAYER)
    {
        SetForceLayer(forceLayer);
    }
}


void EditorLODData::CommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo)
{
    if(command->GetId() == CMDID_BATCH)
    {
		CommandBatch *batch = (CommandBatch *)command;
		Command2 *firstCommand = batch->GetCommand(0);
		if(firstCommand && (firstCommand->GetId() == CMDID_LOD_DISTANCE_CHANGE || 
                            firstCommand->GetId() == CMDID_LOD_COPY_LAST_LOD ||
                            firstCommand->GetId() == CMDID_LOD_DELETE ||
                            firstCommand->GetId() == CMDID_LOD_CREATE_PLANE ||
                            firstCommand->GetId() == CMDID_SET_LOD_INDEX))
		{
            UpdateLODStateFromScene();
			GetLODDataFromScene();
		}
    }
}

void EditorLODData::CreatePlaneLOD(DAVA::int32 fromLayer, DAVA::uint32 textureSize, const DAVA::FilePath & texturePath)
{
    DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
    if(componentsCount && activeScene)
    {
        activeScene->BeginBatch("LOD Added");

        for(DAVA::uint32 i = 0; i < componentsCount; ++i)
            activeScene->Exec(new CreatePlaneLODCommand(lodData[i], fromLayer, textureSize, texturePath, currentLODQuality));

        activeScene->EndBatch();
    }
}

void EditorLODData::CopyLastLodToLod0()
{
    DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
    if(componentsCount && activeScene)
    {
        activeScene->BeginBatch("LOD Added");

        for(DAVA::uint32 i = 0; i < componentsCount; ++i)
            activeScene->Exec(new CopyLastLODToLod0Command(lodData[i], currentLODQuality));

        activeScene->EndBatch();
    }
}

bool EditorLODData::CanCreatePlaneLOD()
{
    if(lodData.size() != 1)
        return false;

    Entity * componentOwner = lodData[0]->GetEntity();
    if(componentOwner->GetComponent(Component::PARTICLE_EFFECT_COMPONENT) || componentOwner->GetParent()->GetComponent(Component::PARTICLE_EFFECT_COMPONENT))
        return false;

    return (GetLodLayersCount(lodData[0]) < LodComponent::MAX_LOD_LAYERS);
}

FilePath EditorLODData::GetDefaultTexturePathForPlaneEntity()
{
    DVASSERT(lodData.size() == 1);
    Entity * entity = lodData[0]->GetEntity();

    FilePath entityPath = activeScene->GetScenePath();
    KeyedArchive * properties = GetCustomPropertiesArchieve(entity);
    if(properties && properties->IsKeyExists(ResourceEditor::EDITOR_REFERENCE_TO_OWNER))
        entityPath = FilePath(properties->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, entityPath.GetAbsolutePathname()));

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

bool EditorLODData::CanDeleteLod()
{
    if(lodData.size() == 0)
        return false;
    
    Entity * componentOwner = lodData[0]->GetEntity();
    if(componentOwner->GetComponent(Component::PARTICLE_EFFECT_COMPONENT) || componentOwner->GetParent()->GetComponent(Component::PARTICLE_EFFECT_COMPONENT))
        return false;
    
    return true;
}

void EditorLODData::DeleteFirstLOD()
{
    if(CanDeleteLod() == false) return;
    
    DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
    if(componentsCount && activeScene)
    {
        activeScene->BeginBatch("Delete First LOD");
        
        for(DAVA::uint32 i = 0; i < componentsCount; ++i)
            activeScene->Exec(new DeleteLODCommand(lodData[i], lodData[i]->lodLayersArray[0].lodIndex, -1, currentLODQuality));
        
        activeScene->EndBatch();
    }
}

void EditorLODData::DeleteLastLOD()
{
    if(CanDeleteLod() == false) return;

    DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
    if(componentsCount && activeScene)
    {
        activeScene->BeginBatch("Delete Last LOD");
        
        for(DAVA::uint32 i = 0; i < componentsCount; ++i)
            activeScene->Exec(new DeleteLODCommand(lodData[i], GetMaxLodLayerIndex(lodData[i]), -1, currentLODQuality));
        
        activeScene->EndBatch();
    }
}

void EditorLODData::EnableAllSceneMode( bool enabled )
{
	allSceneModeEnabled = enabled;

	SceneActivated(activeScene);
}

void EditorLODData::SetLODQuality(const DAVA::FastName& lodQualityName)
{
    currentLODQuality = lodQualityName;
    
    ResetLODInfo();
    GetLODDataFromScene();
}

const DAVA::FastName& EditorLODData::GetLODQuality() const
{
    return currentLODQuality;
}

DAVA::LodComponent::QualityContainer* EditorLODData::GetQualityContainer(const DAVA::FastName& qualityName,
                                                                         DAVA::LodComponent* lodComponent)
{
    DVASSERT(lodComponent);
    
    DAVA::LodComponent::QualityContainer* container = NULL;
    uint32 storedQualityCount = (lodComponent->qualityContainer != NULL) ? lodComponent->qualityContainer->size() : 0;
    for(uint32 i = 0; i < storedQualityCount; ++i)
    {
        if(qualityName == (*lodComponent->qualityContainer)[i].qualityName)
        {
            container = &(*lodComponent->qualityContainer)[i];
        }
    }
    
    return container;
}

void EditorLODData::UpdateLODStateFromScene()
{
    ClearLODData();
    EnumerateLODs();
}

bool EditorLODData::IsEmptyLayer(DAVA::uint32 layerNum) const
{
    DVASSERT(layerNum >= 0 && layerNum < lodInfo.size());
    return lodInfo[layerNum].IsEmpty();
}

DAVA::uint32 EditorLODData::GetDistanceCount() const
{
    DAVA::uint32 distanceCount = 0;
    size_t lodInfoCount = lodInfo.size();
    
    for(size_t i = 0; i < lodInfoCount; ++i)
    {
        if(!lodInfo[i].IsEmpty())
        {
            distanceCount++;
        }
    }
    
    return distanceCount;
}

const DAVA::Vector<DAVA::int32>& EditorLODData::GetLODIndices() const
{
    return sortedLodIndices;
}

const DAVA::Vector<DAVA::int32>& EditorLODData::GetCurrentLODIndices() const
{
    return sortedCurrentLodIndices;
}

void EditorLODData::MapLodIndexToDistanceIndex(DAVA::int32 lodIndex, const DAVA::Vector<DAVA::LodComponent::LodDistance>& lodDistances, DAVA::Vector<DAVA::int32>& indices)
{
    DAVA::int32 index = -1;
    size_t lodDistanceCount = lodDistances.size();
    for(size_t i = 0; i < lodDistanceCount; ++i)
    {
        if(lodDistances[i].lodIndex == lodIndex)
        {
            indices.push_back(i);
        }
    }
}