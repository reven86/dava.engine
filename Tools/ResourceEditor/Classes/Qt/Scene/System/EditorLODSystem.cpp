#include "EditorLODSystem.h"

#include "Scene/SceneSignals.h"
#include "Commands2/ChangeLODDistanceCommand.h"
#include "Commands2/CreatePlaneLODCommand.h"
#include "Commands2/DeleteLODCommand.h"
#include "Commands2/CopyLastLODCommand.h"

EditorLODSystem::EditorLODSystem(DAVA::Scene * scene)
	:    DAVA::SceneSystem(scene)
	,    sceneLodsLayersCount(0)
	,    forceDistanceEnabled(false)
	,    forceDistance(DAVA::LodComponent::MIN_LOD_DISTANCE)
	,    forceLayer(DAVA::LodComponent::INVALID_LOD_LAYER)
	,    allSceneModeEnabled(false)
{
}

EditorLODSystem::~EditorLODSystem(void)
{
}

void EditorLODSystem::AddEntity(DAVA::Entity * entity)
{
	DVASSERT(entity);
	sceneEntities.push_back(entity);
	DAVA::LodComponent *tmpComponent = GetLodComponent(entity);
	DVASSERT(tmpComponent);
	sceneLODs.push_back(tmpComponent);
}

void EditorLODSystem::RemoveEntity(DAVA::Entity * entity)
{
	DVASSERT(entity);
	sceneEntities.erase(std::remove(sceneEntities.begin(), sceneEntities.end(), entity), sceneEntities.end());
	DAVA::LodComponent *tmpComponent = GetLodComponent(entity);
	DVASSERT(tmpComponent);
	sceneLODs.erase(std::remove(sceneLODs.begin(), sceneLODs.end(), tmpComponent), sceneLODs.end());
}

void EditorLODSystem::AddSelectedLODsRecursive(DAVA::Entity *entity)
{
	DVASSERT(entity);
	DAVA::LodComponent *tmpComponent = GetLodComponent(entity);
	if (tmpComponent)
	{
		selectedLODs.push_back(tmpComponent);
	}
	auto count = entity->GetChildrenCount();
	for (auto i = 0; i < count; i++)
	{
		AddSelectedLODsRecursive(entity->GetChild(i));
	}
}

void EditorLODSystem::RemoveSelectedLODsRecursive(DAVA::Entity *entity)
{
	DVASSERT(entity);
	DAVA::LodComponent *tmpComponent = GetLodComponent(entity);
	if (tmpComponent)
	{
		selectedLODs.erase(std::remove(selectedLODs.begin(), selectedLODs.end(), tmpComponent), selectedLODs.end());
	}
	auto count = entity->GetChildrenCount();
	for (auto i = 0; i < count; i++)
	{
		AddSelectedLODsRecursive(entity->GetChild(i));
	}
}

void EditorLODSystem::UpdateDistances( const DAVA::Map<DAVA::uint32, DAVA::float32> & newDistances )
{
	if (getCurrentLODs().empty() || newDistances.empty())
	{
		return;
	}
	for (auto newDistance : newDistances)
	{
		SetLayerDistance(newDistance.first, newDistance.second);
	}
}

void EditorLODSystem::SceneSelectionChanged(const EntityGroup *selected, const EntityGroup *deselected)
{
	const size_t deselectedCount = deselected->Size();
	for (size_t i = 0; i < deselectedCount; ++i)
	{
		if (!allSceneModeEnabled)
		{
			ResetForceState(deselected->GetEntity(i));
		}
		RemoveSelectedLODsRecursive(deselected->GetEntity(i));
	}
	const size_t selectedCount = selected->Size();
	for (size_t i = 0; i < selectedCount; ++i)
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
	if (nullptr == entity)
	{
		return;
	}
	DAVA::LodComponent *tmpComponent = GetLodComponent(entity);
	if (tmpComponent)
	{
		tmpComponent->SetForceDistance(DAVA::LodComponent::INVALID_DISTANCE);
		tmpComponent->SetForceLodLayer(DAVA::LodComponent::INVALID_LOD_LAYER);
		tmpComponent->currentLod = -1;
	}
	for (auto child : entity->children)
	{
		ResetForceState(child);
	}
}

