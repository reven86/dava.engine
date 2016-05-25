#include "Scene3D/Systems/TransformSystem.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Entity.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Debug/Stats.h"
#include "Scene3D/Components/ComponentHelpers.h"

namespace DAVA
{
TransformSystem::TransformSystem(Scene* scene)
    : SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::LOCAL_TRANSFORM_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::TRANSFORM_PARENT_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::ANIMATION_TRANSFORM_CHANGED);
}

TransformSystem::~TransformSystem()
{
}

void TransformSystem::LinkTransform(int32 parentIndex, int32 childIndex)
{
}

void TransformSystem::UnlinkTransform(int32 childIndex)
{
}

void TransformSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("TransformSystem::Process");

    passedNodes = 0;
    multipliedNodes = 0;

    uint32 size = static_cast<uint32>(updatableEntities.size());
    for (uint32 i = 0; i < size; ++i)
    {
        //HierahicFindUpdatableTransform(updatableEntities[i]);
        FindNodeThatRequireUpdate(updatableEntities[i]);
    }

    GlobalEventSystem::Instance()->GroupEvent(GetScene(), sendEvent, EventSystem::WORLD_TRANSFORM_CHANGED);
    sendEvent.clear();

    updatableEntities.clear();

    if (passedNodes)
    {
        //		Logger::Info("TransformSystem %d passed %d multiplied", passedNodes, multipliedNodes);
    }
}

void TransformSystem::FindNodeThatRequireUpdate(Entity* entity)
{
    //    stack1.push(entity);
    static const uint32 STACK_SIZE = 5000;
    uint32 stackPosition = 0;
    Entity* stack[STACK_SIZE];
    stack[stackPosition++] = entity;

    while (stackPosition > 0)
    {
        Entity* entity = stack[--stackPosition];

        if (entity->GetFlags() & Entity::TRANSFORM_NEED_UPDATE)
        {
            TransformAllChildEntities(entity);
        }
        else
        {
            entity->RemoveFlag(Entity::TRANSFORM_NEED_UPDATE | Entity::TRANSFORM_DIRTY);

            // We already marked all children as non-dirty if we entered to TransformAllChildEntities()
            uint32 size = entity->GetChildrenCount();
            for (uint32 i = 0; i < size; ++i)
            {
                Entity* childEntity = entity->GetChild(i);
                if (childEntity->GetFlags() & Entity::TRANSFORM_DIRTY)
                {
                    DVASSERT(stackPosition < STACK_SIZE - 1);
                    stack[stackPosition++] = childEntity;
                }
            }
        }
    }
    DVASSERT(stackPosition == 0);
}

void TransformSystem::TransformAllChildEntities(Entity* entity)
{
    static const uint32 STACK_SIZE = 5000;
    uint32 stackPosition = 0;
    Entity* stack[STACK_SIZE];
    stack[stackPosition++] = entity;

    int32 localMultiplied = 0;

    while (stackPosition > 0)
    {
        Entity* entity = stack[--stackPosition];

        TransformComponent* transform = static_cast<TransformComponent*>(entity->GetComponent(Component::TRANSFORM_COMPONENT));
        if (transform->parentMatrix)
        {
            AnimationComponent* animComp = GetAnimationComponent(entity);
            localMultiplied++;
            if (animComp)
                transform->worldMatrix = animComp->animationTransform * transform->localMatrix * *(transform->parentMatrix);
            else
                transform->worldMatrix = transform->localMatrix * *(transform->parentMatrix);
            //GlobalEventSystem::Instance()->Event(entity, EventSystem::WORLD_TRANSFORM_CHANGED);
            sendEvent.push_back(transform);
        }

        entity->RemoveFlag(Entity::TRANSFORM_NEED_UPDATE | Entity::TRANSFORM_DIRTY);

        //        Vector<Entity*> & children = entity->children;
        //        uint32 childrenSize = children.size();
        //        std::memcpy(&(stack[stackPosition]), &(children[0]), childrenSize*sizeof(Entity*));
        //        DVASSERT(stackPosition < STACK_SIZE - childrenSize);
        //        stackPosition += childrenSize;

        uint32 size = entity->GetChildrenCount();
        for (uint32 i = 0; i < size; ++i)
        {
            DVASSERT(stackPosition < STACK_SIZE - 1);
            stack[stackPosition++] = entity->GetChild(i);
        }
    }
    DVASSERT(stackPosition == 0);
    multipliedNodes += localMultiplied;
}

