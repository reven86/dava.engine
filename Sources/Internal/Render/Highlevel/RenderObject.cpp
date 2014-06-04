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


#include "Render/Highlevel/RenderObject.h"
#include "Base/ObjectFactory.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Utils/StringFormat.h"

namespace DAVA
{


static const int32 DEFAULT_FLAGS = RenderObject::VISIBLE | RenderObject::VISIBLE_STATIC_OCCLUSION;

RenderObject::RenderObject()
    :   type(TYPE_RENDEROBJECT)
    ,   flags(DEFAULT_FLAGS)
    ,   removeIndex(-1)
	,   treeNodeIndex(INVALID_TREE_NODE_INDEX)
    ,   staticOcclusionIndex(INVALID_STATIC_OCCLUSION_INDEX)
	,   startClippingPlane(0)
    ,   debugFlags(0)
    ,   worldTransform(0)
	,	renderSystem(0)
	,	lodIndex(-1)
	,	switchIndex(-1)
{
}
    
RenderObject::~RenderObject()
{
	uint32 size = renderBatchArray.size();
	for(uint32 i = 0; i < size; ++i)
	{
		renderBatchArray[i].renderBatch->Release();
	}
}
  
void RenderObject::AddRenderBatch(RenderBatch * batch)
{
	AddRenderBatch(batch, -1, -1);
}
  
void RenderObject::AddRenderBatch(RenderBatch * batch, int32 _lodIndex, int32 _switchIndex)
{    
	batch->Retain();
    DVASSERT((batch->GetRenderObject() == 0) || (batch->GetRenderObject() == this));
	batch->SetRenderObject(this);
    batch->SetSortingTransformPtr(worldTransform);
	
	IndexedRenderBatch ind;
	ind.lodIndex = _lodIndex;
	ind.switchIndex = _switchIndex;
	ind.renderBatch = batch;
    renderBatchArray.push_back(ind);

    if(_lodIndex == lodIndex && _switchIndex == switchIndex)
    {
        activeRenderBatchArray.push_back(batch);
    }

    if (renderSystem)
        renderSystem->RegisterBatch(batch);
            
    RecalcBoundingBox();
}

void RenderObject::UpdateBatchesSortingTransforms()
{
    for (int32 i=0, batchCount = renderBatchArray.size(); i<batchCount; ++i)
        renderBatchArray[i].renderBatch->SetSortingTransformPtr(worldTransform); 
}

void RenderObject::RemoveRenderBatch(RenderBatch * batch)
{
    batch->SetRenderObject(0);
	
	uint32 size = (uint32)renderBatchArray.size();
	for (uint32 k = 0; k < size; ++k)
	{
		if (renderBatchArray[k].renderBatch == batch)
		{
            if (renderSystem)
                renderSystem->UnregisterBatch(batch);
            batch->Release();

			renderBatchArray[k] = renderBatchArray[size - 1];
			renderBatchArray.pop_back();
            k--;
            size--;
		}
	}

	UpdateActiveRenderBatches();

    RecalcBoundingBox();
}
    
void RenderObject::RemoveRenderBatch(uint32 batchIndex)
{
	uint32 size = (uint32)renderBatchArray.size();
    DVASSERT(batchIndex < size);

    RenderBatch *batch = renderBatchArray[batchIndex].renderBatch;
    if (renderSystem)
        renderSystem->UnregisterBatch(batch);

    batch->SetRenderObject(0);
	batch->Release();

    renderBatchArray[batchIndex] = renderBatchArray[size - 1];
    renderBatchArray.pop_back();
	FindAndRemoveExchangingWithLast(activeRenderBatchArray, batch);

    RecalcBoundingBox();
}

    
void RenderObject::RecalcBoundingBox()
{
    bbox = AABBox3();
    
    uint32 size = (uint32)renderBatchArray.size();
    for (uint32 k = 0; k < size; ++k)
    {
        bbox.AddAABBox(renderBatchArray[k].renderBatch->GetBoundingBox());
    }
}

void RenderObject::CollectRenderBatches(int32 requestLodIndex, int32 requestSwitchIndex, Vector<RenderBatch*> & batches, bool includeShareLods /* = false */) const
{
    uint32 batchesCount = renderBatchArray.size();
    for(uint32 i = 0; i < batchesCount; ++i)
    {
        const IndexedRenderBatch & irb = renderBatchArray[i];
        if( (requestLodIndex == -1 || requestLodIndex == irb.lodIndex || (includeShareLods && irb.lodIndex == -1)) &&
            (requestSwitchIndex == -1 || requestSwitchIndex == irb.switchIndex) )
        {
            batches.push_back(irb.renderBatch);
        }
    }
}

RenderObject * RenderObject::Clone(RenderObject *newObject)
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<RenderObject>(this), "Can clone only RenderObject");
		newObject = new RenderObject();
	}

	newObject->type = type;
	newObject->flags = flags;
	newObject->RemoveFlag(MARKED_FOR_UPDATE);
	newObject->debugFlags = debugFlags;
    newObject->staticOcclusionIndex = staticOcclusionIndex;
	//ro->bbox = bbox;
	//ro->worldBBox = worldBBox;

	//TODO:VK: Do we need remove all renderbatches from newObject?
	DVASSERT(newObject->GetRenderBatchCount() == 0);

	uint32 size = GetRenderBatchCount();
    newObject->renderBatchArray.reserve(size);
	for(uint32 i = 0; i < size; ++i)
	{
        int32 batchLodIndex, batchSwitchIndex;
        RenderBatch *batch = GetRenderBatch(i, batchLodIndex, batchSwitchIndex)->Clone();
		newObject->AddRenderBatch(batch, batchLodIndex, batchSwitchIndex);
        batch->Release();
	}
    newObject->ownerDebugInfo = ownerDebugInfo;

	return newObject;
}

