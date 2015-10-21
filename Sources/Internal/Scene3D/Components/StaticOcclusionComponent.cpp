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


#include "Scene3D/Components/StaticOcclusionComponent.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{



StaticOcclusionComponent::StaticOcclusionComponent()
{
    xSubdivisions = 2;
    ySubdivisions = 2;
    zSubdivisions = 2;
    boundingBox = AABBox3(Vector3(0.0f, 0.0f, 0.0f), Vector3(20.0f, 20.0f, 20.0f));
    placeOnLandscape = false;
}

Component * StaticOcclusionComponent::Clone(Entity * toEntity)
{
	StaticOcclusionComponent * newComponent = new StaticOcclusionComponent();
	newComponent->SetEntity(toEntity);
    newComponent->SetSubdivisionsX(xSubdivisions);
    newComponent->SetSubdivisionsY(ySubdivisions);
    newComponent->SetSubdivisionsZ(zSubdivisions);
    newComponent->SetBoundingBox(boundingBox);
    newComponent->SetPlaceOnLandscape(placeOnLandscape);
    newComponent->cellHeightOffset = cellHeightOffset;
	return newComponent;
}

void StaticOcclusionComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(NULL != archive)
	{
        archive->SetVariant("soc.aabbox", VariantType(boundingBox));
        archive->SetUInt32("soc.xsub", xSubdivisions);
        archive->SetUInt32("soc.ysub", ySubdivisions);
        archive->SetUInt32("soc.zsub", zSubdivisions);
        archive->SetBool("soc.placeOnLandscape", placeOnLandscape);
        if (placeOnLandscape)
            archive->SetByteArray("soc.cellHeightOffset", (uint8*)(&cellHeightOffset.front()), xSubdivisions*ySubdivisions*sizeof(float32));
	}
}

void StaticOcclusionComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	if(NULL != archive)
	{
        boundingBox = archive->GetVariant("soc.aabbox")->AsAABBox3();
        xSubdivisions = archive->GetUInt32("soc.xsub", 1);
        ySubdivisions = archive->GetUInt32("soc.ysub", 1);
        zSubdivisions = archive->GetUInt32("soc.zsub", 1);
        placeOnLandscape = archive->GetBool("soc.placeOnLandscape", false);
        if (placeOnLandscape)
        {
            cellHeightOffset.resize(xSubdivisions*ySubdivisions, 0);
            DVASSERT(xSubdivisions*ySubdivisions*sizeof(float32) == static_cast<uint32>(archive->GetByteArraySize("soc.cellHeightOffset")));
            memcpy(&cellHeightOffset.front(), archive->GetByteArray("soc.cellHeightOffset"), xSubdivisions*ySubdivisions*sizeof(float32));            
        }
    }

	Component::Deserialize(archive, serializationContext);
}
    
StaticOcclusionDataComponent::StaticOcclusionDataComponent()
{
}

StaticOcclusionDataComponent::~StaticOcclusionDataComponent()
{
}

Component * StaticOcclusionDataComponent::Clone(Entity * toEntity)
{
    StaticOcclusionDataComponent * newComponent = new StaticOcclusionDataComponent();
	newComponent->SetEntity(toEntity);
    newComponent->data = data;
    return newComponent;
}
    
void StaticOcclusionDataComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    Component::Serialize(archive, serializationContext);
    
	if(NULL != archive)
	{
        // VB:
        archive->SetVariant("sodc.bbox", VariantType(data.bbox));
        archive->SetUInt32("sodc.blockCount", data.blockCount);
        archive->SetUInt32("sodc.objectCount", data.objectCount);
        archive->SetUInt32("sodc.subX", data.sizeX);
        archive->SetUInt32("sodc.subY", data.sizeY);
        archive->SetUInt32("sodc.subZ", data.sizeZ);
        archive->SetByteArray("sodc.data", (uint8*)data.GetData(), data.blockCount * data.objectCount / 32 * 4);
        if (data.cellHeightOffset)
            archive->SetByteArray("sodc.cellHeightOffset", (uint8*)data.cellHeightOffset, data.sizeX*data.sizeY*sizeof(float32));
    }
}
    
void StaticOcclusionDataComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    if(NULL != archive)
	{
        data.bbox = archive->GetVariant("sodc.bbox")->AsAABBox3();
        data.blockCount = archive->GetUInt32("sodc.blockCount", 0);
        data.objectCount = archive->GetUInt32("sodc.objectCount", 0);
        data.sizeX = archive->GetUInt32("sodc.subX", 1);
        data.sizeY = archive->GetUInt32("sodc.subY", 1);
        data.sizeZ = archive->GetUInt32("sodc.subZ", 1);

        auto numElements = data.blockCount * data.objectCount / 32;
        auto dataSize = sizeof(uint32) * numElements;
        auto ptr = new uint32[numElements];
        DVASSERT(dataSize == archive->GetByteArraySize("sodc.data"));
        data.SetData(reinterpret_cast<const uint32*>(archive->GetByteArray("sodc.data")), dataSize);

        if (archive->IsKeyExists("sodc.cellHeightOffset"))
        {
            data.cellHeightOffset = new float32[data.sizeX*data.sizeY];
            DVASSERT(data.sizeX*data.sizeY*sizeof(float32) == static_cast<uint32>(archive->GetByteArraySize("sodc.cellHeightOffset")));
            memcpy(data.cellHeightOffset, archive->GetByteArray("sodc.cellHeightOffset"), data.sizeX*data.sizeY*sizeof(float32));
        }
    }
    
	Component::Deserialize(archive, serializationContext);
}

StaticOcclusionDebugDrawComponent::StaticOcclusionDebugDrawComponent(RenderObject *object)
{
    renderObject = SafeRetain(object);
}

StaticOcclusionDebugDrawComponent::~StaticOcclusionDebugDrawComponent()
{
    SafeRelease(renderObject);
    
    rhi::DeleteVertexBuffer(vertices);
    rhi::DeleteIndexBuffer(gridIndices);
    rhi::DeleteIndexBuffer(coverIndices);
}    
    
RenderObject * StaticOcclusionDebugDrawComponent::GetRenderObject() const
{
    return renderObject;
}
    
Component * StaticOcclusionDebugDrawComponent::Clone(Entity * toEntity)
{
    RenderObject *clonedRO = NULL;
    if(NULL != renderObject)
    {        
        clonedRO = renderObject->Clone(clonedRO);
    }
    StaticOcclusionDebugDrawComponent * component = new StaticOcclusionDebugDrawComponent(clonedRO);
	component->SetEntity(toEntity);    

    return component;
}
	

void StaticOcclusionDebugDrawComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	DVASSERT(false&&"Should not Serialize debug components. Check entity::save");
}

void StaticOcclusionDebugDrawComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	DVASSERT(false&&"Should not Deserialize debug components. Check entity::save");
}


}