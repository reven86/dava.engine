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


#include "Render/Highlevel/RenderSystem.h"
#include "Scene3D/Entity.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/ShadowVolumeRenderPass.h"
#include "Render/Highlevel/ShadowRect.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Render/Material/MaterialSystem.h"
#include "Render/Highlevel/SpatialTree.h"

// TODO: Move class to other place
#include "Scene3D/Systems/ParticleEmitterSystem.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "Utils/Utils.h"
#include "Debug/Stats.h"

namespace DAVA
{

RenderSystem::RenderSystem()
{
    camera = 0;
    clipCamera = 0;
    // Register available passes & layers
    renderPassesMap.Insert(PASS_FORWARD, new RenderPass(PASS_FORWARD));
    renderPassesMap.Insert(PASS_SHADOW_VOLUME, new ShadowVolumeRenderPass(PASS_SHADOW_VOLUME));
	
    renderLayersMap.Insert(LAYER_OPAQUE, new RenderLayer(LAYER_OPAQUE, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_MATERIAL));
	renderLayersMap.Insert(LAYER_AFTER_OPAQUE, new RenderLayer(LAYER_AFTER_OPAQUE, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_MATERIAL));
    renderLayersMap.Insert(LAYER_ALPHA_TEST_LAYER, new RenderLayer(LAYER_ALPHA_TEST_LAYER, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE));
    
    renderLayersMap.Insert(LAYER_TRANSLUCENT, new RenderLayer(LAYER_TRANSLUCENT, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE));
    renderLayersMap.Insert(LAYER_AFTER_TRANSLUCENT, new RenderLayer(LAYER_AFTER_TRANSLUCENT, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE));
    
    renderLayersMap.Insert(LAYER_SHADOW_VOLUME, new RenderLayer(LAYER_SHADOW_VOLUME, 0));
    
    RenderPass * forwardPass = renderPassesMap[PASS_FORWARD];
    forwardPass->AddRenderLayer(renderLayersMap[LAYER_OPAQUE], LAST_LAYER);
	forwardPass->AddRenderLayer(renderLayersMap[LAYER_AFTER_OPAQUE], LAST_LAYER);
	forwardPass->AddRenderLayer(renderLayersMap[LAYER_ALPHA_TEST_LAYER], LAST_LAYER);
    forwardPass->AddRenderLayer(renderLayersMap[LAYER_TRANSLUCENT], LAST_LAYER);
	forwardPass->AddRenderLayer(renderLayersMap[LAYER_AFTER_TRANSLUCENT], LAST_LAYER);

    RenderPass * shadowVolumePass = renderPassesMap[PASS_SHADOW_VOLUME];
    shadowVolumePass->AddRenderLayer(renderLayersMap[LAYER_SHADOW_VOLUME], LAST_LAYER);

    renderPassOrder.push_back(renderPassesMap[PASS_FORWARD]);
    renderPassOrder.push_back(renderPassesMap[PASS_SHADOW_VOLUME]);

	particleEmitterSystem = new ParticleEmitterSystem();
    renderHierarchy = new QuadTree(10);
	hierarchyInitialized = false;
    globalBatchArray = new RenderPassBatchArray();
	
	for(FastNameMap<RenderLayer*>::iterator it = renderLayersMap.begin();
		it != renderLayersMap.end();
		++it)
	{
		globalBatchArray->InitLayer(it->first, it->second->GetFlags());
	}
	
	materialSystem = new MaterialSystem();
	materialSystem->SetDefaultMaterialQuality(FastName("Normal")); //TODO: add code setting material quality based on device specs
	materialSystem->LoadMaterialConfig("~res:/Materials/MaterialTree.config");

	markedObjects.reserve(100);
}

RenderSystem::~RenderSystem()
{
    SafeDelete(globalBatchArray);
    SafeDelete(renderHierarchy);
	SafeDelete(particleEmitterSystem);
    
    FastNameMap<RenderPass*>::iterator endPasses = renderPassesMap.end();
    for(FastNameMap<RenderPass*>::iterator it = renderPassesMap.begin(); it != endPasses; ++it)
    {
        RenderPass *pass = it->second;
        SafeDelete(pass);
    }
    renderPassesMap.clear();
    
    FastNameMap<RenderLayer*>::iterator endLayers = renderLayersMap.end();
    for(FastNameMap<RenderLayer*>::iterator it = renderLayersMap.begin(); it != endLayers; ++it)
    {
        RenderLayer *layer = it->second;
        SafeDelete(layer);
    }
    renderLayersMap.clear();
}
    

void RenderSystem::RenderPermanent(RenderObject * renderObject)
{
    DVASSERT(renderObject->GetRemoveIndex() == -1);
    
	/*on add calculate valid world bbox*/	
    renderObject->Retain();
    renderObjectArray.push_back(renderObject);
    renderObject->SetRemoveIndex((uint32)(renderObjectArray.size() - 1));
    
    AddRenderObject(renderObject);	
//    uint32 renderBatchCount = renderObject->GetRenderBatchCount();
//    for (uint32 k = 0; k < renderBatchCount; ++k)
//    {
//        RenderBatch * batch = renderObject->GetRenderBatch(k);
//        AddRenderBatch(batch);
//    }
}

void RenderSystem::RemoveFromRender(RenderObject * renderObject)
{
    DVASSERT(renderObject->GetRemoveIndex() != -1);
    
//	uint32 renderBatchCount = renderObject->GetRenderBatchCount();
//	for (uint32 k = 0; k < renderBatchCount; ++k)
//	{
//		RenderBatch * batch = renderObject->GetRenderBatch(k);
//		RemoveRenderBatch(batch);
//	}

	FindAndRemoveExchangingWithLast(markedObjects, renderObject);

	RenderObject * lastRenderObject = renderObjectArray[renderObjectArray.size() - 1];
    renderObjectArray[renderObject->GetRemoveIndex()] = lastRenderObject;
    renderObjectArray.pop_back();
	lastRenderObject->SetRemoveIndex(renderObject->GetRemoveIndex());
    renderObject->SetRemoveIndex(-1);
    
    RemoveRenderObject(renderObject);

	renderObject->Release();
}

void RenderSystem::AddRenderObject(RenderObject * renderObject)
{
	renderObject->RecalculateWorldBoundingBox();						
	renderHierarchy->AddRenderObject(renderObject);

	particleEmitterSystem->AddIfEmitter(renderObject);
	renderObject->SetRenderSystem(this);	
}

void RenderSystem::RemoveRenderObject(RenderObject * renderObject)
{
	renderHierarchy->RemoveRenderObject(renderObject);
    particleEmitterSystem->RemoveIfEmitter(renderObject);
	renderObject->SetRenderSystem(0);	
}

    
RenderPass * RenderSystem::GetRenderPass(const FastName & passName)
{
	return renderPassesMap[passName];
}
    
void RenderSystem::MarkForUpdate(RenderObject * renderObject)
{
	uint32 flags = renderObject->GetFlags();
	if (flags&RenderObject::MARKED_FOR_UPDATE) return;
	flags|=RenderObject::NEED_UPDATE;
	if ((flags&RenderObject::CLIPPING_VISIBILITY_CRITERIA) == RenderObject::CLIPPING_VISIBILITY_CRITERIA)
	{
		markedObjects.push_back(renderObject);
		flags|=RenderObject::MARKED_FOR_UPDATE;
	}
	renderObject->SetFlags(flags);
}
  
void RenderSystem::MarkForUpdate(Light * lightNode)
{
    movedLights.push_back(lightNode);
}
    
void RenderSystem::RegisterForUpdate(IRenderUpdatable * updatable)
{
    objectsForUpdate.push_back(updatable);
}
    
void RenderSystem::UnregisterFromUpdate(IRenderUpdatable * updatable)
{
    uint32 size = objectsForUpdate.size();
	for(uint32 i = 0; i < size; ++i)
	{
		if(objectsForUpdate[i] == updatable)
		{
			objectsForUpdate[i] = objectsForUpdate[size - 1];
			objectsForUpdate.pop_back();
			return;
		}
	}
}
    
//void RenderSystem::MarkForMaterialSort(Material * material)
//{
//    //for (FastNameMap<RenderLayer*>::Iterator it = renderLayersMap.Begin(); it != )
//}

    
void RenderSystem::FindNearestLights(RenderObject * renderObject)
{
	//do not calculate nearest lights for non-lit objects
	bool needUpdate = false;
	uint32 renderBatchCount = renderObject->GetRenderBatchCount();
    for (uint32 k = 0; k < renderBatchCount; ++k)
    {
        RenderBatch * batch = renderObject->GetRenderBatch(k);
        NMaterial * material = batch->GetMaterial();
        if (material && material->IsDynamicLit())
        {
			needUpdate = true;
			break;
		}
	}
	
	if(!needUpdate)
	{
		return;
	}
	
    Light * nearestLight = 0;
    float32 squareMinDistance = 10000000.0f;
    Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
    
    uint32 size = lights.size();
	
	if(1 == size)
	{
		nearestLight = lights[0];
	}
	else
	{
		for (uint32 k = 0; k < size; ++k)
		{
			Light * light = lights[k];
			
			if (!light->IsDynamic())continue;
			
			const Vector3 & lightPosition = light->GetPosition();
			
			float32 squareDistanceToLight = (position - lightPosition).SquareLength();
			if (squareDistanceToLight < squareMinDistance)
			{
				squareMinDistance = squareDistanceToLight;
				nearestLight = light;
			}
		}
	}
    
    for (uint32 k = 0; k < renderBatchCount; ++k)
    {
        RenderBatch * batch = renderObject->GetRenderBatch(k);
        NMaterial * material = batch->GetMaterial();
        if (material)
        {
            material->SetLight(0, nearestLight);
        }
    }
}

void RenderSystem::FindNearestLights()
{
    uint32 size = (uint32)renderObjectArray.size();
    for (uint32 k = 0; k < size; ++k)
    {
        FindNearestLights(renderObjectArray[k]);
    }
}
    
void RenderSystem::AddLight(Light * light)
{
    lights.push_back(SafeRetain(light));
    FindNearestLights();
}
    
void RenderSystem::RemoveLight(Light * light)
{
    lights.erase(std::remove(lights.begin(), lights.end(), light), lights.end());
}

Vector<Light*> & RenderSystem::GetLights()
{
    return lights;
}

void RenderSystem::Update(float32 timeElapsed)
{
	if (!hierarchyInitialized)
	{
		renderHierarchy->Initialize();
		hierarchyInitialized = true;
	}
	if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_PARTICLE_EMMITERS))
	{
		particleEmitterSystem->Update(timeElapsed, clipCamera);
	}
	
    int32 objectBoxesUpdated = 0;
    Vector<RenderObject*>::iterator end = markedObjects.end();
    for (Vector<RenderObject*>::iterator it = markedObjects.begin(); it != end; ++it)
    {
        RenderObject * obj = *it;
		
		obj->RecalculateWorldBoundingBox();
		
		FindNearestLights(obj);
		if (obj->GetTreeNodeIndex()!=INVALID_TREE_NODE_INDEX)
			renderHierarchy->ObjectUpdated(obj);
		
		obj->RemoveFlag(RenderObject::NEED_UPDATE | RenderObject::MARKED_FOR_UPDATE);
        objectBoxesUpdated++;
    }
    markedObjects.clear();

	renderHierarchy->Update();
	
    if (movedLights.size() > 0)
    {
        FindNearestLights();
    }
    movedLights.clear();
    
    globalBatchArray->Clear();
    renderHierarchy->Clip(clipCamera, globalBatchArray);
	
	uint32 size = objectsForUpdate.size();
	for(uint32 i = 0; i < size; ++i)
	{
        objectsForUpdate[i]->RenderUpdate(clipCamera, timeElapsed);
    }
}