void RenderObject::Save(KeyedArchive * archive, SerializationContext* serializationContext)
{
	AnimatedObject::Save(archive);

	if(NULL != archive)
	{
		archive->SetUInt32("ro.type", type);
		archive->SetUInt32("ro.debugflags", debugFlags);
		archive->SetUInt32("ro.batchCount", GetRenderBatchCount());
        archive->SetUInt32("ro.sOclIndex", staticOcclusionIndex);
        
        //VI: save only VISIBLE flag for now. May be extended in the future
        archive->SetUInt32("ro.flags", flags & RenderObject::SERIALIZATION_CRITERIA);

		KeyedArchive *batchesArch = new KeyedArchive();
		for(uint32 i = 0; i < GetRenderBatchCount(); ++i)
		{
			RenderBatch *batch = GetRenderBatch(i);
			if(NULL != batch)
			{
                archive->SetInt32(Format("rb%d.lodIndex", i), renderBatchArray[i].lodIndex);
                archive->SetInt32(Format("rb%d.switchIndex", i), renderBatchArray[i].switchIndex);
				KeyedArchive *batchArch = new KeyedArchive();
				batch->Save(batchArch, serializationContext);
				if(batchArch->Count() > 0)
				{
					batchArch->SetString("rb.classname", batch->GetClassName());
				}
				batchesArch->SetArchive(KeyedArchive::GenKeyFromIndex(i), batchArch);
				batchArch->Release();
			} 
		}

		archive->SetArchive("ro.batches", batchesArch);
		batchesArch->Release();
	}
}

