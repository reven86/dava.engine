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

namespace DAVA
{
    
RenderPassBatchArray::RenderPassBatchArray()
    :	layerBatchArrayMap(8)
{
}
    
RenderPassBatchArray::~RenderPassBatchArray()
{
    HashMap<FastName, RenderLayerBatchArray*>::iterator end = layerBatchArrayMap.end();
    for (HashMap<FastName, RenderLayerBatchArray*>::iterator it = layerBatchArrayMap.begin(); it != end; ++it)
    {
        RenderLayerBatchArray * layer = it->second;
        SafeDelete(layer);
    }
}

void RenderPassBatchArray::Clear()
{
    HashMap<FastName, RenderLayerBatchArray*>::iterator end = layerBatchArrayMap.end();
    for (HashMap<FastName, RenderLayerBatchArray*>::iterator it = layerBatchArrayMap.begin(); it != end; ++it)
    {
        it->second->Clear();
    }    
}
    
RenderLayerBatchArray * RenderPassBatchArray::Get(const FastName & name)
{
    return layerBatchArrayMap.at(name);
}

void RenderPassBatchArray::InitLayer(const FastName& layerName, uint32 sortingFlags)
{
	RenderLayerBatchArray* batchArray = new RenderLayerBatchArray(sortingFlags);
	layerBatchArrayMap.insert(layerName, batchArray);
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
            Vector3 cameraPosition = camera->GetPosition();

            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatch * batch = renderBatchArray[k];
                RenderObject * renderObject = batch->GetRenderObject();
                Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
                float32 distance = (position - cameraPosition).Length();
                uint32 distanceBits = 0xFFFF - ((uint32)distance) & 0xFFFF;
                uint32 materialIndex = (batch->GetMaterial()->GetParent()->GetMaterialName().Index() & 0xFFF);
                batch->layerSortingKey = (pointer_size)((batch->GetSortingKey() << 28) | (materialIndex << 16) | (distanceBits));
            }
            
			std::sort(renderBatchArray.begin(), renderBatchArray.end(), MaterialCompareFunction);
            
            flags &= ~SORT_REQUIRED;
        }
        else if (flags & SORT_BY_DISTANCE)
        {
            Vector3 cameraPosition = camera->GetPosition();
            
            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatch * batch = renderBatchArray[k];
                RenderObject * renderObject = batch->GetRenderObject();
                Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
                float32 distance = (position - cameraPosition).Length();
                batch->layerSortingKey = (((uint32)distance) & 0x0fffffff) | (batch->GetSortingKey() << 28);
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
