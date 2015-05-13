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



#include "Render/Material.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Systems/SkyboxSystem.h"
#include "Scene3D/Entity.h"
#include "Render/Highlevel/SkyboxRenderObject.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"


namespace DAVA
{
	SkyboxSystem::SkyboxSystem(Scene * scene)
	:	SceneSystem(scene)
	{
		skyboxEntity = NULL;
	}
	
	SkyboxSystem::~SkyboxSystem()
	{
		SafeRelease(skyboxEntity);
	}
	
	void SkyboxSystem::AddEntity(Entity * entity)
	{
		if((NULL == skyboxEntity) && GetSkybox(entity))
		{
            skyboxEntity = SafeRetain(entity);
		}
	}
	
	void SkyboxSystem::RemoveEntity(Entity * entity)
	{
		if(entity == skyboxEntity)
		{
			SafeRelease(skyboxEntity);
		}
	}

	bool SkyboxSystem::IsSkyboxPresent()
	{
		return (skyboxEntity != NULL);
	}
	
	Entity* SkyboxSystem::AddSkybox()
	{
		Entity* result = skyboxEntity;
		
		if(NULL == skyboxEntity)
		{
			SkyboxRenderObject* skyboxRenderObject = new SkyboxRenderObject();
						
			RenderComponent* renderComponent = new RenderComponent();
			renderComponent->SetRenderObject(skyboxRenderObject);
			
			result = new Entity();
			result->SetName("Skybox");

			result->AddComponent(renderComponent);
			AABBox3 box = AABBox3(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
			skyboxRenderObject->Initialize(box); //first time initialization

			GetScene()->AddNode(result);			
			
			Matrix4 * worldTransformPointer = ((TransformComponent*)result->GetComponent(Component::TRANSFORM_COMPONENT))->GetWorldTransformPtr();
			skyboxRenderObject->SetWorldTransformPtr(worldTransformPointer);
			result->GetScene()->renderSystem->MarkForUpdate(skyboxRenderObject);
			SafeRelease(skyboxRenderObject);
			
			DVASSERT(skyboxEntity);
			result->Release();
		}
		
		return result;
	}
	
	Entity* SkyboxSystem::GetSkyboxEntity() const
	{
		return skyboxEntity;
	}
	
	void SkyboxSystem::Reload()
	{
#ifdef RHI_COMPLETE
		if(skyboxEntity)
		{
			RenderComponent* renderComponent = static_cast<RenderComponent*>(skyboxEntity->GetComponent(Component::RENDER_COMPONENT));
			SkyboxRenderObject* renderObj = cast_if_equal<SkyboxRenderObject*>(renderComponent->GetRenderObject());
			
			if(renderObj &&
			   renderObj->GetRenderBatchCount() > 0)
			{
				RenderBatch* renderBatch = renderObj->GetRenderBatch(0);
				NMaterial* material = renderBatch->GetMaterial();
				
				if(material)
				{
					Texture* texture = material->GetTexture(NMaterialTextureName::TEXTURE_CUBEMAP);
					if(texture)
					{
						texture->Reload();
					}
				}
			}
		}
#endif // RHI_COMPLETE
	}
};
