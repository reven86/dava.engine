#include "EditorLODSystem.h"

#include "Scene/SceneSignals.h"
#include "Commands2/ChangeLODDistanceCommand.h"
#include "Commands2/CreatePlaneLODCommand.h"
#include "Commands2/DeleteLODCommand.h"
#include "Commands2/CopyLastLODCommand.h"

EditorLODSystem::EditorLODSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
	,    lodLayersCount(0)
	,    forceDistanceEnabled(false)
	,    forceDistance(0.f)
	,    forceLayer(DAVA::LodComponent::INVALID_LOD_LAYER)
	,    scene2(static_cast<SceneEditor2*>(scene))
	,    allSceneModeEnabled(false)
{

}


EditorLODSystem::~EditorLODSystem(void)
{
}

void EditorLODSystem::ClearLODData()
{
	lodLayersCount = 0;

	for(DAVA::int32 i = 0; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
	{
		lodDistances[i] = 0;
		lodTriangles[i] = 0;
	}

	lodData.clear();
}

void EditorLODSystem::ClearForceData()
{
	forceDistance = 0.f;
	forceLayer = DAVA::LodComponent::INVALID_LOD_LAYER;
}

void EditorLODSystem::SetLayerDistance(DAVA::uint32 layerNum, DAVA::float32 distance)
{
	DVASSERT(layerNum < lodLayersCount);
	lodDistances[layerNum] = distance;

	DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
	if (componentsCount)
	{
		scene2->BeginBatch("LOD Distance Changed");
		for(DAVA::uint32 i = 0; i < componentsCount; ++i)
		{
			scene2->Exec(new ChangeLODDistanceCommand(lodData[i], layerNum, distance));
		}
		scene2->EndBatch();
	}
}

void EditorLODSystem::UpdateDistances( const DAVA::Map<DAVA::uint32, DAVA::float32> & newDistances )
{
	DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
	if(componentsCount && newDistances.size() != 0)
	{
		static_cast<SceneEditor2*>(GetScene())->BeginBatch("LOD Distances Changed");

		DAVA::Map<DAVA::uint32, DAVA::float32>::const_iterator endIt = newDistances.end();
		for(auto it = newDistances.begin(); it != endIt; ++it)
		{
			DAVA::uint32 layerNum = it->first;
			DAVA::float32 distance = it->second;

			DVASSERT(layerNum < lodLayersCount)
				lodDistances[layerNum] = distance;

			for(DAVA::uint32 i = 0; i < componentsCount; ++i)
			{
				scene2->Exec(new ChangeLODDistanceCommand(lodData[i], layerNum, distance));
			}
		}

		scene2->EndBatch();
	}
}

void EditorLODSystem::SceneSelectionChanged(const EntityGroup *selected, const EntityGroup *deselected)
{
	if (!allSceneModeEnabled)
	{
		for(size_t i = 0; i < deselected->Size(); ++i)
		{
			ResetForceState(deselected->GetEntity(i));
		}

		CollectLODDataFromScene();
		UpdateForceData();
	}
}

void EditorLODSystem::ResetForceState(DAVA::Entity *entity)
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

void EditorLODSystem::SetForceDistance(DAVA::float32 distance)
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

void EditorLODSystem::SetForceDistanceEnabled(bool enable)
{
	forceDistanceEnabled = enable;
	SetForceDistance(forceDistance);
}

void EditorLODSystem::CollectLODDataFromScene()
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
	}
}

void EditorLODSystem::AddTrianglesInfo(DAVA::uint32 triangles[], DAVA::LodComponent *lod, bool onlyVisibleBatches)
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

void EditorLODSystem::EnumerateLODs()
{
	if (!scene2) return;

	if(allSceneModeEnabled)
	{
		EnumerateLODsRecursive(scene2, lodData);
	}
	else
	{
		EntityGroup selection = scene2->selectionSystem->GetSelection();

		DAVA::uint32 count = selection.Size();
		for(DAVA::uint32 i = 0; i < count; ++i)
		{
			EnumerateLODsRecursive(selection.GetEntity(i), lodData);
		}
	}
}

