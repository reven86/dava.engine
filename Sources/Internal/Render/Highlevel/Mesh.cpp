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


#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/Highlevel/ShadowVolume.h"
#include "Render/Material/NMaterial.h"

namespace DAVA
{


Mesh::Mesh()
{
    type = TYPE_MESH;
}
    
Mesh::~Mesh()
{
    
}

void Mesh::AddPolygonGroup(PolygonGroup * polygonGroup, NMaterial * material)
{
    RenderBatch * batch = new RenderBatch();
    batch->SetPolygonGroup(polygonGroup);
    batch->SetMaterial(material);
    batch->SetStartIndex(0);
    batch->SetIndexCount(polygonGroup->GetIndexCount());
    AddRenderBatch(batch);
    
	batch->Release();
    //polygonGroups.push_back(polygonGroup);
}
    
uint32 Mesh::GetPolygonGroupCount()
{
    return (uint32)renderBatchArray.size();
}

PolygonGroup * Mesh::GetPolygonGroup(uint32 index)
{
    return renderBatchArray[index].renderBatch->GetPolygonGroup();
}

RenderObject * Mesh::Clone( RenderObject *newObject )
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<Mesh>(this), "Can clone only Mesh");
		newObject = new Mesh();
	}

	return RenderObject::Clone(newObject);
}

void Mesh::Save(KeyedArchive *archive, SerializationContext *serializationContext)
{
	RenderObject::Save(archive, serializationContext);
}

void Mesh::Load(KeyedArchive *archive, SerializationContext *serializationContext)
{
	RenderObject::Load(archive, serializationContext);
}

void Mesh::BakeGeometry(const Matrix4 & transform)
{
	uint32 size = static_cast<uint32>(renderBatchArray.size());
	for(uint32 i = 0; i < size; ++i)
	{
		PolygonGroup * pg = renderBatchArray[i].renderBatch->GetPolygonGroup();
		DVASSERT(pg);
		pg->ApplyMatrix(transform);
		pg->BuildBuffers();

		renderBatchArray[i].renderBatch->UpdateAABBoxFromSource();
	}

	RecalcBoundingBox();
}

};
