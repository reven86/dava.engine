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
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Entity.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Debug/Stats.h"
#include "Job/JobManager.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{

TransformSystem::TransformSystem(Scene * scene)
:	SceneSystem(scene)
{
	scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::LOCAL_TRANSFORM_CHANGED);
	scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::TRANSFORM_PARENT_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::ANIMATION_TRANSFORM_CHANGED);
}

TransformSystem::~TransformSystem()                                                                                                                                                   
{ }

static void test_fn()
{

}

void TransformSystem::Process(float32 timeElapsed)
{
	static int iii = 0;
	uint64 _sss1;
	uint64 _sss2;
	uint64 _sss3;
	static uint64 ttt0 = 0;
	static uint64 ttt1 = 0;
	static uint64 ttt2 = 0;
	static uint64 ttt3 = 0;
	static uint64 ttt4 = 0;
	static bool _in = false;

	_sss1 = SystemTimer::Instance()->GetAbsoluteNano();
    TIME_PROFILE("TransformSystem::Process");
    
    passedNodes = 0;
    multipliedNodes = 0;

	// calculate optimal jobs count for current number of entities should be processed
	uint32 entitiesCount = updatableEntities.size();
	uint32 jobsCount = Min(maxProcessingThreads, Min(1 + entitiesCount / 4, JobManager2::Instance()->GetWorkersCount()));
	uint32 entitiesPerJobCount = entitiesCount / jobsCount;

	if(_in && jobsCount > 1 && RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::TEST_OPTION))
    {
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
			//Function<void()> fn = Bind(MakeFunction(this, &TransformSystem::UpdateHierarchyPart), firstIndex, count, &sendEvent[i]);
			//JobManager2::Instance()->CreateWorkerJob(fn);
			JobManager2::Instance()->CreateWorkerJob(&test_fn);

			// move to next part of entities
			firstIndex += count;
		}

		_sss2 = SystemTimer::Instance()->GetAbsoluteNano();
		JobManager2::Instance()->WaitWorkerJobs();

		_sss3 = SystemTimer::Instance()->GetAbsoluteNano();
		for(uint32 i = 0; i < jobsCount; ++i)
		{
			GlobalEventSystem::Instance()->GroupEvent(GetScene(), sendEvent[i], EventSystem::WORLD_TRANSFORM_CHANGED);
			sendEvent[i].clear();
		}
    }
    else
    {
		_sss2 = SystemTimer::Instance()->GetAbsoluteNano();

		// single thread processing
        uint32 size = updatableEntities.size();
        for(uint32 i = 0; i < size; ++i)
        {
            UpdateHierarchy(updatableEntities[i], &sendEvent[0]);
        }
        
		_sss3 = SystemTimer::Instance()->GetAbsoluteNano();
        GlobalEventSystem::Instance()->GroupEvent(GetScene(), sendEvent[0], EventSystem::WORLD_TRANSFORM_CHANGED);
        sendEvent[0].clear();
    }
    
	if(_in)
	{
		ttt0 += (SystemTimer::Instance()->GetAbsoluteNano() - _sss1); // whole
		ttt1 += (_sss3 - _sss1); // cr_wait
		ttt2 += (_sss2 - _sss1); // cr
		ttt3 += (_sss3 - _sss2); // wait
		ttt4 += (SystemTimer::Instance()->GetAbsoluteNano() - _sss3); // events
	}
	else if(jobsCount > 1)
	{
		const uint64 ccc = 256;
		if(++iii == ccc)
		{
			iii = 0;

			_in = true;
			
			ttt0 = 0; ttt1 = 0; ttt2 = 0; ttt3 = 0;
			ttt0 = ttt0 / ccc;
			ttt1 = ttt1 / ccc;
			ttt2 = ttt2 / ccc;
			ttt3 = ttt3 / ccc;
			ttt4 = ttt4 / ccc;
			RenderManager::Instance()->GetOptions()->SetOption(RenderOptions::TEST_OPTION, true);
			for(size_t i = 0; i < 100; i++)
			{
				Process((float32) i);
			}

			Logger::Warning("TRAN_N: whole %8llu, cr_wait = %8llu, cr = %8llu, wait = %8llu, events = %8llu", ttt0, ttt1, ttt2, ttt3, ttt4);

			ttt0 = 0; ttt1 = 0; ttt2 = 0; ttt3 = 0;
			ttt0 = ttt0 / ccc;
			ttt1 = ttt1 / ccc;
			ttt2 = ttt2 / ccc;
			ttt3 = ttt3 / ccc;
			ttt4 = ttt4 / ccc;
			RenderManager::Instance()->GetOptions()->SetOption(RenderOptions::TEST_OPTION, false);
			for(size_t i = 0; i < 100; i++)
			{
				Process((float32)i);
			}

			Logger::Warning("TRAN_1: whole %8llu, cr_wait = %8llu, cr = %8llu, wait = %8llu, events = %8llu", ttt0, ttt1, ttt2, ttt3, ttt4);

			_in = false;
		}

		updatableEntities.clear();
	}

	// TODO: uncomment
	//updatableEntities.clear();
    
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
            AnimationComponent * animComp = GetAnimationComponent(entity);
            if (animComp)
                transform->worldMatrix = animComp->animationTransform * transform->localMatrix * *(transform->parentMatrix);
            else
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
    case EventSystem::ANIMATION_TRANSFORM_CHANGED:
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
