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


#include "Entity/Component.h"
#include "Scene3D/Systems/DebugRenderSystem.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/LightComponent.h"

#include "Render/Highlevel/Camera.h"
#include "Render/RenderHelper.h"
#include "Debug/Stats.h"
#include "Render/Renderer.h"

namespace DAVA
{

DebugRenderSystem::DebugRenderSystem(Scene * scene)
:	SceneSystem(scene),
	camera(0)
{
}
    
DebugRenderSystem::~DebugRenderSystem()
{
    
}

void DebugRenderSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("DebugRenderSystem::Process");

    SetCamera(GetScene()->GetCurrentCamera());

    /*    uint32 size = static_cast<uint32>(entities.size());
	for(uint32 i = 0; i < size; ++i)
	{
        Entity * entity = entities[i];
        
        DebugRenderComponent * debugRenderComponent = cast_if_equal<DebugRenderComponent*>(entity->GetComponent(Component::DEBUG_RENDER_COMPONENT));
        TransformComponent * transformComponent = cast_if_equal<TransformComponent*>(entity->GetComponent(Component::TRANSFORM_COMPONENT));
        //RenderComponent * renderComponent = cast_if_equal<RenderComponent*>(entity->GetComponent(Component::RENDER_COMPONENT));
        
        //Matrix4 worldTransform = camera->GetMatrix();
        Renderer::GetDynamicBindings().SetDynamicParam(PARAM_VIEW, &camera->GetMatrix(), DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

        AABBox3 debugBoundigBox = entity->GetWTMaximumBoundingBoxSlow();
        uint32 debugFlags = debugRenderComponent->GetDebugFlags();

		// Camera debug draw
		if(debugFlags & DebugRenderComponent::DEBUG_DRAW_CAMERA)
		{
			CameraComponent * entityCameraComp = (CameraComponent *) entity->GetComponent(Component::CAMERA_COMPONENT);

			if(NULL != entityCameraComp)
			{
				Camera* entityCamera = entityCameraComp->GetCamera();
				if(NULL != entityCamera && camera != entityCamera)
				{
					Color camColor(0.0f, 1.0f, 0.0f, 1.0f);
					Vector3 camPos = entityCamera->GetPosition();
					//Vector3 camDirect = entityCamera->GetDirection();
					AABBox3 camBox(camPos, 2.5f);

					// If this is clip camera - show it as red camera
					if (entityCamera == entity->GetScene()->GetDrawCamera()) camColor = Color(1.0f, 0.0f, 0.0f, 1.0f);


					RenderSystem2D::Instance()->SetColor(camColor);
					RenderHelper::Instance()->DrawBox(camBox, 2.5f, depthWriteState);

					//RenderManager::Instance()->SetState(RenderState::DEFAULT_3D_STATE);
					RenderSystem2D::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);

					debugBoundigBox = camBox;
				}
			}
		}

		// UserNode debug draw
		if(debugFlags & DebugRenderComponent::DEBUG_DRAW_USERNODE)
		{
			if(NULL != entity->GetComponent(Component::USER_COMPONENT))
			{
				Color dcColor(0.0f, 0.0f, 1.0f, 1.0f);
				AABBox3 dcBox(Vector3(), 1.0f);

				//Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
				//Matrix4 finalMatrix = transformComponent->GetWorldTransform() * prevMatrix;
				
                Renderer::GetDynamicBindings().SetDynamicParam(PARAM_WORLD, &transformComponent->GetWorldTransform(), DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
                Renderer::GetDynamicBindings().SetDynamicParam(PARAM_VIEW, &camera->GetMatrix(), DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

								
				RenderSystem2D::Instance()->SetColor(1.f, 1.f, 0, 1.0f);
				RenderHelper::Instance()->DrawLine(Vector3(0, 0, 0), Vector3(1.f, 0, 0), 1.0f, depthTestState);
				RenderSystem2D::Instance()->SetColor(1.f, 0, 1.f, 1.0f);
				RenderHelper::Instance()->DrawLine(Vector3(0, 0, 0), Vector3(0, 1.f, 0), 1.0f, depthTestState);
				RenderSystem2D::Instance()->SetColor(0, 1.f, 1.f, 1.0f);
				RenderHelper::Instance()->DrawLine(Vector3(0, 0, 0), Vector3(0, 0, 1.f), 1.0f, depthTestState);

				RenderSystem2D::Instance()->SetColor(dcColor);
				RenderHelper::Instance()->DrawBox(dcBox, 1.0f, depthTestState);

				//RenderManager::Instance()->SetState(RenderState::DEFAULT_3D_STATE);
				RenderSystem2D::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
				//RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);

				dcBox.GetTransformedBox(transformComponent->GetWorldTransform(), debugBoundigBox);
			}
		}

		// LightNode debug draw
		if (debugFlags & DebugRenderComponent::DEBUG_DRAW_LIGHT_NODE)
		{
			LightComponent *lightComp = (LightComponent *) entity->GetComponent(Component::LIGHT_COMPONENT);

			if(NULL != lightComp)
			{
				Light* light = lightComp->GetLightObject();

				if(NULL != light)
				{
					Vector3 lPosition = light->GetPosition();

					RenderSystem2D::Instance()->SetColor(1.0f, 1.0f, 0.0f, 1.0f);

					switch (light->GetType())
					{
					case Light::TYPE_DIRECTIONAL:
						{
							Vector3 lDirection = light->GetDirection();

							lDirection.Normalize();
							RenderHelper::Instance()->DrawArrow(lPosition, lPosition + lDirection * 10, 2.5f, 1.0f, depthWriteState);
							RenderHelper::Instance()->DrawBox(AABBox3(lPosition, 0.5f), 1.5f, depthWriteState);

							debugBoundigBox = AABBox3(lPosition, 2.5f);
						}
						break;
					default:
						{
							AABBox3 lightBox(lPosition, 2.5f);
							RenderHelper::Instance()->DrawBox(lightBox, 2.5f, depthWriteState);

							debugBoundigBox = lightBox;
						}
						break;
					}

					//RenderManager::Instance()->SetState(RenderState::DEFAULT_3D_STATE);
					RenderSystem2D::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
				}
			}
		}
        
        if ((debugFlags & DebugRenderComponent::DEBUG_DRAW_AABOX_CORNERS))
        {
            RenderSystem2D::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
            RenderHelper::Instance()->DrawCornerBox(debugBoundigBox, 1.0f, depthTestState);
        }
        
        if (debugFlags & DebugRenderComponent::DEBUG_DRAW_RED_AABBOX)
        {
            RenderSystem2D::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
            RenderHelper::Instance()->DrawBox(debugBoundigBox, 1.0f, depthWriteState);
            RenderSystem2D::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        }               
    }*/
}