void RenderObject::Load(KeyedArchive * archive, SerializationContext *serializationContext)
{
	if(NULL != archive)
	{
		type = archive->GetUInt32("ro.type", TYPE_RENDEROBJECT);
		debugFlags = archive->GetUInt32("ro.debugflags", 0);
        staticOcclusionIndex = (uint16)archive->GetUInt32("ro.sOclIndex", INVALID_STATIC_OCCLUSION_INDEX);
        
        //VI: load only VISIBLE flag for now. May be extended in the future.
        uint32 savedFlags = RenderObject::SERIALIZATION_CRITERIA & archive->GetUInt32("ro.flags", RenderObject::SERIALIZATION_CRITERIA);
        
        flags = (savedFlags | (flags & ~RenderObject::SERIALIZATION_CRITERIA));
        
		uint32 roBatchCount = archive->GetUInt32("ro.batchCount");
        KeyedArchive *batchesArch = archive->GetArchive("ro.batches");
        for(uint32 i = 0; i < roBatchCount; ++i)
			{
                int32 batchLodIndex = archive->GetInt32(Format("rb%d.lodIndex", i), -1);
                int32 batchSwitchIndex = archive->GetInt32(Format("rb%d.switchIndex", i), -1);

				KeyedArchive *batchArch = batchesArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
				if(NULL != batchArch)
				{
					RenderBatch *batch = ObjectFactory::Instance()->New<RenderBatch>(batchArch->GetString("rb.classname"));
					
					if(NULL != batch)
					{
						batch->Load(batchArch, serializationContext);
						AddRenderBatch(batch, batchLodIndex, batchSwitchIndex);
						batch->Release();
					}
				}
			}
		}

	AnimatedObject::Load(archive);
}

void RenderObject::SetRenderSystem(RenderSystem * _renderSystem)
{
	renderSystem = _renderSystem;
}

RenderSystem * RenderObject::GetRenderSystem()
{
	return renderSystem;
}

void RenderObject::BakeGeometry(const Matrix4 & transform)
{
}

void RenderObject::RecalculateWorldBoundingBox()
{
	DVASSERT(!bbox.IsEmpty());
	bbox.GetTransformedBox(*worldTransform, worldBBox);
}


void RenderObject::PrepareToRender(Camera *camera)
{
}

void RenderObject::SetLodIndex(int32 _lodIndex)
{
	if(lodIndex != _lodIndex)
	{
		lodIndex = _lodIndex;
		UpdateActiveRenderBatches();
	}
}

void RenderObject::SetSwitchIndex(int32 _switchIndex)
{
	if(switchIndex != _switchIndex)
	{
		switchIndex = _switchIndex;
		UpdateActiveRenderBatches();
	}
}

int32 RenderObject::GetLodIndex() const
{
    return lodIndex;
}

int32 RenderObject::GetSwitchIndex() const
{
    return switchIndex;
}

void RenderObject::UpdateActiveRenderBatches()
{
	activeRenderBatchArray.clear();
	uint32 size = renderBatchArray.size();
	for(uint32 i = 0; i < size; ++i)
	{
		IndexedRenderBatch & irb = renderBatchArray[i];
		if((irb.lodIndex == lodIndex || -1 == irb.lodIndex) && (irb.switchIndex == switchIndex || -1 == irb.switchIndex))
		{
			activeRenderBatchArray.push_back(irb.renderBatch);
		}
	}
}

int32 RenderObject::GetMaxLodIndex() const
{
    int32 ret = -1;
    uint32 size = renderBatchArray.size();
    for(uint32 i = 0; i < size; ++i)
    {
        const IndexedRenderBatch & irb = renderBatchArray[i];
        ret = Max(ret, irb.lodIndex);
    }

    return ret;
}

int32 RenderObject::GetMaxLodIndexForSwitchIndex(int32 forSwitchIndex) const
{
    int32 ret = -1;
    uint32 size = renderBatchArray.size();
    for(uint32 i = 0; i < size; ++i)
    {
        const IndexedRenderBatch & irb = renderBatchArray[i];
        if(irb.switchIndex == forSwitchIndex)
        {
            ret = Max(ret, irb.lodIndex);
        }
    }

    return ret;
}

int32 RenderObject::GetMaxSwitchIndex() const
{
    int32 ret = -1;
    uint32 size = renderBatchArray.size();
    for(uint32 i = 0; i < size; ++i)
    {
        const IndexedRenderBatch & irb = renderBatchArray[i];
        ret = Max(ret, irb.switchIndex);
    }

    return ret;
}

void RenderObject::GetDataNodes(Set<DataNode*> & dataNodes)
{
    //empty by default
}

};
