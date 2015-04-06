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


#include "Render/Highlevel/RenderBatch.h"
#include "Render/Material.h"
#include "Render/RenderDataObject.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Scene3D/SceneFileV2.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Systems/MaterialSystem.h"
#include "Render/OcclusionQuery.h"
#include "Debug/Stats.h"

namespace DAVA
{

    
RenderBatch::RenderBatch()
    :   dataSource(0)
    ,   renderDataObject(0)
    ,   material(0)
    ,   renderObject(0)
    ,   sortingTransformPtr(NULL)
    ,   startIndex(0)
    ,   indexCount(0)
    ,   type(PRIMITIVETYPE_TRIANGLELIST)
    ,   sortingKey(SORTING_KEY_DEF_VALUE)
    ,   aabbox(Vector3(), Vector3())
{
	
#if defined(__DAVA_USE_OCCLUSION_QUERY__)
    occlusionQuery = new OcclusionQuery();
    queryRequested = -1;
    queryRequestFrame = 0;
    lastFraemDrawn = -10;
#endif        
}
    
RenderBatch::~RenderBatch()
{
#if defined(__DAVA_USE_OCCLUSION_QUERY__)
    SafeDelete(occlusionQuery);
#endif
	SafeRelease(dataSource);
	SafeRelease(renderDataObject);
		
	SafeRelease(material);
}
    
void RenderBatch::SetPolygonGroup(PolygonGroup * _polygonGroup)
{
	SafeRelease(dataSource);
    dataSource = SafeRetain(_polygonGroup);
	UpdateAABBoxFromSource();
}

void RenderBatch::SetRenderDataObject(RenderDataObject * _renderDataObject)
{
	SafeRelease(renderDataObject);
    renderDataObject = SafeRetain(_renderDataObject);
}

void RenderBatch::SetMaterial(NMaterial * _material)
{
	NMaterial* oldMat = material;
    material = SafeRetain(_material);
	SafeRelease(oldMat);
        
}    

void RenderBatch::Draw(const FastName & ownerRenderPass, Camera * camera)
{
//  TIME_PROFILE("RenderBatch::Draw");
//	if(!renderObject)return;
    DVASSERT(renderObject != 0);    	    
    
    renderObject->BindDynamicParameters(camera);
    material->BindMaterialTechnique(ownerRenderPass, camera);

	if (dataSource)
		material->Draw(dataSource);
	else
		material->Draw(renderDataObject);

}
    
void RenderBatch::SetRenderObject(RenderObject * _renderObject)
{
	renderObject = _renderObject;
}

const AABBox3 & RenderBatch::GetBoundingBox() const
{
    return aabbox;
}
    
    
void RenderBatch::SetSortingKey(uint32 _key)
{
    DVASSERT(_key<16);
    sortingKey = (sortingKey&~SORTING_KEY_MASK)+_key;
}

void RenderBatch::SetSortingOffset(uint32 offset)
{
    DVASSERT(offset<32);    
    sortingKey=(sortingKey&~SORTING_OFFSET_MASK)+(offset<<SORTING_OFFSET_SHIFT);
}


void RenderBatch::GetDataNodes(Set<DataNode*> & dataNodes)
{
#if RHI_COMPLETE
	NMaterial* curNode = material;
	while(curNode != NULL && curNode->GetMaterialType() != NMaterial::MATERIALTYPE_GLOBAL)
	{
		dataNodes.insert(curNode);
		curNode = curNode->GetParent();
	}
	
	if(dataSource)
	{
		InsertDataNode(dataSource, dataNodes);
	}
#endif //RHI_COMPLETE
}

void RenderBatch::InsertDataNode(DataNode *node, Set<DataNode*> & dataNodes)
{
	dataNodes.insert(node);

	/*for(int32 i = 0; i < node->GetChildrenNodeCount(); ++i)
	{
		InsertDataNode(node->GetChildNode(i), dataNodes);
	}*/
}

RenderBatch * RenderBatch::Clone(RenderBatch * destination)
{
    RenderBatch * rb = destination;
    if (!rb)
        rb = new RenderBatch();

    SafeRelease(rb->dataSource);
	rb->dataSource = SafeRetain(dataSource);
    
    SafeRelease(rb->renderDataObject);
	rb->renderDataObject = SafeRetain(renderDataObject);
	
    SafeRelease(rb->material);
	if(material)
	{
		NMaterial *mat = material->Clone();
		rb->SetMaterial(mat);
		mat->Release();

 		//rb->material->SetMaterialSystem(NULL);
	}

	rb->startIndex = startIndex;
	rb->indexCount = indexCount;
	rb->type = type;

	rb->aabbox = aabbox;
	rb->sortingKey = sortingKey;

	return rb;
}

void RenderBatch::Save(KeyedArchive * archive, SerializationContext* serializationContext)
{
	BaseObject::SaveObject(archive);

	if(NULL != archive)
	{
		archive->SetUInt32("rb.type", type);
		archive->SetUInt32("rb.indexCount", indexCount);
		archive->SetUInt32("rb.startIndex", startIndex);
		archive->SetVariant("rb.aabbox", VariantType(aabbox));
		archive->SetVariant("rb.datasource", VariantType((uint64)dataSource));
        archive->SetUInt32("rb.sortingKey", sortingKey);
		
		NMaterial* material = GetMaterial();
		if(material)
		{
			uint64 matKey = material->GetMaterialKey();
			archive->SetUInt64("rb.nmatname", matKey);
		}
		
		//archive->SetVariant("rb.material", VariantType((uint64)GetMaterial()));
		
		//KeyedArchive *mia = new KeyedArchive();
		//archive->SetArchive("rb.matinst", mia);
		//mia->Release();
	}
}

void RenderBatch::Load(KeyedArchive * archive, SerializationContext *serializationContext)
{
	if(NULL != archive)
	{
		type = archive->GetUInt32("rb.type", type);
		indexCount = archive->GetUInt32("rb.indexCount", indexCount);
		startIndex = archive->GetUInt32("rb.startIndex", startIndex);
		aabbox = archive->GetVariant("rb.aabbox")->AsAABBox3();
        sortingKey = archive->GetUInt32("rb.sortingKey", SORTING_KEY_DEF_VALUE);
		PolygonGroup *pg = static_cast<PolygonGroup*>(serializationContext->GetDataBlock(archive->GetVariant("rb.datasource")->AsUInt64()));
		
		NMaterial * newMaterial = NULL;
		bool shouldConvertMaterial = archive->IsKeyExists("rb.material");
		if(shouldConvertMaterial)
		{
			uint64 materialId = archive->GetVariant("rb.material")->AsUInt64();
			Material *mat = static_cast<Material*>(serializationContext->GetDataBlock(materialId));
			
			InstanceMaterialState * oldMaterialInstance = new InstanceMaterialState();
			KeyedArchive *mia = archive->GetArchive("rb.matinst");
			if(NULL != mia)
			{
				oldMaterialInstance->Load(mia, serializationContext);
			}
			
			if(mat)
			{
				newMaterial = serializationContext->ConvertOldMaterialToNewMaterial(mat, oldMaterialInstance, materialId);
			}
			
			SafeRelease(oldMaterialInstance);
		}
		else
		{
			int64 matKey = archive->GetUInt64("rb.nmatname");
			
			newMaterial = static_cast<NMaterial*>(serializationContext->GetDataBlock(matKey));

			SafeRetain(newMaterial); //VI: material refCount should be >1 at this point
		}

        if (pg!=dataSource)
        {
            SafeRelease(dataSource);
            dataSource = SafeRetain(pg);
        }

		if(newMaterial)
		{
			SetMaterial(newMaterial);
			DVASSERT(material->GetRetainCount() > 1);

			SafeRelease(newMaterial);
		}

        
	}

	BaseObject::LoadObject(archive);
}

void RenderBatch::UpdateAABBoxFromSource()
{
	if(NULL != dataSource)
	{
		aabbox = dataSource->GetBoundingBox();
		DVASSERT(aabbox.min.x != AABBOX_INFINITY &&
			aabbox.min.y != AABBOX_INFINITY &&
			aabbox.min.z != AABBOX_INFINITY);
	}
}

};