void TransformSystem::HierahicFindUpdatableTransform(Entity* entity, bool forcedUpdate)
{
    passedNodes++;

    if (forcedUpdate || entity->GetFlags() & Entity::TRANSFORM_NEED_UPDATE)
    {
        forcedUpdate = true;
        multipliedNodes++;
        TransformComponent* transform = static_cast<TransformComponent*>(entity->GetComponent(Component::TRANSFORM_COMPONENT));
        if (transform->parentMatrix)
        {
            transform->worldMatrix = transform->localMatrix * *(transform->parentMatrix);
            GlobalEventSystem::Instance()->Event(transform, EventSystem::WORLD_TRANSFORM_CHANGED);
        }
    }

    uint32 size = entity->GetChildrenCount();
    for (uint32 i = 0; i < size; ++i)
    {
        if (forcedUpdate || entity->GetChild(i)->GetFlags() & Entity::TRANSFORM_DIRTY)
        {
            HierahicFindUpdatableTransform(entity->GetChild(i), forcedUpdate);
        }
    }

    entity->RemoveFlag(Entity::TRANSFORM_NEED_UPDATE);
    entity->RemoveFlag(Entity::TRANSFORM_DIRTY);
}

void TransformSystem::SortAndThreadSplit()
{
}

void TransformSystem::ImmediateEvent(Component* component, uint32 event)
{
    Entity* entity = component->GetEntity();
    switch (event)
    {
    case EventSystem::LOCAL_TRANSFORM_CHANGED:
    case EventSystem::TRANSFORM_PARENT_CHANGED:
    case EventSystem::ANIMATION_TRANSFORM_CHANGED:
        EntityNeedUpdate(entity);
        HierahicAddToUpdate(entity);
        break;
    }
}

void TransformSystem::EntityNeedUpdate(Entity* entity)
{
    entity->AddFlag(Entity::TRANSFORM_NEED_UPDATE);
}

void TransformSystem::HierahicAddToUpdate(Entity* entity)
{
    if (!(entity->GetFlags() & Entity::TRANSFORM_DIRTY))
    {
        entity->AddFlag(Entity::TRANSFORM_DIRTY);
        Entity* parent = entity->GetParent();
        if (parent && parent->GetParent())
        {
            HierahicAddToUpdate(entity->GetParent());
        }
        else
        { //topmost parent
            DVASSERT(entity->GetRetainCount() >= 1);
            updatableEntities.push_back(entity);
        }
    }
}

void TransformSystem::AddEntity(Entity* entity)
{
    TransformComponent* transform = static_cast<TransformComponent*>(entity->GetComponent(Component::TRANSFORM_COMPONENT));
    if (!transform)
        return; //just in case
    if (transform->parentMatrix)
        transform->worldMatrix = transform->localMatrix * *(transform->parentMatrix);
    else
        transform->worldMatrix = transform->localMatrix;
}

void TransformSystem::RemoveEntity(Entity* entity)
{
    //TODO: use hashmap
    uint32 size = static_cast<uint32>(updatableEntities.size());
    for (uint32 i = 0; i < size; ++i)
    {
        if (updatableEntities[i] == entity)
        {
            updatableEntities[i] = updatableEntities[size - 1];
            updatableEntities.pop_back();
            break;
        }
    }

    entity->RemoveFlag(Entity::TRANSFORM_NEED_UPDATE);
    entity->RemoveFlag(Entity::TRANSFORM_DIRTY);
}
};
