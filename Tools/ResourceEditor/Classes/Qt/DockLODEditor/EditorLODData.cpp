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

#include "Classes/Qt/Scene/SceneSignals.h"
#include "Classes/Commands2/ChangeLODDistanceCommand.h"

EditorLODData::EditorLODData()
    :   lodLayersCount(0)
    ,   forceDistanceEnabled(false)
    ,   forceDistance(0.f)
    ,   forceLayer(DAVA::LodComponent::INVALID_LOD_LAYER)
    ,   activeScene(NULL)
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

DAVA::int32 EditorLODData::GetLayersCount() const
{
    return lodLayersCount;
}

DAVA::float32 EditorLODData::GetLayerDistance(DAVA::int32 layerNum) const
{
    DVASSERT(0 <= layerNum && layerNum < lodLayersCount)
    return lodDistances[layerNum];
}

void EditorLODData::SetLayerDistance(DAVA::int32 layerNum, DAVA::float32 distance)
{
    DVASSERT(0 <= layerNum && layerNum < lodLayersCount)
    lodDistances[layerNum] = distance;

    DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
    if(componentsCount && activeScene)
    {
        activeScene->BeginBatch("LOD Distance Changed");
        
        for(DAVA::uint32 i = 0; i < componentsCount; ++i)
        {
            if(layerNum >= GetLayersCount(lodData[i]))
                continue;
           
			activeScene->Exec(new ChangeLODDistanceCommand(lodData[i], layerNum, distance));
        }
        
        activeScene->EndBatch();
    }
}

void EditorLODData::UpdateDistances( const DAVA::Map<DAVA::int32, DAVA::float32> & newDistances )
{
	DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
	if(componentsCount && activeScene && newDistances.size() != 0)
	{
		activeScene->BeginBatch("LOD Distances Changed");

		DAVA::Map<DAVA::int32, DAVA::float32>::const_iterator endIt = newDistances.end();
		for(auto it = newDistances.begin(); it != endIt; ++it)
		{
			DAVA::int32 layerNum = it->first;
			DAVA::float32 distance = it->second;

			DVASSERT(0 <= layerNum && layerNum < lodLayersCount)
			lodDistances[layerNum] = distance;

			for(DAVA::uint32 i = 0; i < componentsCount; ++i)
			{
				if(layerNum < GetLayersCount(lodData[i]))
				{
					activeScene->Exec(new ChangeLODDistanceCommand(lodData[i], layerNum, distance));
				}
			}
		}

		activeScene->EndBatch();
	}
}



DAVA::uint32 EditorLODData::GetLayerTriangles(DAVA::int32 layerNum) const
{
    DVASSERT(0 <= layerNum && layerNum < lodLayersCount)
    return lodTriangles[layerNum];
}


void EditorLODData::SceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected)
{
    if(activeScene == scene)
    {
		for(size_t i = 0; i < deselected->Size(); ++i)
		{
			ResetForceState(deselected->GetEntity(i));
		}

		GetDataFromSelection();
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


void EditorLODData::GetDataFromSelection()
{
    ClearLODData();
    EnumerateSelectionLODs(activeScene);

    DAVA::int32 lodComponentsSize = lodData.size();
    if(lodComponentsSize)
    {
        DAVA::int32 lodComponentsCount[DAVA::LodComponent::MAX_LOD_LAYERS] = { 0 };
        
        
        for(DAVA::int32 i = 0; i < lodComponentsSize; ++i)
        {
            //distances
            DAVA::int32 layersCount = GetLayersCount(lodData[i]);
            for(DAVA::int32 layer = 0; layer < layersCount; ++layer)
            {
                lodDistances[layer] += lodData[i]->GetLodLayerDistance(layer);
                ++lodComponentsCount[layer];
            }

            //triangles
            Vector<LodComponent::LodData*> lodLayers;
            lodData[i]->GetLodData(lodLayers);
            Vector<LodComponent::LodData*>::const_iterator lodLayerIt = lodLayers.begin();
            for(DAVA::int32 layer = 0; layer < layersCount && lodLayerIt != lodLayers.end(); ++layer, ++lodLayerIt)
            {
                lodTriangles[layer] += GetTrianglesForLodLayer(*lodLayerIt, false);
            }
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

DAVA::uint32 EditorLODData::GetTrianglesForLodLayer(DAVA::LodComponent::LodData *lodData, bool checkVisibility)
{
    Vector<Entity *> meshes;
    
    for(int32 n = 0; n < (int32)lodData->nodes.size(); ++n)
    {
        meshes.push_back(lodData->nodes[n]);
        
        lodData->nodes[n]->GetChildNodes(meshes);
    }
    
    uint32 trianglesCount = 0;
    uint32 meshesCount = (uint32)meshes.size();
    for(uint32 m = 0; m < meshesCount; ++m)
    {
        if(checkVisibility)
        {
            RenderObject *ro = GetRenderObject(meshes[m]);
            if(!ro || ((ro->GetFlags() & RenderObject::VISIBLE_LOD) != RenderObject::VISIBLE_LOD))
            {
                continue;
            }
        }
        
        trianglesCount += GetTrianglesForEntity(meshes[m], checkVisibility);
    }
    
    return trianglesCount;
}

DAVA::uint32 EditorLODData::GetTrianglesForEntity(DAVA::Entity *entity, bool checkVisibility)
{
    RenderObject *ro = GetRenderObject(entity);
    if(!ro || ro->GetType() != RenderObject::TYPE_MESH) return 0;
    
    uint32 trianglesCount = 0;
    uint32 count = ro->GetRenderBatchCount();
    for(uint32 r = 0; r < count; ++r)
    {
        RenderBatch *rb = ro->GetRenderBatch(r);
        if(checkVisibility && !rb->GetVisible())
            continue;
        
        PolygonGroup *pg = rb->GetPolygonGroup();
        if(pg)
        {
            trianglesCount += pg->GetIndexCount() / 3;
        }
    }

    return trianglesCount;
}

void EditorLODData::EnumerateSelectionLODs(SceneEditor2 * scene)
{
    EntityGroup selection = scene->selectionSystem->GetSelection();
    
    DAVA::uint32 count = selection.Size();
    for(DAVA::uint32 i = 0; i < count; ++i)
    {
        EnumerateLODsRecursive(selection.GetEntity(i), lodData);
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
    GetDataFromSelection();
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
        GetDataFromSelection();
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


DAVA::int32 EditorLODData::GetLayersCount(DAVA::LodComponent *lod) const
{
    if(GetEmitter(lod->GetEntity()))
    {
        return DAVA::LodComponent::MAX_LOD_LAYERS;
    }

    return lod->GetLodLayersCount();
}

void EditorLODData::CommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo)
{
    if(command->GetId() == CMDID_BATCH)
    {
		CommandBatch *batch = (CommandBatch *)command;
		Command2 *firstCommand = batch->GetCommand(0);
		if(firstCommand && (firstCommand->GetId() == CMDID_LOD_DISTANCE_CHANGE))
		{
			GetDataFromSelection();
		}
    }
}


