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


#include "Scene3D/Components/RenderComponent.h"
#include "Base/ObjectFactory.h"

namespace DAVA 
{
    
    
RenderComponent::RenderComponent(RenderObject * _object)
{
    renderObject = SafeRetain(_object);
}

RenderComponent::~RenderComponent()
{
    SafeRelease(renderObject);
}
    
void RenderComponent::SetRenderObject(RenderObject * _renderObject)
{
	SafeRelease(renderObject);
    renderObject = SafeRetain(_renderObject);
}
    
RenderObject * RenderComponent::GetRenderObject()
{
    return renderObject;
}
    
Component * RenderComponent::Clone(Entity * toEntity)
{
    RenderComponent * component = new RenderComponent();
	component->SetEntity(toEntity);

    if(NULL != renderObject)
    {
        //TODO: Do not forget ot check what does it means.
        component->renderObject = renderObject->Clone(component->renderObject);
    }

    return component;
}
	
void RenderComponent::OptimizeBeforeExport()
{
/*
    uint32 count = renderObject->GetRenderBatchCount();
    for(uint32 i = 0; i < count; ++i)
    {
        RenderBatch *renderBatch = renderObject->GetRenderBatch(i);
		if(NULL != renderBatch)
		{
			PolygonGroup* polygonGroup = renderBatch->GetPolygonGroup();
			NMaterial* material = renderBatch->GetMaterial();
			if(NULL != polygonGroup && NULL != material)
			{
				//uint32 newFormat = MaterialOptimizer::GetOptimizedVertexFormat((Material::eType)renderBatch->GetMaterial()->type);
				// polygonGroup->OptimizeVertices(newFormat); //TODO::VK crash on Tanks/USSR/T-28_crash.sc2
			}
		}
	}
*/
}

void RenderComponent::GetDataNodes(Set<DAVA::DataNode *> &dataNodes)
{
    if(NULL != renderObject)
    {
        renderObject->GetDataNodes(dataNodes);
    }
}

void RenderComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(NULL != archive && NULL != renderObject)
	{
		KeyedArchive *roArch = new KeyedArchive();
		renderObject->Save(roArch, serializationContext);
		archive->SetArchive("rc.renderObj", roArch);
		roArch->Release();
	}
}

void RenderComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	if(NULL != archive)
	{
		KeyedArchive *roArch = archive->GetArchive("rc.renderObj");
		if(NULL != roArch)
		{
			RenderObject* ro = (RenderObject *) ObjectFactory::Instance()->New<RenderObject>(roArch->GetString("##name"));
			if(NULL != ro)
			{
				ro->Load(roArch, serializationContext);
				SetRenderObject(ro);

				ro->Release();
			}
		}
	}

	Component::Deserialize(archive, serializationContext);
}

};
