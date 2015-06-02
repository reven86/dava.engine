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


#include "Scene3D/Systems/SwitchSystem.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Debug/Stats.h"
#include "Scene3D/Components/ActionComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{

SwitchSystem::SwitchSystem(Scene * scene)
:	SceneSystem(scene)
{
	scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SWITCH_CHANGED);
}

void SwitchSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("SwitchSystem::Process");
    
	Set<Entity*>::iterator it;
	Set<Entity*>::const_iterator itEnd = updatableEntities.end();
	for(it = updatableEntities.begin(); it != itEnd; ++it)
	{
		Entity * entity = *it;
		SwitchComponent * sw = (SwitchComponent*)entity->GetComponent(Component::SWITCH_COMPONENT);

        if (sw->oldSwitchIndex != sw->newSwitchIndex)
		{
            SetSwitchHierarchy(entity, sw->newSwitchIndex);

			sw->oldSwitchIndex = sw->newSwitchIndex;
			
			ActionComponent* actionComponent = cast_if_equal<ActionComponent*>(entity->GetComponent(Component::ACTION_COMPONENT));
			if(NULL != actionComponent)
			{
				actionComponent->StartSwitch(sw->newSwitchIndex);
			}
		}
	}

	updatableEntities.clear();
}

void SwitchSystem::ImmediateEvent(Component * component, uint32 event)
{
	if(EventSystem::SWITCH_CHANGED == event)
	{
		updatableEntities.insert(component->GetEntity());
	}
}

void SwitchSystem::AddEntity(Entity * entity)
{
    updatableEntities.insert(entity); //need update entity when add it into scene
}

void SwitchSystem::RemoveEntity(Entity* entity)
{
    updatableEntities.erase(entity);
}

void SwitchSystem::SetSwitchHierarchy(Entity * entity, int32 switchIndex)
{
    RenderObject * ro = GetRenderObject(entity);
	if(ro)
    {
        ro->SetSwitchIndex(switchIndex);
    }

	uint32 size = entity->GetChildrenCount();
	for(uint32 i = 0; i < size; ++i)
	{
		SetSwitchHierarchy(entity->GetChild(i), switchIndex);
	}
}

}