void EditorLODSystem::CollectLODDataFromScene()
{
	sceneLodsLayersCount = 0;
	std::fill(lodDistances.begin(), lodDistances.end(), 0);
	std::fill(lodTrianglesCount.begin(), lodTrianglesCount.end(), 0);
	std::array<DAVA::int32, DAVA::LodComponent::MAX_LOD_LAYERS> lodsComponentsCount = { 0 };
	for (auto lod : getCurrentLODs())
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
	for(DAVA::int32 i = 0; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
	{
		if(0 != lodsComponentsCount[i])
		{
			lodDistances[i] /= lodsComponentsCount[i];
			++sceneLodsLayersCount;
		}
	}
}

void EditorLODSystem::AddTrianglesInfo(std::array<DAVA::uint32, DAVA::LodComponent::MAX_LOD_LAYERS> &triangles, DAVA::LodComponent *lod, bool onlyVisibleBatches)
{
	Entity * en = lod->GetEntity();
	if (GetEffectComponent(en))
	{
		return;
	}
	RenderObject * ro = GetRenderObject(en);
	if (!ro)
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
			Logger::Error("got unexpected lod index (%d) when collecting triangles on entitie %s. Correct values for lod index is 0÷%d", lodIndex, en->GetName().c_str(), DAVA::LodComponent::MAX_LOD_LAYERS);
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

				if(batchIsVisible == false) // need to skip this render batch
					continue;
			}

			PolygonGroup *pg = rb->GetPolygonGroup();
			if(pg)
			{
				DVASSERT(lodIndex < DAVA::LodComponent::MAX_LOD_LAYERS);
				DVASSERT(lodIndex >= 0);
				triangles[lodIndex] += pg->GetIndexCount() / 3; 
			}
		}
	}
}

void EditorLODSystem::EnumerateLODsRecursive(DAVA::Entity *entity, DAVA::Vector<DAVA::LodComponent *> *lods)
{
	DAVA::LodComponent *lod = GetLodComponent(entity);
	if(lod)
	{
		lods->push_back(lod);
		return;
	}

	for (auto child : entity->children)
	{
		EnumerateLODsRecursive(child, lods);
	}
}

void EditorLODSystem::UpdateForceData()
{
	UpdateForceDistance();
	if(forceLayer != DAVA::LodComponent::INVALID_LOD_LAYER)
	{
		UpdateForceLayer();
	}
}

void EditorLODSystem::CreatePlaneLOD(DAVA::int32 fromLayer, DAVA::uint32 textureSize, const DAVA::FilePath & texturePath)
{
	if (getCurrentLODs().empty())
	{
		return;
	}

	SceneEditor2* sceneEditor2 = static_cast<SceneEditor2*>(GetScene());

	sceneEditor2->BeginBatch("LOD Added");

	for (DAVA::LodComponent *currentLOD : getCurrentLODs())
	{
		sceneEditor2->Exec(new CreatePlaneLODCommand(currentLOD, fromLayer, textureSize, texturePath));
	}
	sceneEditor2->EndBatch();
}

void EditorLODSystem::CopyLastLodToLod0()
{
	if (getCurrentLODs().empty())
	{
		return;
	}	
	SceneEditor2* sceneEditor2 = static_cast<SceneEditor2*>(GetScene());

	sceneEditor2->BeginBatch("LOD Added");

	for (DAVA::LodComponent *currentLOD : getCurrentLODs())
	{
		sceneEditor2->Exec(new CopyLastLODToLod0Command(currentLOD));
	}
	sceneEditor2->EndBatch();
}

bool EditorLODSystem::CanCreatePlaneLOD()
{
	if (1 != getCurrentLODs().size())
	{
		return false;
	}
	DAVA::LodComponent *firstLod = getCurrentLODs().at(0);
	Entity * componentOwner = firstLod->GetEntity();
	if (componentOwner->GetComponent(Component::PARTICLE_EFFECT_COMPONENT) || componentOwner->GetParent()->GetComponent(Component::PARTICLE_EFFECT_COMPONENT))
	{
		return false;
	}
	return (GetLodLayersCount(firstLod) < LodComponent::MAX_LOD_LAYERS);
}

