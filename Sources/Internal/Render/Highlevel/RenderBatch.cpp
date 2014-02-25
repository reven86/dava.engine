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
    materialInstance = new InstanceMaterialState();
    ownerLayerName = INHERIT_FROM_MATERIAL;
	visiblityCriteria = RenderObject::VISIBILITY_CRITERIA;
	aabbox = AABBox3(Vector3(), Vector3());
}
    
RenderBatch::~RenderBatch()
{
	SafeRelease(dataSource);
	SafeRelease(renderDataObject);
	SafeRelease(material);
    SafeRelease(materialInstance);
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

void RenderBatch::SetMaterial(Material * _material)
{
	SafeRelease(material);
    material = SafeRetain(_material);
}

    
void RenderBatch::Draw(Camera * camera)
{
	if(!renderObject)return;
    Matrix4 * worldTransformPtr = renderObject->GetWorldTransformPtr();
    if (!worldTransformPtr)
    {
        return;
    }
    
//    uint32 flags = renderObject->GetFlags();
//    if ((flags & visiblityCriteria) != visiblityCriteria)
//        return;
    
    if(!GetVisible())
        return;
	
    Matrix4 finalMatrix = (*worldTransformPtr) * camera->GetMatrix();
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);
    material->Draw(dataSource,  materialInstance, worldTransformPtr);
}
    
    
const FastName & RenderBatch::GetOwnerLayerName()
{
    if (ownerLayerName == INHERIT_FROM_MATERIAL)
    {
        DVASSERT(material != 0);
        return material->GetOwnerLayerName();
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
    if (ownerLayer)ownerLayer->ForceLayerSort();
}


void RenderBatch::GetDataNodes(Set<DataNode*> & dataNodes)
{
	if(material)
	{
		InsertDataNode(material, dataNodes);
	}

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
	rb->material = SafeRetain(material);
	SafeRelease(rb->materialInstance);
    rb->materialInstance = materialInstance->Clone();

	rb->startIndex = startIndex;
	rb->indexCount = indexCount;
	rb->type = type;

	rb->aabbox = aabbox;

	rb->ownerLayerName = ownerLayerName;
	rb->sortingKey = sortingKey;

	return rb;
}

void RenderBatch::Save(KeyedArchive * archive, SceneFileV2* sceneFile)
{
	BaseObject::Save(archive);

	if(NULL != archive)
	{
		archive->SetUInt32("rb.type", type);
		archive->SetUInt32("rb.indexCount", indexCount);
		archive->SetUInt32("rb.startIndex", startIndex);
		archive->SetVariant("rb.aabbox", VariantType(aabbox));
		archive->SetVariant("rb.datasource", VariantType((uint64)dataSource));
		archive->SetVariant("rb.material", VariantType((uint64)GetMaterial()));
		
		if(material &&
		   materialInstance &&
		   Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP != material->type &&
		   Material::MATERIAL_VERTEX_LIT_LIGHTMAP != material->type &&
		   materialInstance->GetLightmap() != NULL)
		{
			materialInstance->ClearLightmap();
			Logger::Warning("[RenderBatch::Save] Material %s is not a lightmap but has lightmap texture set to %s. Clearing lightmap to avoid future problems.",
							material->GetName().c_str(),
							materialInstance->GetLightmapName().GetAbsolutePathname().c_str());
		}
		
		KeyedArchive *mia = new KeyedArchive();
		materialInstance->Save(mia, sceneFile);
		archive->SetArchive("rb.matinst", mia);
		mia->Release();
	}
}

void RenderBatch::Load(KeyedArchive * archive, SceneFileV2 *sceneFile)
{
	if(NULL != archive)
	{
		type = archive->GetUInt32("rb.type", type);
		indexCount = archive->GetUInt32("rb.indexCount", indexCount);
		startIndex = archive->GetUInt32("rb.startIndex", startIndex);
		aabbox = archive->GetVariant("rb.aabbox")->AsAABBox3();

		PolygonGroup *pg = dynamic_cast<PolygonGroup*>(sceneFile->GetNodeByPointer(archive->GetVariant("rb.datasource")->AsUInt64()));
		Material *mat = dynamic_cast<Material*>(sceneFile->GetNodeByPointer(archive->GetVariant("rb.material")->AsUInt64()));

		SetPolygonGroup(pg);
		SetMaterial(mat);

		KeyedArchive *mia = archive->GetArchive("rb.matinst");
		if(NULL != mia)
		{
			if(material &&
			   Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP != material->type &&
			   Material::MATERIAL_VERTEX_LIT_LIGHTMAP != material->type &&
			   mia->GetString("ims.lightmapname").size() > 0)
			{
				Logger::Warning("[RenderBatch::Load] Material %s is not a lightmap but has lightmap texture set to %s. Clearing lightmap to avoid future problems.",
								material->GetName().c_str(),
								mia->GetString("ims.lightmapname").c_str());
				
				mia->SetString("ims.lightmapname", "");
			}
			
			materialInstance->Load(mia, sceneFile);
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
    


};
