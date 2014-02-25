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



#include "Scene3D/Systems/LodSystem.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Particles/ParticleEmitter.h"
#include "Render/Highlevel/Camera.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Platform/SystemTimer.h"
#include "Core/PerformanceSettings.h"

namespace DAVA
{

LodSystem::LodSystem(Scene * scene)
:	SceneSystem(scene)
{
	camera = 0;
	UpdatePartialUpdateIndices();
}

void LodSystem::Process(float32 timeElapsed)
{
	float32 currFps = 1.0f/timeElapsed;

	float32 currPSValue = (currFps - PerformanceSettings::Instance()->GetPsPerformanceMinFPS())/(PerformanceSettings::Instance()->GetPsPerformanceMaxFPS()-PerformanceSettings::Instance()->GetPsPerformanceMinFPS());
	currPSValue = Clamp(currPSValue, 0.0f, 1.0f);
	float32 lodOffset = PerformanceSettings::Instance()->GetPsPerformanceLodOffset()*(1-currPSValue);
	float32 lodMult = 1.0f+(PerformanceSettings::Instance()->GetPsPerformanceLodMult()-1.0f)*(1-currPSValue);
	/*as we use square values - multiply it too*/
	lodOffset*=lodOffset;
	lodMult*=lodMult;

	for(int32 i = partialUpdateIndices[currentPartialUpdateIndex]; i < partialUpdateIndices[currentPartialUpdateIndex+1]; ++i)
	{
		Entity * entity = entities[i];
		LodComponent * lod = static_cast<LodComponent*>(entity->GetComponent(Component::LOD_COMPONENT));
		if(lod->flags & LodComponent::NEED_UPDATE_AFTER_LOAD)
		{
			UpdateEntityAfterLoad(entity);
			lod->flags &= ~LodComponent::NEED_UPDATE_AFTER_LOAD;
		}

		UpdateLod(entity, lodOffset, lodMult);
	}

	currentPartialUpdateIndex = currentPartialUpdateIndex < UPDATE_PART_PER_FRAME-1 ? currentPartialUpdateIndex+1 : 0;
}

void LodSystem::AddEntity(Entity * entity)
{
	entities.push_back(entity);
	UpdatePartialUpdateIndices();
}

void LodSystem::RemoveEntity(Entity * entity)
{
	uint32 size = entities.size();
	for(uint32 i = 0; i < size; ++i)
	{
		if(entities[i] == entity)
		{
			entities[i] = entities[size-1];
			entities.pop_back();
			UpdatePartialUpdateIndices();
			return;
		}
	}
	
	DVASSERT(0);
}

void LodSystem::UpdateEntityAfterLoad(Entity * entity)
{
	LodComponent * lod = static_cast<LodComponent*>(entity->GetComponent(Component::LOD_COMPONENT));
	for (Vector<LodComponent::LodData>::iterator it = lod->lodLayers.begin(); it != lod->lodLayers.end(); ++it)
	{
		LodComponent::LodData & ld = *it;
		size_t size = ld.indexes.size();
		for (size_t idx = 0; idx < size; ++idx)
		{
			int32 desiredIndex = ld.indexes[idx];
			if(desiredIndex < entity->GetChildrenCount())
			{
				Entity * childEntity = entity->GetChild(desiredIndex);
				ld.nodes.push_back(childEntity);
				{
					childEntity->SetLodVisible(false);
				}
			}
		}
	}

	lod->currentLod = LodComponent::INVALID_LOD_LAYER;
	ParticleEmitter * emmiter = GetEmitter(entity);
	if (emmiter)
	{
		lod->currentLod = LodComponent::MAX_LOD_LAYERS-1;
		emmiter->SetDesiredLodLevel(lod->currentLod);
	}
	else if(lod->lodLayers.size() > 0)
	{
		lod->SetCurrentLod(lod->lodLayers.size()-1);
	}
}

void LodSystem::UpdatePartialUpdateIndices()
{
	currentPartialUpdateIndex = 0;

	int32 size = (int32)entities.size();
	int32 partSize = size/UPDATE_PART_PER_FRAME;
	
	partialUpdateIndices.clear();
	int32 currentIndex = 0;
	partialUpdateIndices.push_back(currentIndex);
	for(int32 i = 0; i < UPDATE_PART_PER_FRAME; ++i)
	{
		currentIndex += partSize;
		partialUpdateIndices.push_back(currentIndex);
	}

	int32 & LastSlot = partialUpdateIndices[partialUpdateIndices.size()-1];
	LastSlot = Max(LastSlot, size);
}

void LodSystem::UpdateLod(Entity * entity, float32 psLodOffsetSq, float32 psLodMultSq)
{
	LodComponent * lodComponent = GetLodComponent(entity);
	int32 oldLod = lodComponent->currentLod;
	if(!RecheckLod(entity, psLodOffsetSq, psLodMultSq))
	{
		if (oldLod != LodComponent::INVALID_LOD_LAYER)
		{
			lodComponent->SetLayerVisibility(oldLod, false);
		}

		lodComponent->currentLod = LodComponent::INVALID_LOD_LAYER;
		return;
	}

	if (oldLod != lodComponent->currentLod) 
	{
		ParticleEmitter * emmiter = GetEmitter(entity);
		if (emmiter)
		{
			emmiter->SetDesiredLodLevel(lodComponent->currentLod);
			return;
		}
		
		if (oldLod != LodComponent::INVALID_LOD_LAYER)
		{
			lodComponent->SetLayerVisibility(oldLod, false);
		}

		lodComponent->SetLayerVisibility(lodComponent->currentLod, true);
	}
}

bool LodSystem::RecheckLod(Entity * entity, float32 psLodOffsetSq, float32 psLodMultSq)
{
	LodComponent * lodComponent = GetLodComponent(entity);
	bool usePsSettings = (GetEmitter(entity) != NULL);

	if(LodComponent::INVALID_LOD_LAYER != lodComponent->forceLodLayer) 
	{
		if (usePsSettings)
			lodComponent->currentLod = lodComponent->forceLodLayer;
		else if(lodComponent->lodLayers.size())
		{
			lodComponent->currentLod = Min((int32)lodComponent->lodLayers.size() - 1, lodComponent->forceLodLayer);
		}
		else
		{
			lodComponent->currentLod = LodComponent::INVALID_LOD_LAYER;
		}
		return true;
	}

	int32 layersCount = lodComponent->GetLodLayersCount();	
	float32 dst = CalculateDistanceToCamera(entity, lodComponent);

	
	if (usePsSettings)
	{
		layersCount = LodComponent::MAX_LOD_LAYERS;
		if (dst>lodComponent->GetLodLayerFarSquare(0)) //preserv lod 0 from degrade
			dst = dst*psLodMultSq+psLodOffsetSq;
	}

	int32 layer = FindProperLayer(dst, lodComponent, layersCount);
	if(layer != lodComponent->currentLod)
	{
		lodComponent->currentLod = layer;
	}

	return (lodComponent->currentLod != LodComponent::INVALID_LOD_LAYER);
}

DAVA::float32 LodSystem::CalculateDistanceToCamera(const Entity * entity, const LodComponent *lodComponent) const
{
	if (lodComponent->forceDistance != LodComponent::INVALID_DISTANCE) //LodComponent::INVALID_DISTANCE
	{
		return lodComponent->forceDistanceSq;
	}

	if(camera)
	{
		float32 dst = (camera->GetPosition() - entity->GetWorldTransform().GetTranslationVector()).SquareLength();
		dst *= camera->GetZoomFactor() * camera->GetZoomFactor();

		return dst;
	}

	return 0.f;
}

int32 LodSystem::FindProperLayer( float32 distance, const LodComponent *lodComponent, int32 requestedLayersCount)
{
	if(lodComponent->currentLod != LodComponent::INVALID_LOD_LAYER)
	{
		if ((distance >= lodComponent->GetLodLayerNearSquare(lodComponent->currentLod)) && (distance <= lodComponent->GetLodLayerFarSquare(lodComponent->currentLod)))
		{
			return lodComponent->currentLod;
		}
	}

	int32 layer = LodComponent::INVALID_LOD_LAYER;
	for (int32 i = requestedLayersCount - 1; i >= 0; --i)
	{
		if (distance < lodComponent->GetLodLayerFarSquare(i))
		{
			layer = i;
		}
	}

	return layer;
}

void LodSystem::SetCamera(Camera * _camera)
{
	camera = _camera;
}

void LodSystem::MergeChildLods(Entity * toEntity)
{
	LodSystem::LodMerger merger(toEntity);
	merger.MergeChildLods();
}

LodSystem::LodMerger::LodMerger(Entity * _toEntity)
{
	DVASSERT(_toEntity);
	toEntity = _toEntity;
}

void LodSystem::LodMerger::MergeChildLods()
{
	LodComponent * toLod = (LodComponent*)toEntity->GetOrCreateComponent(Component::LOD_COMPONENT);
	

	Vector<Entity*> allLods;
	GetLodComponentsRecursive(toEntity, allLods);

	uint32 count = allLods.size();
	for(uint32 i = 0; i < count; ++i)
	{
		LodComponent * fromLod = GetLodComponent(allLods[i]);
		int32 fromLodsCount = fromLod->GetMaxLodLayer();
		for(int32 l = 0; l <= fromLodsCount; ++l)
		{
			LodComponent::LodData & fromData = fromLod->lodLayers[l];
			int32 lodLayerIndex = fromData.layer;

			LodComponent::LodData * toData = 0;

			int32 maxLod = toLod->GetMaxLodLayer();
			//create loddata if needed
			if(lodLayerIndex > maxLod)
			{
				DVASSERT(maxLod == lodLayerIndex-1);

				toLod->lodLayers.push_back(fromData);
				toData = &(toLod->lodLayers[lodLayerIndex]);
				toData->nodes.clear();
				toData->indexes.clear(); //indeces will not have any sense after lod merge

				toLod->lodLayersArray = fromLod->lodLayersArray;
			}
			else
			{
				toData = &(toLod->lodLayers[lodLayerIndex]);
			}

			uint32 nodesToCopy = fromData.nodes.size();
			for(uint32 j = 0; j < nodesToCopy; ++j)
			{
				toData->nodes.push_back(fromData.nodes[j]); 
			}
		}

		allLods[i]->RemoveComponent(Component::LOD_COMPONENT);
	}
}

void LodSystem::LodMerger::GetLodComponentsRecursive(Entity * fromEntity, Vector<Entity*> & allLods)
{
	if(fromEntity != toEntity)
	{
		LodComponent * lod = GetLodComponent(fromEntity);
		ParticleEmitter *emitter = GetEmitter(fromEntity);
		if(lod&&(!emitter)) //as emitters have separate LOD logic
		{
			if(lod->flags & LodComponent::NEED_UPDATE_AFTER_LOAD)
			{
				LodSystem::UpdateEntityAfterLoad(fromEntity);
				lod->flags &= ~LodComponent::NEED_UPDATE_AFTER_LOAD;
			}

			allLods.push_back(fromEntity);
		}
	}
	int32 count = fromEntity->GetChildrenCount();
	for(int32 i = 0; i < count; ++i)
	{
		GetLodComponentsRecursive(fromEntity->GetChild(i), allLods);
	}
}

}