FilePath EditorLODSystem::GetDefaultTexturePathForPlaneEntity()
{
	DVASSERT(getCurrentLODs().size() == 1);
	Entity * entity = getCurrentLODs().at(0)->GetEntity();

	FilePath entityPath = static_cast<SceneEditor2*>(GetScene())->GetScenePath();
	KeyedArchive * properties = GetCustomPropertiesArchieve(entity);
	if (properties && properties->IsKeyExists(ResourceEditor::EDITOR_REFERENCE_TO_OWNER))
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

bool EditorLODSystem::CanDeleteLod()
{
	if (getCurrentLODs().empty())
	{
		return false;
	}

	for (DAVA::LodComponent *currentLOD : getCurrentLODs())
	{
		Entity * componentOwner = currentLOD->GetEntity();
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
	if (false == CanDeleteLod())
	{
		return;
	}

	SceneEditor2* sceneEditor2 = static_cast<SceneEditor2*>(GetScene());
	sceneEditor2->BeginBatch("Delete First LOD");
	for (DAVA::LodComponent *currentLOD : getCurrentLODs())
	{
		sceneEditor2->Exec(new DeleteLODCommand(currentLOD, 0, -1));
	}
	sceneEditor2->EndBatch();
}

void EditorLODSystem::DeleteLastLOD()
{
	if (false == CanDeleteLod())
	{
	    return;
	}

	SceneEditor2* sceneEditor2 = static_cast<SceneEditor2*>(GetScene());
	sceneEditor2->BeginBatch("Delete Last LOD");
	for (DAVA::LodComponent *currentLOD : getCurrentLODs())
	{
		if (GetLodLayersCount(currentLOD) > 0)
		{
			sceneEditor2->Exec(new DeleteLODCommand(currentLOD, GetLodLayersCount(currentLOD) - 1, -1));
		}
	}
	sceneEditor2->EndBatch();
}

void EditorLODSystem::SetLayerDistance(DAVA::uint32 layerNum, DAVA::float32 distance)
{
	DVASSERT(layerNum < sceneLodsLayersCount);
	lodDistances[layerNum] = distance;
	if (getCurrentLODs().empty())
	{
		return;
	}

	SceneEditor2* sceneEditor2 = static_cast<SceneEditor2*>(GetScene());
	sceneEditor2->BeginBatch("LOD Distance Changed");
	for (DAVA::LodComponent *lod : getCurrentLODs())
	{
		sceneEditor2->Exec(new ChangeLODDistanceCommand(lod, layerNum, distance));
	}
	sceneEditor2->EndBatch();
}

void EditorLODSystem::SetForceDistanceEnabled(bool enable)
{
	forceDistanceEnabled = enable;
	UpdateForceDistance();
}

void EditorLODSystem::SetForceDistance(DAVA::float32 distance)
{
	if (forceDistance == distance)
	{
		return;
	}
	forceDistance = distance;
	UpdateForceDistance();
}

void EditorLODSystem::UpdateForceDistance()
{
	if (forceDistanceEnabled)
	{
		for (DAVA::LodComponent *currentLOD : getCurrentLODs())
		{
			currentLOD->SetForceDistance(forceDistance);
			currentLOD->SetForceLodLayer(DAVA::LodComponent::INVALID_LOD_LAYER);
		}
	}
	else
	{
		for (DAVA::LodComponent *currentLOD : getCurrentLODs())
		{
			currentLOD->SetForceDistance(DAVA::LodComponent::INVALID_DISTANCE);
		}
	}
}

void EditorLODSystem::SetForceLayer(DAVA::int32 layer)
{
	if (forceLayer == layer)
	{
		return;
	}

	forceLayer = layer;
	UpdateForceLayer();
}

void EditorLODSystem::UpdateForceLayer()
{
	for (DAVA::LodComponent *currentLOD : getCurrentLODs())
	{
		currentLOD->SetForceLodLayer(forceLayer);
		currentLOD->currentLod = DAVA::LodComponent::INVALID_LOD_LAYER;
	}
}

void EditorLODSystem::SetAllSceneModeEnabled( bool enabled )
{
	if (allSceneModeEnabled == enabled)
	{
		return;
	}
	allSceneModeEnabled = enabled;
	UpdateAllSceneModeEnabled();
}

void EditorLODSystem::UpdateAllSceneModeEnabled()
{
	for (DAVA::Entity *entity : sceneEntities)
	{
		ResetForceState(entity);
	}
	CollectLODDataFromScene();
	UpdateForceData();
}
