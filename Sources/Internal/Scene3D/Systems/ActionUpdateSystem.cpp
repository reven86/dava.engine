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


#include "Scene3D/Entity.h"
#include "Platform/SystemTimer.h"
#include "Scene3D/Systems/ActionUpdateSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Debug/Stats.h"

namespace DAVA
{
	
ActionUpdateSystem::ActionUpdateSystem(Scene * scene)
:	SceneSystem(scene)
{
	UnblockAllEvents();
}

void ActionUpdateSystem::SetBlockEvent(ActionComponent::Action::eEvent eventType, bool block)
{
	eventBlocked[eventType] = block;
}

bool ActionUpdateSystem::IsBlockEvent(ActionComponent::Action::eEvent eventType)
{
	return eventBlocked[eventType];
}

void ActionUpdateSystem::UnblockAllEvents()
{
	for (int i=0; i<ActionComponent::Action::EVENTS_COUNT; i++)
		eventBlocked[i] = false;
}

void ActionUpdateSystem::AddEntity(Entity * entity)
{
	SceneSystem::AddEntity(entity);
	ActionComponent* actionComponent = static_cast<ActionComponent*>(entity->GetComponent(Component::ACTION_COMPONENT));
	actionComponent->StartAdd();
}

void ActionUpdateSystem::RemoveEntity(Entity * entity)
{	
	ActionComponent* actionComponent = static_cast<ActionComponent*>(entity->GetComponent(Component::ACTION_COMPONENT));	
	if (actionComponent->IsStarted())
		UnWatch(actionComponent);	
	SceneSystem::RemoveEntity(entity);
}
		
void ActionUpdateSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("ActionUpdateSystem::Process");

	DelayedDeleteActions();
	
	uint32 size = static_cast<uint32>(activeActions.size());
	for(uint32 index = 0; index < size; ++index)
	{
		ActionComponent* component = activeActions[index];
		component->Update(timeElapsed);
	}
}

void ActionUpdateSystem::DelayedDeleteActions()
{
	Vector<ActionComponent*>::iterator end = deleteActions.end();
	for(Vector<ActionComponent*>::iterator it = deleteActions.begin(); it != end; ++it)
	{
		Vector<ActionComponent*>::iterator i = std::find(activeActions.begin(), activeActions.end(), *it);

		if(i != activeActions.end())
		{
			activeActions.erase(i);
		}
	}

	deleteActions.clear();
}
	
void ActionUpdateSystem::Watch(ActionComponent* component)
{
	activeActions.push_back(component);
}

void ActionUpdateSystem::UnWatch(ActionComponent* component)
{
	deleteActions.push_back(component);
}
	
}