void DebugRenderSystem::AddEntity(Entity * entity)
{
	entities.push_back(entity);

    //DebugRenderComponent * debugRenderComponent = static_cast<DebugRenderComponent*>(entity->GetComponent(Component::DEBUG_RENDER_COMPONENT));
    RenderComponent * renderComponent = static_cast<RenderComponent*>(entity->GetComponent(Component::RENDER_COMPONENT));
    if (renderComponent)
    {
        //renderComponent->renderObject->SetDebugFlags(debugRenderComponent->GetFlags());
    }
}

void DebugRenderSystem::RemoveEntity(Entity * entity)
{
    //DebugRenderComponent * debugRenderComponent = static_cast<DebugRenderComponent*>(entity->GetComponent(Component::DEBUG_RENDER_COMPONENT));
    RenderComponent * renderComponent = static_cast<RenderComponent*>(entity->GetComponent(Component::RENDER_COMPONENT));
    if (renderComponent)
    {
        //renderComponent->renderObject->SetDebugFlags(0);
    }

	
    uint32 size = static_cast<uint32>(entities.size());
	for(uint32 i = 0; i < size; ++i)
	{
		if(entities[i] == entity)
		{
			entities[i] = entities[size-1];
			entities.pop_back();
			return;
		}
	}
	
	DVASSERT(0);
}
    
void DebugRenderSystem::SetCamera(Camera * _camera)
{
    camera = _camera;
}

}
