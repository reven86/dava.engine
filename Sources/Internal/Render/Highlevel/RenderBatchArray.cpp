/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Render/Highlevel/RenderBatchArray.h"
#include "Debug/Stats.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderLayerManager.h"
#include "Render/Highlevel/RenderPass.h"

namespace DAVA
{
    
RenderPassBatchArray::RenderPassBatchArray()
{
    const RenderLayerManager * manager = RenderLayerManager::Instance();
    for (RenderLayerID id = 0; id < RENDER_LAYER_ID_COUNT; ++id)
    {
        RenderLayerBatchArray* batchArray = new RenderLayerBatchArray( manager->GetRenderLayer(id)->GetFlags() );
        layerBatchArrays[id] = batchArray;
    }
}
    
void RenderPassBatchArray::InitPassLayers(RenderPass * renderPass)
{
    // const RenderLayerManager * manager = RenderLayerManager::Instance();
    for (RenderLayerID id = 0; id < RENDER_LAYER_ID_COUNT; ++id)
    {
        RenderLayer * layer = 0;
        for (uint32 k = 0; k < renderPass->GetRenderLayerCount(); ++k)
        {
            if (renderPass->GetRenderLayer(k)->GetRenderLayerID() == id)
            {
                layer = renderPass->GetRenderLayer(k);
                break;
            }
        }
        if (layer)
        {
            layerBatchArrays[id]->SetFlags(layer->GetFlags());
        }
    }
}
    
void RenderPassBatchArray::InitPassLayersWithSingleLayer(RenderPass * renderPass, RenderLayerBatchArray * singleLayer)
{
    // const RenderLayerManager * manager = RenderLayerManager::Instance();
    for (RenderLayerID id = 0; id < RENDER_LAYER_ID_COUNT; ++id)
    {
        SafeDelete(layerBatchArrays[id]);
        layerBatchArrays[id] = singleLayer;
        //->SetFlags(layer->GetFlags());
    }
}
    
RenderPassBatchArray::~RenderPassBatchArray()
{
    for (RenderLayerID id = 0; id < RENDER_LAYER_ID_COUNT; ++id)
    {
        SafeDelete(layerBatchArrays[id]);
    }
}
    
void RenderPassBatchArray::Clear()
{
    for (RenderLayerID id = 0; id < RENDER_LAYER_ID_COUNT; ++id)
    {
        layerBatchArrays[id]->Clear();
    }
}

void RenderPassBatchArray::PrepareVisibilityArray(VisibilityArray * visibilityArray, Camera * camera, const FastName& passName)
{    
    uint32 size = visibilityArray->GetCount();
    for (uint32 ro = 0; ro < size; ++ro)
    {
        RenderObject * renderObject = visibilityArray->Get(ro);
        if (renderObject->GetFlags() & RenderObject::CUSTOM_PREPARE_TO_RENDER)

		    renderObject->PrepareToRender(camera);
        //cameraWorldMatrices[ro] = camera->GetTransform() * (*renderObject->GetWorldTransformPtr());
        
        uint32 batchCount = renderObject->GetActiveRenderBatchCount();
		for (uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
		{
			RenderBatch * batch = renderObject->GetActiveRenderBatch(batchIndex);
            //batch->SetCameraWorldTransformPtr(&cameraWorldMatrices[ro]);

			NMaterial * material = batch->GetMaterial();
            DVASSERT(material);			
            if (material->PreBuildMaterial(passName))
                AddRenderBatch(material->GetRenderLayerID(), batch);			
		}
    }
}
    
    
RenderLayerBatchArray::RenderLayerBatchArray(uint32 sortingFlags)
    : flags(sortingFlags)
{
    //flags = SORT_ENABLED | SORT_BY_MATERIAL | SORT_BY_DISTANCE;
	//renderBatchArray.reserve(4096);
}
    
RenderLayerBatchArray::~RenderLayerBatchArray()
{
    
}

void RenderLayerBatchArray::Clear()
{
    renderBatchArray.clear();
}
    
bool RenderLayerBatchArray::MaterialCompareFunction(const RenderBatch * a, const RenderBatch *  b)
{
    return a->layerSortingKey > b->layerSortingKey;
}
	
void RenderLayerBatchArray::Sort(Camera * camera)
{
    TIME_PROFILE("RenderLayerBatchArray::Sort");
    
    // Need sort
	flags |= SORT_REQUIRED;
	
    if ((flags & SORT_THIS_FRAME) == SORT_THIS_FRAME)
    {
        uint32 renderBatchCount = (uint32)renderBatchArray.size();
        if (flags & SORT_BY_MATERIAL)
        {
            //Vector3 cameraPosition = camera->GetPosition();

            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatch * batch = renderBatchArray[k];
				//pointer_size renderObjectId = (pointer_size)batch->GetRenderObject();
                //RenderObject * renderObject = batch->GetRenderObject();
                //Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
                //float32 distance = (position - cameraPosition).Length();
                //uint32 distanceBits = (0xFFFF - ((uint32)distance) & 0xFFFF);
                uint32 materialIndex = batch->GetMaterial()->GetSortingKey();
				//VI: sorting key has the following layout: (m:8)(s:4)(d:20)
                //batch->layerSortingKey = (pointer_size)((materialIndex << 20) | (batch->GetSortingKey() << 28) | (distanceBits));
				batch->layerSortingKey = (pointer_size)(materialIndex | (batch->GetSortingKey() << 28));
				//batch->layerSortingKey = (pointer_size)((batch->GetMaterial()->GetSortingKey() << 20) | (batch->GetSortingKey() << 28) | (renderObjectId & 0x000FFFFF));
            }
            
			std::sort(renderBatchArray.begin(), renderBatchArray.end(), MaterialCompareFunction);
            
            flags &= ~SORT_REQUIRED;
        }
        else if (flags & SORT_BY_DISTANCE_BACK_TO_FRONT)
        {
            Vector3 cameraPosition = camera->GetPosition();
            Vector3 cameraDirection = camera->GetDirection();
            
            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatch * batch = renderBatchArray[k];
                Vector3 delta = batch->GetRenderObject()->GetWorldTransformPtr()->GetTranslationVector() - cameraPosition;                
                uint32 distance = delta.DotProduct(cameraDirection)<0?0:((uint32)(delta.Length() * 1000.0f)); //x1000.0f is to prevent resorting of nearby objects (still 26 km range)
                distance = distance + 31 - batch->GetSortingOffset();
                batch->layerSortingKey = (distance & 0x0fffffff) | (batch->GetSortingKey() << 28);
            }
            
            std::stable_sort(renderBatchArray.begin(), renderBatchArray.end(), MaterialCompareFunction);
            
            flags |= SORT_REQUIRED;
        }else if (flags & SORT_BY_DISTANCE_FRONT_TO_BACK)
        {
            Vector3 cameraPosition = camera->GetPosition();
            
            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatch * batch = renderBatchArray[k];
                RenderObject * renderObject = batch->GetRenderObject();
                Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
                uint32 distance = ((uint32)((position - cameraPosition).Length() * 100.0f)) + 31 - batch->GetSortingOffset();
                uint32 distanceBits = 0x0fffffff - distance & 0x0fffffff;

                batch->layerSortingKey = distanceBits | (batch->GetSortingKey() << 28);
            }
            
            std::sort(renderBatchArray.begin(), renderBatchArray.end(), MaterialCompareFunction);
            
            flags |= SORT_REQUIRED;
        }
    }
}
    
uint32 RenderLayerBatchArray::GetRenderBatchCount()
{
    return (uint32)renderBatchArray.size();
}

RenderBatch * RenderLayerBatchArray::Get(uint32 index)
{
    return renderBatchArray[index];
}

};
