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



#include "Scene3D/Systems/TransformSystem.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Entity.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Debug/Stats.h"
#include "Job/JobManager.h"
#include "Platform/DeviceInfo.h"

namespace DAVA
{

TransformSystem::TransformSystem(Scene * scene)
:	SceneSystem(scene)
{
	scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::LOCAL_TRANSFORM_CHANGED);
	scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::TRANSFORM_PARENT_CHANGED);
}

TransformSystem::~TransformSystem()                                                                                                                                                   
{ }

void TransformSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("TransformSystem::Process");
    
    passedNodes = 0;
    multipliedNodes = 0;

	// calculate optimal jobs count for current number of entities should be processed
	uint32 jobsCount = Min(maxProcessingThreads, (uint32) updatableEntities.size() / DeviceInfo::GetCPUCoresCount());

	if(jobsCount > 0 && RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::IMPOSTERS_ENABLE))
    {
		const uint32 entitiesCount = updatableEntities.size();
		const uint32 entitiesPerJobCount = entitiesCount / jobsCount;

		uint32 firstIndex = 0;
		uint32 rest = entitiesCount - (jobsCount * entitiesPerJobCount);

		for(uint32 i = 0; i < jobsCount; ++i)
		{
			// entities = 90, jobsCount = 4, so "rest" will be (100 - 22 * 4) = 2
			// count per step will be: 23, 23, 22, 22
			uint32 count = entitiesPerJobCount;
			if(rest > 0)
			{
				count++;
				rest--;
			}

			// run jobs
			Function<void()> fn = Bind(MakeFunction(this, &TransformSystem::UpdateHierarchyPart), firstIndex, count, &sendEvent[i]);
			JobManager2::Instance()->CreateWorkerJob(fn);

			// move to next part of entities
			firstIndex += count;
		}

		JobManager2::Instance()->WaitWorkerJobs();

		for(uint32 i = 0; i < jobsCount; ++i)
		{
			GlobalEventSystem::Instance()->GroupEvent(GetScene(), sendEvent[i], EventSystem::WORLD_TRANSFORM_CHANGED);
			sendEvent[i].clear();
		}
    }
    else
    {
		// single thread processing
        uint32 size = updatableEntities.size();
        for(uint32 i = 0; i < size; ++i)
        {
            UpdateHierarchy(updatableEntities[i], &sendEvent[0]);
        }
        
        GlobalEventSystem::Instance()->GroupEvent(GetScene(), sendEvent[0], EventSystem::WORLD_TRANSFORM_CHANGED);
        sendEvent[0].clear();
    }
    
    updatableEntities.clear();
    
    if(passedNodes)
    {
        // Logger::Info("TransformSystem %d passed %d multiplied", passedNodes, multipliedNodes);
    }
}

void TransformSystem::UpdateHierarchyPart(uint32 from, uint32 count, Vector<Entity*> *updatedEntities)
{
	for(uint32 i = 0; i < count; ++i)
	{
		UpdateHierarchy(updatableEntities[from + i], updatedEntities);
	}
}

void TransformSystem::UpdateHierarchy(Entity *entity, Vector<Entity*> *updatedEntities, bool force)
{
	AtomicIncrement(passedNodes);

    if(force || entity->GetFlags() & Entity::TRANSFORM_NEED_UPDATE)
    {
		TransformComponent * transform = (TransformComponent*) entity->GetComponent(Component::TRANSFORM_COMPONENT);
		if(transform->parentMatrix)
		{
			transform->worldMatrix = transform->localMatrix * *(transform->parentMatrix);
            updatedEntities->push_back(entity);

            AtomicIncrement(multipliedNodes);
            force = true;
        }
    }

	uint32 size = entity->GetChildrenCount();
	for(uint32 i = 0; i < size; ++i)
	{
        UpdateHierarchy(entity->GetChild(i), updatedEntities, force);
    }

	entity->RemoveFlag(Entity::TRANSFORM_NEED_UPDATE);
	entity->RemoveFlag(Entity::TRANSFORM_DIRTY);
}

void TransformSystem::ImmediateEvent(Entity * entity, uint32 event)
{
	switch(event)
	{
	case EventSystem::LOCAL_TRANSFORM_CHANGED:
	case EventSystem::TRANSFORM_PARENT_CHANGED:
		EntityNeedUpdate(entity);
		HierahicAddToUpdate(entity);
		break;
	}
}

void TransformSystem::EntityNeedUpdate(Entity * entity)
{
	entity->AddFlag(Entity::TRANSFORM_NEED_UPDATE);
}

void TransformSystem::HierahicAddToUpdate(Entity * entity)
{
	if(!(entity->GetFlags() & Entity::TRANSFORM_DIRTY))
	{
		entity->AddFlag(Entity::TRANSFORM_DIRTY);
		Entity * parent = entity->GetParent();
		if(parent && parent->GetParent())
		{
			HierahicAddToUpdate(entity->GetParent());
		}
		else
		{//topmost parent
			DVASSERT(entity->GetRetainCount() >= 1);
			updatableEntities.push_back(entity);
		}
	}
}

void TransformSystem::AddEntity(Entity * entity)
{
	TransformComponent * transform = (TransformComponent*)entity->GetComponent(Component::TRANSFORM_COMPONENT);
	if (!transform) return; //just in case
	if(transform->parentMatrix)	
		transform->worldMatrix = transform->localMatrix * *(transform->parentMatrix);
	else
		transform->worldMatrix = transform->localMatrix;
}

void TransformSystem::RemoveEntity(Entity * entity)
{
	//TODO: use hashmap
	uint32 size = updatableEntities.size();
	for(uint32 i = 0; i < size; ++i)
	{
		if(updatableEntities[i] == entity)
		{
			entity->RemoveFlag(Entity::TRANSFORM_NEED_UPDATE);
			entity->RemoveFlag(Entity::TRANSFORM_DIRTY);

			updatableEntities[i] = updatableEntities[size-1];
			updatableEntities.pop_back();
			return;
		}
	}
}

};