void EditorLODSystem::EnumerateLODsRecursive(DAVA::Entity *entity, DAVA::Vector<DAVA::LodComponent *> & lods)
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

void EditorLODSystem::SetForceLayer(DAVA::int32 layer)
{
	forceLayer = layer;

	DAVA::uint32 count = lodData.size();
	for(DAVA::uint32 i = 0; i < count; ++i)
	{
		lodData[i]->SetForceLodLayer(forceLayer);
		lodData[i]->currentLod = DAVA::LodComponent::INVALID_LOD_LAYER;
	}
}

void EditorLODSystem::SceneStructureChanged(DAVA::Entity *parent)
{
	ResetForceState(parent);
	CollectLODDataFromScene();
	UpdateForceData();
}

void EditorLODSystem::UpdateForceData()
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

void EditorLODSystem::CreatePlaneLOD(DAVA::int32 fromLayer, DAVA::uint32 textureSize, const DAVA::FilePath & texturePath)
{
	DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
	if (componentsCount)
	{
		scene2->BeginBatch("LOD Added");

		for(DAVA::uint32 i = 0; i < componentsCount; ++i)
			scene2->Exec(new CreatePlaneLODCommand(lodData[i], fromLayer, textureSize, texturePath));

		scene2->EndBatch();
	}
}

void EditorLODSystem::CopyLastLodToLod0()
{
	DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
	if(componentsCount)
	{
		scene2->BeginBatch("LOD Added");

		for(DAVA::uint32 i = 0; i < componentsCount; ++i)
			scene2->Exec(new CopyLastLODToLod0Command(lodData[i]));

		scene2->EndBatch();
	}
}

bool EditorLODSystem::CanCreatePlaneLOD()
{
	if(lodData.size() != 1)
		return false;

	Entity * componentOwner = lodData[0]->GetEntity();
	if(componentOwner->GetComponent(Component::PARTICLE_EFFECT_COMPONENT) || componentOwner->GetParent()->GetComponent(Component::PARTICLE_EFFECT_COMPONENT))
		return false;

	return (GetLodLayersCount(lodData[0]) < LodComponent::MAX_LOD_LAYERS);
}

FilePath EditorLODSystem::GetDefaultTexturePathForPlaneEntity()
{
	DVASSERT(lodData.size() == 1);
	Entity * entity = lodData[0]->GetEntity();

	FilePath entityPath = scene2->GetScenePath();
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

bool EditorLODSystem::CanDeleteLod()
{
	const uint32 count = (const uint32)lodData.size();
	if(count == 0)
	{
		return false;
	}

	for(uint32 i = 0; i < count; ++i)
	{
		Entity * componentOwner = lodData[i]->GetEntity();
		if(     componentOwner->GetComponent(Component::PARTICLE_EFFECT_COMPONENT)
			||   componentOwner->GetParent()->GetComponent(Component::PARTICLE_EFFECT_COMPONENT))
		{
			return false;
		}
	}

	return true;
}

void EditorLODSystem::DeleteFirstLOD()
{
	if(CanDeleteLod() == false) return;

	DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
	if(componentsCount)
	{
		scene2->BeginBatch("Delete First LOD");

		for(DAVA::uint32 i = 0; i < componentsCount; ++i)
			scene2->Exec(new DeleteLODCommand(lodData[i], 0, -1));

		scene2->EndBatch();
	}
}

void EditorLODSystem::DeleteLastLOD()
{
	if(CanDeleteLod() == false) return;

	DAVA::uint32 componentsCount = (DAVA::uint32)lodData.size();
	if (componentsCount)
	{
		scene2->BeginBatch("Delete Last LOD");

		for(DAVA::uint32 i = 0; i < componentsCount; ++i)
			if (GetLodLayersCount(lodData[i]) > 0)
				scene2->Exec(new DeleteLODCommand(lodData[i], GetLodLayersCount(lodData[i]) - 1, -1));

		scene2->EndBatch();
	}
}

void EditorLODSystem::SetAllSceneModeEnabled( bool enabled )
{
	allSceneModeEnabled = enabled;

	CollectLODDataFromScene();
	ClearForceData();
}

