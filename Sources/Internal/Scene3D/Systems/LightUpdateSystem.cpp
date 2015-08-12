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


#include "Scene3D/Systems/LightUpdateSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Landscape.h"

#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Scene3D/Scene.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
LightUpdateSystem::LightUpdateSystem(Scene * scene)
:	SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
}

LightUpdateSystem::~LightUpdateSystem()
{
}

void LightUpdateSystem::ImmediateEvent(Component * component, uint32 event)
{
    if (event == EventSystem::WORLD_TRANSFORM_CHANGED)
    {
        Entity * entity = component->GetEntity();
        RecalcLight(entity);
    }
    
    //if (event == EventSystem::ACTIVE_CAMERA_CHANGED)
    {
        // entity->GetCameraComponent();
        // RenderSystem::
    }
}

void LightUpdateSystem::RecalcLight(Entity *entity)
{
    // Update new transform pointer, and mark that transform is changed
    Matrix4 * worldTransformPointer = ((TransformComponent*)entity->GetComponent(Component::TRANSFORM_COMPONENT))->GetWorldTransformPtr();
    Light * light = ((LightComponent*)entity->GetComponent(Component::LIGHT_COMPONENT))->GetLightObject();
    light->SetPositionDirectionFromMatrix(*worldTransformPointer);
    entity->GetScene()->renderSystem->MarkForUpdate(light);
}
    
void LightUpdateSystem::AddEntity(Entity * entity)
{
    Light * lightObject = ((LightComponent*)entity->GetComponent(Component::LIGHT_COMPONENT))->GetLightObject();
    if (!lightObject)return;

    entityObjectMap.insert(entity, lightObject);
    GetScene()->GetRenderSystem()->AddLight(lightObject);

    RecalcLight(entity);
}

void LightUpdateSystem::RemoveEntity(Entity * entity)
{
    Light * lightObject = entityObjectMap.at(entity);
    if (!lightObject)
	{
		return;
	}
    
    GetScene()->GetRenderSystem()->RemoveLight(lightObject);
	entityObjectMap.erase(entity);
}
       
};