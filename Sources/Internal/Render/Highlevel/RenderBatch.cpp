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
#include "Scene3D/SceneFileV2.h"
#include "Debug/DVAssert.h"
#include "Render/Material/MaterialSystem.h"
#include "Render/OcclusionQuery.h"
#include "Debug/Stats.h"

namespace DAVA
{

    
RenderBatch::RenderBatch()
    :   ownerLayer(0)
    ,   removeIndex(-1)
    ,   sortingKey(8)
{
    dataSource = 0;
    renderDataObject = 0;
    material = 0;
    startIndex = 0;
    indexCount = 0;
    type = PRIMITIVETYPE_TRIANGLELIST;
	renderObject = 0;
    ownerLayerName = INHERIT_FROM_MATERIAL;
	visiblityCriteria = RenderObject::VISIBILITY_CRITERIA;
	aabbox = AABBox3(Vector3(), Vector3());
#if defined(__DAVA_USE_OCCLUSION_QUERY__)
    occlusionQuery = new OcclusionQuery();
#endif
    queryRequested = -1;
    lastFraemDrawn = -10;
}
    
RenderBatch::~RenderBatch()
{
#if defined(__DAVA_USE_OCCLUSION_QUERY__)
    SafeDelete(occlusionQuery);
#endif
	SafeRelease(dataSource);
	SafeRelease(renderDataObject);
	
	if(material)
	{
		material->SetParent(NULL);
	}
	
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
	SafeRelease(material);
    material = SafeRetain(_material);
	
	//VI: material should be ready after it has been set to render batch
	//VI: so render system will be able to determine different materials-related renderbatch properties
	//VI: such as if renderbatch receives dynamic light etc
	if(material && !material->IsReady())
	{
		material->Rebuild();
	}	
}
    
void RenderBatch::Draw(const FastName & ownerRenderPass, Camera * camera)
{
//  TIME_PROFILE("RenderBatch::Draw");
//	if(!renderObject)return;
    DVASSERT(renderObject != 0);
    Matrix4 * worldTransformPtr = renderObject->GetWorldTransformPtr();
    DVASSERT(worldTransformPtr != 0);
    
#if defined(DYNAMIC_OCCLUSION_CULLING_ENABLED)
    uint32 globalFrameIndex = Core::Instance()->GetGlobalFrameIndex();
    
    if (RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::DYNAMIC_OCCLUSION_ENABLE))
    {
        if ((queryRequested >= 0) && occlusionQuery->IsResultAvailable())
        {
            uint32 result = 0;
            occlusionQuery->GetQuery(&result);
            if (result == 0 && ((globalFrameIndex - queryRequestFrame) < 3) && (lastFraemDrawn == globalFrameIndex - 1))
            {
                //RenderManager::Instance()->GetStats().occludedRenderBatchCount++;
                occlusionQuery->ResetResult();
                queryRequested = -2;
            }
            else queryRequested = -1;
        }
    }
    
    if (queryRequested < -1)
    {
        queryRequested++;
        RenderManager::Instance()->GetStats().occludedRenderBatchCount++;
        return;
    }
#endif
//    if (!worldTransformPtr)
//    {
//        return;
//    }
//    uint32 flags = renderObject->GetFlags();
//    if ((flags & visiblityCriteria) != visiblityCriteria)
//        return;
//    if(!GetVisible())
//        return;
	
    Matrix4 finalMatrix = (*worldTransformPtr) * camera->GetMatrix();
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix, (pointer_size)worldTransformPtr);

    material->BindMaterialTechnique(ownerRenderPass, camera);

#if defined(DYNAMIC_OCCLUSION_CULLING_ENABLED)
    if (RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::DYNAMIC_OCCLUSION_ENABLE))
    {
        if (queryRequested == -1)
        {
            queryRequestFrame = globalFrameIndex;
            occlusionQuery->BeginQuery();
            queryRequested = 0;
        }
        else queryRequested++;
    }
#endif
    material->Draw(dataSource);
    
#if defined(DYNAMIC_OCCLUSION_CULLING_ENABLED)
    lastFraemDrawn = globalFrameIndex;
    
    if (RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::DYNAMIC_OCCLUSION_ENABLE))
    {
        if (queryRequested == 0)
        {
            occlusionQuery->EndQuery();
        }
    }
#endif
}
    
    
FastName RenderBatch::GetOwnerLayerName()
{
    if (ownerLayerName == INHERIT_FROM_MATERIAL)
    {
        DVASSERT(material != 0);
        return ownerLayerName;
    }
    else
    {
        return ownerLayerName;
    }
}
    
void RenderBatch::SetOwnerLayerName(const FastName & fastname)
{
    ownerLayerName = fastname;
}
    
