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
#include "Job/JobScheduler.h"
#include "Job/JobWaiter.h"

namespace DAVA
{

TransformSystem::TransformSystem(Scene * scene)
:	SceneSystem(scene)
{
	scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::LOCAL_TRANSFORM_CHANGED);
	scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::TRANSFORM_PARENT_CHANGED);
}

TransformSystem::~TransformSystem()                                                                                                                                                   

{
}


void TransformSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("TransformSystem::Process");
    
    passedNodes = 0;
    multipliedNodes = 0;
    
    if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::IMPOSTERS_ENABLE))
    {
        uint32 size = updatableEntities.size();
        for(uint32 i = 0; i < size; ++i)
        {
            JobArgument * arg = new JobArgument();
            arg->entity = updatableEntities[i];
            arg->forceUpdate = false;
            arg->recursionDepth = 0;

            HierahicFindUpdatableTransform(0, arg, 0);
        }

        TaggedWorkerJobsWaiter waiter(2);
        waiter.Wait();
        
        for(Map<Thread::ThreadId, Vector<Entity*> >::iterator it = eventMap.begin(), itEnd = eventMap.end(); it != itEnd; ++it)
        {
            Vector<Entity*> & vector = (*it).second;
            GlobalEventSystem::Instance()->GroupEvent(GetScene(), vector, EventSystem::WORLD_TRANSFORM_CHANGED);
            vector.clear();
        }
    }
    else
    {
        uint32 size = updatableEntities.size();
        for(uint32 i = 0; i < size; ++i)
        {
            FindNodeThatRequireUpdate(updatableEntities[i]);
        }
        
        GlobalEventSystem::Instance()->GroupEvent(GetScene(), sendEvent, EventSystem::WORLD_TRANSFORM_CHANGED);
        sendEvent.clear();
    }

    
    updatableEntities.clear();
    
    if(passedNodes)
    {
        //		Logger::Info("TransformSystem %d passed %d multiplied", passedNodes, multipliedNodes);
    }
}

void TransformSystem::FindNodeThatRequireUpdate(Entity * entity)
{
    //    stack1.push(entity);
    static const uint32 STACK_SIZE = 5000;
    uint32 stackPosition = 0;
    Entity * stack[STACK_SIZE];
    stack[stackPosition++] = entity;
    
    while(stackPosition > 0)
    {
        Entity * entity = stack[--stackPosition];
        
        if (entity->GetFlags() & Entity::TRANSFORM_NEED_UPDATE)
        {
            TransformAllChildEntities(entity);
        }
        else
        {
            entity->RemoveFlag(Entity::TRANSFORM_NEED_UPDATE | Entity::TRANSFORM_DIRTY);
            
            // We already marked all children as non-dirty if we entered to TransformAllChildEntities()
            uint32 size = entity->GetChildrenCount();
            for(uint32 i = 0; i < size; ++i)
            {
                Entity * childEntity = entity->GetChild(i);
                if(childEntity->GetFlags() & Entity::TRANSFORM_DIRTY)
                {
                    DVASSERT(stackPosition < STACK_SIZE - 1);
                    stack[stackPosition++] = childEntity;
                }
            }
        }
    }
    DVASSERT(stackPosition == 0);
    
}

void TransformSystem::TransformAllChildEntities(Entity * entity)
{
    static const uint32 STACK_SIZE = 5000;
    uint32 stackPosition = 0;
    Entity * stack[STACK_SIZE];
    stack[stackPosition++] = entity;
    
    int32 localMultiplied = 0;
    
    while(stackPosition > 0)
    {
        Entity * entity = stack[--stackPosition];
        
        TransformComponent * transform = (TransformComponent*)entity->GetComponent(Component::TRANSFORM_COMPONENT);
        if(transform->parentMatrix)
        {
            localMultiplied++;
            transform->worldMatrix = transform->localMatrix * *(transform->parentMatrix);
            //GlobalEventSystem::Instance()->Event(entity, EventSystem::WORLD_TRANSFORM_CHANGED);
            sendEvent.push_back(entity);
        }
        
        entity->RemoveFlag(Entity::TRANSFORM_NEED_UPDATE | Entity::TRANSFORM_DIRTY);
        
        //        Vector<Entity*> & children = entity->children;
        //        uint32 childrenSize = children.size();
        //        std::memcpy(&(stack[stackPosition]), &(children[0]), childrenSize*sizeof(Entity*));
        //        DVASSERT(stackPosition < STACK_SIZE - childrenSize);
        //        stackPosition += childrenSize;
        
        uint32 size = entity->GetChildrenCount();
        for(uint32 i = 0; i < size; ++i)
        {
            DVASSERT(stackPosition < STACK_SIZE - 1);
            stack[stackPosition++] = entity->GetChild(i);
        }
    }
    DVASSERT(stackPosition == 0);
    multipliedNodes += localMultiplied;
}


void TransformSystem::HierahicFindUpdatableTransform(BaseObject * bo, void * userData, void * callerData)
{
    JobArgument * arg = (JobArgument*)userData;
    Entity * entity = arg->entity;
    bool & forcedUpdate = arg->forceUpdate;
    int32 recursionDepth = arg->recursionDepth;

	passedNodes++;

	if(forcedUpdate || entity->GetFlags() & Entity::TRANSFORM_NEED_UPDATE)
	{
		forcedUpdate = true;
		multipliedNodes++;
		TransformComponent * transform = (TransformComponent*)entity->GetComponent(Component::TRANSFORM_COMPONENT);
		if(transform->parentMatrix)
		{
			transform->worldMatrix = transform->localMatrix * *(transform->parentMatrix);
            eventMap[Thread::GetCurrentThreadId()].push_back(entity);
		}
	}

	uint32 size = entity->GetChildrenCount();
	for(uint32 i = 0; i < size; ++i)
	{
		if(forcedUpdate || entity->GetChild(i)->GetFlags() & Entity::TRANSFORM_DIRTY)
		{
            JobArgument * arg = new JobArgument();
            arg->entity = entity->GetChild(i);
            arg->forceUpdate = forcedUpdate;
            arg->recursionDepth = recursionDepth+1;
            if(arg->recursionDepth == 1)
            {
                Job * j = new Job(Message(this, &TransformSystem::HierahicFindUpdatableTransform, arg), Thread::GetCurrentThreadId(), 0, 2);
                JobScheduler::Instance()->PushJob(j);
                j->Release();
            }
            else
            {
                HierahicFindUpdatableTransform(0, arg, 0);
            }
		}
	}

	entity->RemoveFlag(Entity::TRANSFORM_NEED_UPDATE);
	entity->RemoveFlag(Entity::TRANSFORM_DIRTY);

    delete arg;
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