void RenderSystem::DebugDrawHierarchy(const Matrix4& cameraMatrix)
{
	if (renderHierarchy)
		renderHierarchy->DebugDraw(cameraMatrix);
}

void RenderSystem::Render()
{
    TIME_PROFILE("RenderSystem::Render");
    uint32 size = (uint32)renderPassOrder.size();
    for (uint32 k = 0; k < size; ++k)
    {
        renderPassOrder[k]->Draw(camera, globalBatchArray);
    }
    
    //Logger::FrameworkDebug("OccludedRenderBatchCount: %d", RenderManager::Instance()->GetStats().occludedRenderBatchCount);
}

RenderLayer * RenderSystem::AddRenderLayer(const FastName & layerName, uint32 sortingFlags, const FastName & passName, const FastName & afterLayer)
{
	DVASSERT(false == renderLayersMap.count(layerName));

	RenderLayer * newLayer = new RenderLayer(layerName, sortingFlags);
	renderLayersMap.Insert(layerName, newLayer);

	RenderPass * inPass = renderPassesMap[passName];
	inPass->AddRenderLayer(newLayer, afterLayer);

	return newLayer;
}
    
void RenderSystem::SetShadowRectColor(const Color &color)
{
    ShadowVolumeRenderPass *shadowVolume = static_cast<ShadowVolumeRenderPass *>(renderPassesMap[PASS_SHADOW_VOLUME]);
    DVASSERT(shadowVolume);

    ShadowRect *shadowRect = shadowVolume->GetShadowRect();
    DVASSERT(shadowRect);

    shadowRect->SetColor(color);
}
    
const Color & RenderSystem::GetShadowRectColor()
{
    ShadowVolumeRenderPass *shadowVolume = static_cast<ShadowVolumeRenderPass *>(renderPassesMap[PASS_SHADOW_VOLUME]);
    DVASSERT(shadowVolume);
    
    ShadowRect *shadowRect = shadowVolume->GetShadowRect();
    DVASSERT(shadowRect);
    
    return shadowRect->GetColor();
}
};