//const FastName & RenderBatch::GetOwnerLayerName()
//{
//    static FastName opaqueLayer("OpaqueRenderLayer");
//    static FastName translucentLayer("TransclucentRenderLayer");
//    
//    if (material)
//    {
//        if(material->GetOpaque() || material->GetAlphablend())
//		{
//			return translucentLayer;
//		}
//    }
//    
//    return opaqueLayer;
//}

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
    sortingKey = _key;
}


void RenderBatch::GetDataNodes(Set<DataNode*> & dataNodes)
{
	//VI: NMaterial is not a DataNode anymore
	/*if(material)
	{
		InsertDataNode(material, dataNodes);
	}*/

	if(dataSource)
	{
		InsertDataNode(dataSource, dataNodes);
	}
}

void RenderBatch::InsertDataNode(DataNode *node, Set<DataNode*> & dataNodes)
{
	dataNodes.insert(node);

	for(int32 i = 0; i < node->GetChildrenCount(); ++i)
	{
		InsertDataNode(node->GetChild(i), dataNodes);
	}
}

RenderBatch * RenderBatch::Clone(RenderBatch * destination)
{
    RenderBatch * rb = destination;
    if (!rb)
        rb = new RenderBatch();

	rb->dataSource = SafeRetain(dataSource);
	rb->renderDataObject = SafeRetain(renderDataObject);
	
	if(material)
	{
		rb->material = material->Clone();
		//rb->material->SetMaterialSystem(NULL);
	}
	else
	{
		SafeRelease(rb->material);
	}

	rb->startIndex = startIndex;
	rb->indexCount = indexCount;
	rb->type = type;

	rb->aabbox = aabbox;

	rb->ownerLayerName = ownerLayerName;
	rb->sortingKey = sortingKey;

	return rb;
}

void RenderBatch::Save(KeyedArchive * archive, SerializationContext* serializationContext)
{
	BaseObject::Save(archive);

	if(NULL != archive)
	{
		archive->SetUInt32("rb.type", type);
		archive->SetUInt32("rb.indexCount", indexCount);
		archive->SetUInt32("rb.startIndex", startIndex);
		archive->SetVariant("rb.aabbox", VariantType(aabbox));
		archive->SetVariant("rb.datasource", VariantType((uint64)dataSource));
		
		NMaterial* material = GetMaterial();
		if(material)
		{
			archive->SetString("rb.nmatname", material->GetMaterialName().c_str());
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
		PolygonGroup *pg = static_cast<PolygonGroup*>(serializationContext->GetDataBlock(archive->GetVariant("rb.datasource")->AsUInt64()));
		
		NMaterial * newMaterial = NULL;
		bool shouldConvertMaterial = archive->IsKeyExists("rb.material");
		if(shouldConvertMaterial)
		{
			Material *mat = static_cast<Material*>(serializationContext->GetDataBlock(archive->GetVariant("rb.material")->AsUInt64()));
			
			InstanceMaterialState * oldMaterialInstance = new InstanceMaterialState();
			KeyedArchive *mia = archive->GetArchive("rb.matinst");
			if(NULL != mia)
			{
				oldMaterialInstance->Load(mia, serializationContext);
			}
			
			if(mat)
			{
				newMaterial = serializationContext->ConvertOldMaterialToNewMaterial(mat, oldMaterialInstance);
				MaterialSystem* matSystem = serializationContext->GetMaterialSystem();
				
				matSystem->BindMaterial(newMaterial);
			}
			
			SafeRelease(oldMaterialInstance);
		}
		else
		{
			String matName = archive->GetString("rb.nmatname");
			
			newMaterial = serializationContext->GetNewMaterial(matName);
		}

		SetPolygonGroup(pg);
        
		if(newMaterial)
		{
			SetMaterial(newMaterial);
		}
	}

	BaseObject::Load(archive);
}

void RenderBatch::SetVisibilityCriteria(uint32 criteria)
{
	visiblityCriteria = criteria;
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
    
bool RenderBatch::GetVisible() const
{
    uint32 flags = renderObject->GetFlags();
    return ((flags & visiblityCriteria) == visiblityCriteria);
}
    
void RenderBatch::AttachToRenderSystem(RenderSystem* rs)
{
	MaterialSystem* matSystem = (rs) ? rs->GetMaterialSystem() : NULL;
	MaterialSystem* prevSystem = (material) ? material->GetMaterialSystem() : NULL;
	if(material &&
	   prevSystem != matSystem)
	{
		if(NULL == matSystem)
		{
			if(prevSystem)
			{
				prevSystem->RemoveMaterial(material);
			}
			
			material->SetMaterialSystem(NULL);
		}
		else
		{
			matSystem->BindMaterial(material);
			
			if(prevSystem)
			{
				prevSystem->RemoveMaterial(material);
			}
		}
	}
}
};
