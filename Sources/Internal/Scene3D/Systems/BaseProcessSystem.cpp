#include "Scene3D/Systems/BaseProcessSystem.h"
#include "Scene3D/Entity.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
BaseProcessSystem::BaseProcessSystem(uint32 componentId, Scene* scene)
    : SceneSystem(scene)
    ,
    processingComponentId(componentId)
{
}

void BaseProcessSystem::AddComponent(Entity* entity, Component* component)
{
    components.push_back(component);
}

void BaseProcessSystem::RemoveComponent(Entity* entity, Component* component)
{
    uint32 size = static_cast<uint32>(components.size());
    for (uint32 i = 0; i < size; ++i)
    {
        if (components[i] == component)
        {
            components[i] = components[size - 1];
            components.pop_back();
            return;
        }
    }

    DVASSERT(0);
}

void BaseProcessSystem::AddEntity(Entity* entity)
{
    uint32 size = entity->GetComponentCount(processingComponentId);
    for (uint32 i = 0; i < size; ++i)
    {
        AddComponent(entity, entity->GetComponent(processingComponentId, i));
    }
}

void BaseProcessSystem::RemoveEntity(Entity* entity)
{
    uint32 size = entity->GetComponentCount(processingComponentId);
    for (uint32 i = 0; i < size; ++i)
    {
        RemoveComponent(entity, entity->GetComponent(processingComponentId, i));
    }
}
}