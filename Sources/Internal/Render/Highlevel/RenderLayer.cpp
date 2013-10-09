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


#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/Camera.h"
#include "Base/Radix/Radix.h"

namespace DAVA
{
RenderLayer::RenderLayer(const FastName & _name)
    : name(_name)
    , flags(0)
{
    flags = SORT_ENABLED | SORT_BY_MATERIAL | VISIBLE;
}
    
RenderLayer::~RenderLayer()
{
    
}

void RenderLayer::AddRenderBatch(RenderBatch * batch)
{
    DVASSERT(batch->GetRemoveIndex() == -1)
    renderBatchArray.push_back(batch);
    batch->SetRemoveIndex(this, renderBatchArray.size() - 1);
    flags |= SORT_REQUIRED;
}

void RenderLayer::RemoveRenderBatch(RenderBatch * batch)
{
    DVASSERT(batch->GetRemoveIndex() != -1)
	uint32 oldIndex = batch->GetRemoveIndex();
    renderBatchArray[oldIndex] = renderBatchArray[renderBatchArray.size() - 1];
	renderBatchArray[oldIndex]->SetRemoveIndex(this, oldIndex);
    renderBatchArray.pop_back();
    batch->SetRemoveIndex(0, -1);
}
    
bool RenderLayer::MaterialCompareFunction(const RenderBatchSortItem & a, const RenderBatchSortItem & b)
{
    return a.sortingKey > b.sortingKey;
}

void RenderLayer::Update(Camera * camera)
{
    // Need sort
    if ((flags & SORT_THIS_FRAME) == SORT_THIS_FRAME)
    {
        uint32 renderBatchCount = (uint32)renderBatchArray.size();
        sortArray.resize(renderBatchCount);
        if (flags & SORT_BY_MATERIAL)
        {
            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatchSortItem & item = sortArray[k];
                RenderBatch * batch = renderBatchArray[k];;
                item.renderBatch = batch;
                item.sortingKey = ((pointer_size)renderBatchArray[k]->GetMaterial() & 0x0fffffff) | (batch->GetSortingKey() << 28);
            }
            
            std::stable_sort(sortArray.begin(), sortArray.end(), MaterialCompareFunction);
            
            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatchSortItem & item = sortArray[k];
                renderBatchArray[k] = item.renderBatch;
                item.renderBatch->SetRemoveIndex(this, k);
            }
            flags &= ~SORT_REQUIRED;
        }
        
        if (flags & SORT_BY_DISTANCE)
        {
            Vector3 cameraPosition = camera->GetPosition();
            
            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatchSortItem & item = sortArray[k];
                RenderBatch * batch = renderBatchArray[k];;
                item.renderBatch = batch;

                item.sortingKey = 0;
                if (batch)
                {
                    RenderObject * renderObject = batch->GetRenderObject();
                    Vector3 position = renderObject->GetBoundingBox().GetCenter();
                    float32 distance = (position - cameraPosition).Length();
                    
                    item.sortingKey = (((uint32)distance) & 0x0fffffff) | (batch->GetSortingKey() << 24);
                }
            }
            
            std::stable_sort(sortArray.begin(), sortArray.end(), MaterialCompareFunction);
            
            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatchSortItem & item = sortArray[k];
                renderBatchArray[k] = item.renderBatch;
                item.renderBatch->SetRemoveIndex(this, k);
            }
            
            flags |= SORT_REQUIRED;
        }
    }
    
    
    
}
    
uint32 RenderLayer::GetRenderBatchCount()
{
    return (uint32)renderBatchArray.size();
}

void RenderLayer::Draw(Camera * camera)
{
	if(flags & VISIBLE)
	{
		Update(camera);
		uint32 size = (uint32)renderBatchArray.size();
		for (uint32 k = 0; k < size; ++k)
		{
			renderBatchArray[k]->Draw(camera);
		}
#if 0
		Logger::FrameworkDebug("Layer: %s Objects: %d", name.c_str(), renderBatchArray.size());
#endif
	}
}

const FastName & RenderLayer::GetName()
{
	return name;
}

void RenderLayer::SetVisible(bool visible)
{
	if(visible)
	{
		flags |= VISIBLE;
	}
	else
	{
		flags &= ~VISIBLE;
	}
}

bool RenderLayer::GetVisible()
{
	return (0 != (flags & VISIBLE));
}

};
