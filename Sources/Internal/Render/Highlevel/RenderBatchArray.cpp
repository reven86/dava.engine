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


#include "Render/Highlevel/RenderBatchArray.h"
#include "Debug/Stats.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderPass.h"

namespace DAVA
{
RenderBatchArray::RenderBatchArray()
    : sortFlags(0)
{
    //sortFlags = SORT_ENABLED | SORT_BY_MATERIAL | SORT_BY_DISTANCE;
    //renderBatchArray.reserve(4096);
}

bool RenderBatchArray::MaterialCompareFunction(const RenderBatch* a, const RenderBatch* b)
{
    return a->layerSortingKey > b->layerSortingKey;
}

void RenderBatchArray::Sort(Camera* camera)
{
    TIME_PROFILE("RenderBatchArray::Sort");

    // Need sort
    sortFlags |= SORT_REQUIRED;

    if ((sortFlags & SORT_THIS_FRAME) == SORT_THIS_FRAME)
    {
        uint32 renderBatchCount = (uint32)renderBatchArray.size();
        if (sortFlags & SORT_BY_MATERIAL)
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

            sortFlags &= ~SORT_REQUIRED;
        }
        else if (sortFlags & SORT_BY_DISTANCE_BACK_TO_FRONT)
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

            sortFlags |= SORT_REQUIRED;
        }
        else if (sortFlags & SORT_BY_DISTANCE_FRONT_TO_BACK)
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

            sortFlags |= SORT_REQUIRED;
        }
    }
}

};
