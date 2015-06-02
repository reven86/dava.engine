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


#include "Scene3D/Systems/RenderUpdateSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Landscape.h"

#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Scene3D/Scene.h"
#include "Platform/SystemTimer.h"
#include "Debug/Stats.h"


namespace DAVA
{
RenderUpdateSystem::RenderUpdateSystem(Scene * scene)
:	SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
}

RenderUpdateSystem::~RenderUpdateSystem()
{
}

void RenderUpdateSystem::ImmediateEvent(Component * component, uint32 event)
{
    if (event == EventSystem::WORLD_TRANSFORM_CHANGED)
    {
        Entity * entity = component->GetEntity();
        // Update new transform pointer, and mark that transform is changed
        Matrix4 * worldTransformPointer = ((TransformComponent*)entity->GetComponent(Component::TRANSFORM_COMPONENT))->GetWorldTransformPtr();
		RenderObject * object = ((RenderComponent*)entity->GetComponent(Component::RENDER_COMPONENT))->GetRenderObject();
        if(NULL != object)
        {
            object->SetWorldTransformPtr(worldTransformPointer);
            entity->GetScene()->renderSystem->MarkForUpdate(object);
        }
    }
    
    //if (event == EventSystem::ACTIVE_CAMERA_CHANGED)
    {
        // entity->GetCameraComponent();
        // RenderSystem::
    }
}
    
void RenderUpdateSystem::AddEntity(Entity * entity)
{
    RenderObject * renderObject = ((RenderComponent*)entity->GetComponent(Component::RENDER_COMPONENT))->GetRenderObject();
    if (!renderObject)return;
	Matrix4 * worldTransformPointer = ((TransformComponent*)entity->GetComponent(Component::TRANSFORM_COMPONENT))->GetWorldTransformPtr();
    renderObject->SetWorldTransformPtr(worldTransformPointer);
    UpdateActiveIndexes(entity, renderObject);
    entityObjectMap.insert(entity, renderObject);
	GetScene()->GetRenderSystem()->RenderPermanent(renderObject);
}

void RenderUpdateSystem::RemoveEntity(Entity * entity)
{
    RenderObject * renderObject = entityObjectMap.at(entity);
    if (!renderObject)
	{
		return;
	}
    
    GetScene()->GetRenderSystem()->RemoveFromRender(renderObject);

	entityObjectMap.erase(entity);
}
    
void RenderUpdateSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("RenderUpdateSystem::Process");

    RenderSystem * renderSystem = GetScene()->GetRenderSystem();
    renderSystem->SetMainCamera(GetScene()->GetCurrentCamera());
    renderSystem->SetDrawCamera(GetScene()->GetDrawCamera());

    GetScene()->GetRenderSystem()->Update(timeElapsed);
}

void RenderUpdateSystem::UpdateActiveIndexes(Entity *entity, RenderObject *object)
{
    Entity *parent;
    
    // search effective lod index
    parent = entity;
    while(NULL != parent)
    {
        LodComponent *lc = GetLodComponent(parent);
        if(NULL != lc)
        {
            object->SetLodIndex(lc->currentLod);
            break;
        }

        parent = parent->GetParent();
    }

    // search effective switch index
    parent = entity;
    while(NULL != parent)
    {
        SwitchComponent *sc = GetSwitchComponent(parent);
        if(NULL != sc)
        {
            object->SetSwitchIndex(sc->GetSwitchIndex());
            break;
        }

        parent = parent->GetParent();
    }
}
    
};