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
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/ShadowVolumeRenderPass.h"
#include "Render/Highlevel/ShadowRect.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/Systems/ParticleEmitterSystem.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "Utils/Utils.h"

namespace DAVA
{

RenderSystem::RenderSystem()
{
    // Register available passes & layers
    renderPassesMap.Insert(PASS_FORWARD, new RenderPass(PASS_FORWARD));
    renderPassesMap.Insert(PASS_SHADOW_VOLUME, new ShadowVolumeRenderPass(PASS_SHADOW_VOLUME));

    renderLayersMap.Insert(LAYER_OPAQUE, new RenderLayer(LAYER_OPAQUE));
	renderLayersMap.Insert(LAYER_AFTER_OPAQUE, new RenderLayer(LAYER_AFTER_OPAQUE));
    renderLayersMap.Insert(LAYER_ALPHA_TEST_LAYER, new RenderLayer(LAYER_ALPHA_TEST_LAYER));
    
    renderLayersMap.Insert(LAYER_TRANSLUCENT, new RenderLayer(LAYER_TRANSLUCENT));
    renderLayersMap.Insert(LAYER_AFTER_TRANSLUCENT, new RenderLayer(LAYER_AFTER_TRANSLUCENT));
    
    renderLayersMap.Insert(LAYER_SHADOW_VOLUME, new RenderLayer(LAYER_SHADOW_VOLUME));
    
    
    RenderPass * forwardPass = renderPassesMap[PASS_FORWARD];
    forwardPass->AddRenderLayer(renderLayersMap[LAYER_OPAQUE], LAST_LAYER);
	forwardPass->AddRenderLayer(renderLayersMap[LAYER_AFTER_OPAQUE], LAST_LAYER);
    forwardPass->AddRenderLayer(renderLayersMap[LAYER_TRANSLUCENT], LAST_LAYER);

    ShadowVolumeRenderPass * shadowVolumePass = (ShadowVolumeRenderPass*)renderPassesMap[PASS_SHADOW_VOLUME];
    shadowVolumePass->AddRenderLayer(renderLayersMap[LAYER_SHADOW_VOLUME], LAST_LAYER);

    renderPassOrder.push_back(renderPassesMap[PASS_FORWARD]);
    renderPassOrder.push_back(renderPassesMap[PASS_SHADOW_VOLUME]);

	particleEmitterSystem = new ParticleEmitterSystem();

	markedObjects.reserve(100);

	spatialTree = NULL;
}

RenderSystem::~RenderSystem()
{
	SafeDelete(particleEmitterSystem);
    
    FastNameMap<RenderPass*>::Iterator endPasses = renderPassesMap.End();
    for(FastNameMap<RenderPass*>::Iterator it = renderPassesMap.Begin(); it != endPasses; ++it)
    {
        RenderPass *pass = it.GetValue();
        SafeDelete(pass);
    }
    renderPassesMap.Clear();
    
    FastNameMap<RenderLayer*>::Iterator endLayers = renderLayersMap.End();
    for(FastNameMap<RenderLayer*>::Iterator it = renderLayersMap.Begin(); it != endLayers; ++it)
    {
        RenderLayer *layer = it.GetValue();
        SafeDelete(layer);
    }
    renderLayersMap.Clear();

	SafeRelease(spatialTree);
}
    

void RenderSystem::RenderPermanent(RenderObject * renderObject)
{
    DVASSERT(renderObject->GetRemoveIndex() == -1);
    
    renderObject->Retain();
    renderObjectArray.push_back(renderObject);
    renderObject->SetRemoveIndex((uint32)(renderObjectArray.size() - 1));
    
    AddRenderObject(renderObject);
    
    uint32 renderBatchCount = renderObject->GetRenderBatchCount();
    for (uint32 k = 0; k < renderBatchCount; ++k)
    {
        RenderBatch * batch = renderObject->GetRenderBatch(k);
        AddRenderBatch(batch);
    }
}

void RenderSystem::RemoveFromRender(RenderObject * renderObject)
{
    DVASSERT(renderObject->GetRemoveIndex() != -1);

	uint32 renderBatchCount = renderObject->GetRenderBatchCount();
	for (uint32 k = 0; k < renderBatchCount; ++k)
	{
		RenderBatch * batch = renderObject->GetRenderBatch(k);
		RemoveRenderBatch(batch);
	}

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
	particleEmitterSystem->AddIfEmitter(renderObject);

	/*on add calculate valid world bbox*/
	renderObject->RecalculateWorldBoundingBox();

	if (spatialTree&&!renderObject->GetBoundingBox().IsEmpty())
	{
		if (!(renderObject->GetFlags()&RenderObject::ALWAYS_CLIPPING_VISIBLE))
			spatialTree->AddObject(renderObject);
	}
	renderObject->SetRenderSystem(this);
	
}

void RenderSystem::RemoveRenderObject(RenderObject * renderObject)
{
    particleEmitterSystem->RemoveIfEmitter(renderObject);
	if (renderObject->GetTreeNodeIndex() != INVALID_TREE_NODE_INDEX)
		spatialTree->RemoveObject(renderObject);
	renderObject->SetRenderSystem(0);	
}

void RenderSystem::AddRenderBatch(RenderBatch * renderBatch)
{
    // Get Layer Name
    const FastName & name = renderBatch->GetOwnerLayerName();

    RenderLayer * oldLayer = renderBatch->GetOwnerLayer();
    if (oldLayer != 0)
    {
        oldLayer->RemoveRenderBatch(renderBatch);
    }
    RenderLayer * layer = renderLayersMap[name];
    layer->AddRenderBatch(renderBatch);
}
    
void RenderSystem::RemoveRenderBatch(RenderBatch * renderBatch)
{
    RenderLayer * oldLayer = renderBatch->GetOwnerLayer();
    if (oldLayer != 0)
    {
        oldLayer->RemoveRenderBatch(renderBatch);
    }
}

RenderPass * RenderSystem::GetRenderPass(const FastName & passName)
{
	return renderPassesMap[passName];
}

void RenderSystem::ImmediateUpdateRenderBatch(RenderBatch * renderBatch)
{
    AddRenderBatch(renderBatch);
}
    
void RenderSystem::SetCamera(Camera * _camera)
{
    camera = _camera;
}

Camera * RenderSystem::GetCamera()
{
	return camera;
}

void RenderSystem::CreateSpatialTree(const AABBox3 &worldBox, int32 maxTreeDepth)
{
	if (spatialTree)
		RemoveSpatialTree();

	spatialTree = new QuadTree(worldBox, maxTreeDepth);	

	uint32 size = renderObjectArray.size();
	for (uint32 pos = 0; pos < size; ++pos)
	{
		if (!(renderObjectArray[pos]->GetFlags()&RenderObject::ALWAYS_CLIPPING_VISIBLE))
		{
			spatialTree->AddObject(renderObjectArray[pos]);
		}

	}	
}

void RenderSystem::RemoveSpatialTree()
{
	SafeRelease(spatialTree);
	uint32 size = renderObjectArray.size();
	for (uint32 pos = 0; pos < size; ++pos)
		renderObjectArray[pos]->SetTreeNodeIndex(INVALID_TREE_NODE_INDEX);
}
//

void RenderSystem::CreateSpatialTree()
{	
	AABBox3 worldBox;	
	uint32 size = renderObjectArray.size();
	for (uint32 pos = 0; pos < size; ++pos)
	{
		if (!(renderObjectArray[pos]->GetFlags()&RenderObject::ALWAYS_CLIPPING_VISIBLE))
			worldBox.AddAABBox(renderObjectArray[pos]->GetWorldBoundingBox());				
	}	
	if (worldBox.IsEmpty())
		worldBox = AABBox3(Vector3(0,0,0), Vector3(0,0,0));
	CreateSpatialTree(worldBox, 10);	
}

void RenderSystem::DebugDrawSpatialTree()
{
	if (spatialTree)
		spatialTree->DebugDraw();
}
    
void RenderSystem::ProcessClipping()
{
    int32 objectBoxesUpdated = 0;
    Vector<RenderObject*>::iterator end = markedObjects.end();
    for (Vector<RenderObject*>::iterator it = markedObjects.begin(); it != end; ++it)
    {
        RenderObject * obj = *it;

		obj->RecalculateWorldBoundingBox();
		
		FindNearestLights(obj);
		if (obj->GetTreeNodeIndex()!=INVALID_TREE_NODE_INDEX)					
			spatialTree->ObjectUpdated(obj);		
		
        objectBoxesUpdated++;
    }
    markedObjects.clear();
    
    if (movedLights.size() > 0)
    {
        FindNearestLights();
    }
    movedLights.clear();       

	if (!spatialTree)
		CreateSpatialTree();

	if (spatialTree)
		spatialTree->UpdateTree();
    
    Frustum * frustum = camera->GetFrustum();
	int32 objectsToClip = 0;
    uint32 size = renderObjectArray.size();
    for (uint32 pos = 0; pos < size; ++pos)
    {
        RenderObject * node = renderObjectArray[pos];
				
		if ((node->GetFlags()&RenderObject::CLIPPING_VISIBILITY_CRITERIA) != RenderObject::CLIPPING_VISIBILITY_CRITERIA)
		{
			continue;
		}
		node->RemoveFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
		if (node->GetTreeNodeIndex()==INVALID_TREE_NODE_INDEX) //process clipping if not in spatial tree for some reason (eg. no SpatialTree)
		{						
			if ((RenderObject::ALWAYS_CLIPPING_VISIBLE&node->GetFlags()) || frustum->IsInside(node->GetWorldBoundingBox()))
			{
				node->AddFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
			}
		}
		else
			objectsToClip++;
        
    }
	if (spatialTree)
		spatialTree->ProcessClipping(frustum);
	

	//spatial tree validation test
	/*int32 mismatchCount = 0;
	for (uint32 pos = 0; pos < size; ++pos)
	{
		RenderObject * node = renderObjectArray[pos];

		if ((node->GetFlags()&RenderObject::CLIPPING_VISIBILITY_CRITERIA) != RenderObject::CLIPPING_VISIBILITY_CRITERIA)
		{
			continue;
		}
		if (RenderObject::ALWAYS_CLIPPING_VISIBLE&node->GetFlags())	
			continue;			
		bool treeClippingResult = node->GetFlags()&RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME;
		bool frustumClippingResult = frustum->IsInside(node->GetWorldBoundingBox());
		if (treeClippingResult!=frustumClippingResult)
		{
			mismatchCount++;
		}		

	}*/
}

void RenderSystem::MarkForUpdate(RenderObject * renderObject)
{
    markedObjects.push_back(renderObject);
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
    Light * nearestLight = 0;
    float32 squareMinDistance = 10000000.0f;
    Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
    
    uint32 size = lights.size();
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
    
    uint32 renderBatchCount = renderObject->GetRenderBatchCount();
    for (uint32 k = 0; k < renderBatchCount; ++k)
    {
        RenderBatch * batch = renderObject->GetRenderBatch(k);
        batch->GetMaterialInstance()->SetLight(0, nearestLight);
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
    Vector<Light *>::const_iterator endIt = lights.end();
    for(Vector<Light *>::iterator it = lights.begin(); it != endIt; ++it)
    {
        if(*it == light)
        {
            SafeRelease(*it);
            lights.erase(it);
            break;
        }
    }
}

Vector<Light*> & RenderSystem::GetLights()
{
    return lights;
}

void RenderSystem::Update(float32 timeElapsed)
{
	if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_PARTICLE_EMMITERS))
	{
		particleEmitterSystem->Update(timeElapsed, camera);
	}

	if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::PROCESS_CLIPPING))
	{
		ProcessClipping();
	}
    
    uint32 size = objectsForUpdate.size();
	for(uint32 i = 0; i < size; ++i)
	{
        objectsForUpdate[i]->RenderUpdate(camera, timeElapsed);
    }

}

void RenderSystem::Render()
{
    uint32 size = (uint32)renderPassOrder.size();
    for (uint32 k = 0; k < size; ++k)
    {
        renderPassOrder[k]->Draw(camera);
    }
}

RenderLayer * RenderSystem::AddRenderLayer(const FastName & layerName, const FastName & passName, const FastName & afterLayer)
{
	DVASSERT(false == renderLayersMap.IsKey(layerName));

	RenderLayer * newLayer = new RenderLayer(layerName);
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