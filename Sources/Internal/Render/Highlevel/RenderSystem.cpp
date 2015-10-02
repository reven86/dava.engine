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
#include "Render/Highlevel/RenderBatch.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Render/Highlevel/SpatialTree.h"
#include "Render/ShaderCache.h"

#include "Utils/Utils.h"
#include "Debug/Stats.h"

namespace DAVA
{

RenderSystem::RenderSystem()
    :   forceUpdateLights(false)
    ,   mainCamera(0)
    ,   drawCamera(0)
    ,   globalMaterial(NULL)
{
    mainRenderPass = new MainForwardRenderPass(PASS_FORWARD);

    renderHierarchy = new QuadTree(10);
	hierarchyInitialized = false;   
	markedObjects.reserve(100);

    debugDrawer = new RenderHelper();
}

RenderSystem::~RenderSystem()
{
    SafeRelease(mainCamera);
    SafeRelease(drawCamera);

    SafeRelease(globalMaterial);
    
    SafeDelete(renderHierarchy);	
    SafeDelete(mainRenderPass);

    SafeDelete(debugDrawer);
}
    

void RenderSystem::RenderPermanent(RenderObject * renderObject)
{
    DVASSERT(renderObject->GetRemoveIndex() == static_cast<uint32>(-1));
    
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
    DVASSERT(renderObject->GetRemoveIndex() != static_cast<uint32>(-1));
    
//	uint32 renderBatchCount = renderObject->GetRenderBatchCount();
//	for (uint32 k = 0; k < renderBatchCount; ++k)
//	{
//		RenderBatch * batch = renderObject->GetRenderBatch(k);
//		RemoveRenderBatch(batch);
//	}

	FindAndRemoveExchangingWithLast(markedObjects, renderObject);
	renderObject->RemoveFlag(RenderObject::MARKED_FOR_UPDATE);	

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

	renderObject->SetRenderSystem(this);
    
	uint32 size = renderObject->GetRenderBatchCount();
	for(uint32 i = 0; i < size; ++i)
	{
        RenderBatch *batch = renderObject->GetRenderBatch(i);
        RegisterBatch(batch);
    }
}

void RenderSystem::RemoveRenderObject(RenderObject * renderObject)
{
	uint32 size = renderObject->GetRenderBatchCount();
	for(uint32 i = 0; i < size; ++i)
	{
        RenderBatch *batch = renderObject->GetRenderBatch(i);
        UnregisterBatch(batch);
    }

	renderHierarchy->RemoveRenderObject(renderObject);
	renderObject->SetRenderSystem(0);	
}
    
void RenderSystem::RegisterBatch(RenderBatch * batch)
{
    RegisterMaterial(batch->GetMaterial());
}
    
void RenderSystem::UnregisterBatch(RenderBatch * batch)
{
    UnregisterMaterial(batch->GetMaterial());
}
    
void RenderSystem::RegisterMaterial(NMaterial * material)
{     
    NMaterial * topParent = nullptr;

    while (nullptr != material)
    {
        topParent = material;
        material = material->GetParent();
    }

    // set globalMaterial to be parent for top material
    if (nullptr != topParent && topParent != globalMaterial)
    {
        topParent->SetParent(globalMaterial);
    }
}
    
void RenderSystem::UnregisterMaterial(NMaterial * material)
{
    /*
    if (!material) return;

    while (material->GetParent() && material->GetParent() != globalMaterial)
    {
        material = material->GetParent();
    }

    if (material->GetParent())
    {
        material->SetParent(nullptr);
    }
    */
}
    
void RenderSystem::SetGlobalMaterial(NMaterial * newGlobalMaterial)
{
    Set<DataNode*> dataNodes;
    for (RenderObject* obj : renderObjectArray)
    {
        obj->GetDataNodes(dataNodes);
    }
    for (DataNode* dataNode : dataNodes)
    {
        NMaterial* batchMaterial = dynamic_cast<NMaterial*>(dataNode);
        if (batchMaterial)
        {
            while (batchMaterial->GetParent() && batchMaterial->GetParent() != globalMaterial && batchMaterial->GetParent() != newGlobalMaterial)
            {
                batchMaterial = batchMaterial->GetParent();
            }
            batchMaterial->SetParent(newGlobalMaterial);
        }
    }

    SafeRelease(globalMaterial);
    globalMaterial = SafeRetain(newGlobalMaterial);
}

NMaterial* RenderSystem::GetGlobalMaterial() const
{
    return globalMaterial;
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
    uint32 size = static_cast<uint32>(objectsForUpdate.size());
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
    
    uint32 size = static_cast<uint32>(lights.size());
	
	if(1 == size)
	{
		nearestLight = (lights[0] && lights[0]->IsDynamic()) ? lights[0] : NULL;
	}
	else
	{
		for (uint32 k = 0; k < size; ++k)
		{
			Light * light = lights[k];
			
			if (!light->IsDynamic())continue;
			
			const Vector3 & lightPosition = light->GetPosition();
			
			float32 squareDistanceToLight = (position - lightPosition).SquareLength();
			if ((!nearestLight) || (squareDistanceToLight < squareMinDistance))
			{
				squareMinDistance = squareDistanceToLight;
				nearestLight = light;
			}
		}
	}
    
    renderObject->SetLight(0, nearestLight);    
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
    FindAndRemoveExchangingWithLast(lights, light);
    FindNearestLights();
    
    SafeRelease(light);
}

Vector<Light*> & RenderSystem::GetLights()
{
    return lights;
}

void RenderSystem::SetForceUpdateLights()
{
    forceUpdateLights = true;
}

void RenderSystem::Update(float32 timeElapsed)
{
	if (!hierarchyInitialized)
	{
		renderHierarchy->Initialize();
		hierarchyInitialized = true;
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
	
    if (movedLights.size() > 0 || forceUpdateLights)
    {
        FindNearestLights();
        
        forceUpdateLights = false;
		movedLights.clear();
    }
    
	uint32 size = static_cast<uint32>(objectsForUpdate.size());
	for(uint32 i = 0; i < size; ++i)
	{
        objectsForUpdate[i]->RenderUpdate(mainCamera, timeElapsed);
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

    mainRenderPass->Draw(this);
}

};