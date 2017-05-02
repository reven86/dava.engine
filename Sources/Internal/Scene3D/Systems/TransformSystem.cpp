#include "Scene3D/Systems/TransformSystem.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Entity.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"

namespace DAVA
{
TransformSystem::TransformSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void TransformSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_TRANSFORM_SYSTEM);

    TransformSingleComponent* tsc = GetScene()->transformSingleComponent;
    for (Entity* e : tsc->localTransformChanged)
    {
        EntityNeedUpdate(e);
        HierahicAddToUpdate(e);
    }
    for (Entity* e : tsc->transformParentChanged)
    {
        EntityNeedUpdate(e);
        HierahicAddToUpdate(e);
    }
    for (Entity* e : tsc->animationTransformChanged)
    {
        EntityNeedUpdate(e);
        HierahicAddToUpdate(e);
    }

    passedNodes = 0;
    multipliedNodes = 0;

    uint32 size = static_cast<uint32>(updatableEntities.size());
    for (uint32 i = 0; i < size; ++i)
    {
        FindNodeThatRequireUpdate(updatableEntities[i]);
    }
    updatableEntities.clear();

    if (passedNodes)
    {
        //		Logger::Info("TransformSystem %d passed %d multiplied", passedNodes, multipliedNodes);
    }
}

void TransformSystem::FindNodeThatRequireUpdate(Entity* entity)
{
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
            TransformSingleComponent* tsc = GetScene()->transformSingleComponent;
            tsc->worldTransformChanged.Push(entity);
        }

        entity->RemoveFlag(Entity::TRANSFORM_NEED_UPDATE | Entity::TRANSFORM_DIRTY);

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
            TransformSingleComponent* tsc = GetScene()->transformSingleComponent;
            tsc->worldTransformChanged.Push(entity);
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
    EntityNeedUpdate(entity);
    HierahicAddToUpdate(entity);
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
