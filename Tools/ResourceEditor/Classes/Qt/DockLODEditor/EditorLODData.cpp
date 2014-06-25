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


EditorLODData::EditorLODData()
    :   lodLayersCount(0)
    ,   forceDistanceEnabled(false)
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
    lodLayersCount = 0;
    
    for(DAVA::int32 i = 0; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
    {
        lodDistances[i] = 0;
        lodTriangles[i] = 0;
    }

    lodData.clear();
    
    emit DataChanged();
}

void EditorLODData::ClearForceData()
{
    forceDistance = 0.f;
    forceLayer = DAVA::LodComponent::INVALID_LOD_LAYER;
}

DAVA::uint32 EditorLODData::GetLayersCount() const
{
    return lodLayersCount;
}

DAVA::float32 EditorLODData::GetLayerDistance(DAVA::uint32 layerNum) const
{
    DVASSERT(layerNum < lodLayersCount)
    return lodDistances[layerNum];
}

void EditorLODData::SetLayerDistance(DAVA::uint32 layerNum, DAVA::float32 distance)
{
    DVASSERT(layerNum < lodLayersCount)
    lodDistances[layerNum] = distance;

    DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
    if(componentsCount && activeScene)
    {
        activeScene->BeginBatch("LOD Distance Changed");
        
        for(DAVA::uint32 i = 0; i < componentsCount; ++i)
        {
			activeScene->Exec(new ChangeLODDistanceCommand(lodData[i], layerNum, distance));
        }
        
        activeScene->EndBatch();
    }
}

void EditorLODData::UpdateDistances( const DAVA::Map<DAVA::uint32, DAVA::float32> & newDistances )
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

			DVASSERT(layerNum < lodLayersCount)
			lodDistances[layerNum] = distance;

			for(DAVA::uint32 i = 0; i < componentsCount; ++i)
			{
                activeScene->Exec(new ChangeLODDistanceCommand(lodData[i], layerNum, distance));
			}
		}

		activeScene->EndBatch();
	}
}



DAVA::uint32 EditorLODData::GetLayerTriangles(DAVA::uint32 layerNum) const
{
    DVASSERT(layerNum < lodLayersCount)
    return lodTriangles[layerNum];
}


void EditorLODData::SceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected)
{
    if(activeScene == scene && !allSceneModeEnabled)
    {
		for(size_t i = 0; i < deselected->Size(); ++i)
		{
			ResetForceState(deselected->GetEntity(i));
		}

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
    ClearLODData();
    EnumerateLODs();

    DAVA::int32 lodComponentsSize = lodData.size();
    if(lodComponentsSize)
    {
        DAVA::int32 lodComponentsCount[DAVA::LodComponent::MAX_LOD_LAYERS] = { 0 };
        for(DAVA::int32 i = 0; i < lodComponentsSize; ++i)
        {
            //distances
            DAVA::int32 layersCount = GetLodLayersCount(lodData[i]);
            for(DAVA::int32 layer = 0; layer < layersCount; ++layer)
            {
                lodDistances[layer] += lodData[i]->GetLodLayerDistance(layer);
                ++lodComponentsCount[layer];
            }

            //triangles
            AddTrianglesInfo(lodTriangles, lodData[i], false);
        }
        

        for(DAVA::int32 i = 0; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
        {
            if(lodComponentsCount[i])
            {
                lodDistances[i] /= lodComponentsCount[i];
                ++lodLayersCount;
            }
        }

        emit DataChanged();
    }
}

void EditorLODData::AddTrianglesInfo(DAVA::uint32 triangles[], DAVA::LodComponent *lod, bool onlyVisibleBatches)
{
    Entity * en = lod->GetEntity();
    if (GetEffectComponent(en))
        return;
    RenderObject * ro = GetRenderObject(en);
    if(ro)
    {
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
                    triangles[lodIndex] += (pg->GetIndexCount() / 3);
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
                            firstCommand->GetId() == CMDID_LOD_CREATE_PLANE))
		{
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
            activeScene->Exec(new CreatePlaneLODCommand(lodData[i], fromLayer, textureSize, texturePath));

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
            activeScene->Exec(new CopyLastLODToLod0Command(lodData[i]));

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
            activeScene->Exec(new DeleteLODCommand(lodData[i], 0, -1));
        
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
            activeScene->Exec(new DeleteLODCommand(lodData[i], GetLodLayersCount(lodData[i]) - 1, -1));
        
        activeScene->EndBatch();
    }
}

void EditorLODData::EnableAllSceneMode( bool enabled )
{
	allSceneModeEnabled = enabled;

	SceneActivated(activeScene);
}


