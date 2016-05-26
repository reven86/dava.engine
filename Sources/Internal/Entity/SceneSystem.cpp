#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
SceneSystem::SceneSystem(Scene* _scene)
    : requiredComponents(0)
    ,
    scene(_scene)
    , locked(false)
{
}

SceneSystem::~SceneSystem()
{
}

void SceneSystem::RegisterEntity(Entity* entity)
{
    uint64 requiredComponents = this->GetRequiredComponents();
    bool needAdd = ((requiredComponents & entity->GetAvailableComponentFlags()) == requiredComponents);

    if (needAdd)
        this->AddEntity(entity);
}

void SceneSystem::UnregisterEntity(Entity* entity)
{
    uint64 requiredComponents = this->GetRequiredComponents();
    bool needRemove = ((requiredComponents & entity->GetAvailableComponentFlags()) == requiredComponents);

    if (needRemove)
        this->RemoveEntity(entity);
}

bool SceneSystem::IsEntityComponentFitsToSystem(Entity* entity, Component* component)
{
    uint64 entityComponentFlags = entity->GetAvailableComponentFlags();
    uint64 componentToCheckType = MAKE_COMPONENT_MASK(component->GetType());
    uint64 requiredBySystemComponents = this->GetRequiredComponents();

    bool isAllRequiredComponentsAvailable = ((entityComponentFlags & requiredBySystemComponents) == requiredBySystemComponents);
    bool isComponentMarkedForCheckAvailable = ((requiredBySystemComponents & componentToCheckType) == componentToCheckType);

    return (isAllRequiredComponentsAvailable && isComponentMarkedForCheckAvailable);
}

void SceneSystem::RegisterComponent(Entity* entity, Component* component)
{
    if (IsEntityComponentFitsToSystem(entity, component))
    {
        if (entity->GetComponentCount(component->GetType()) == 1)
        {
            AddEntity(entity);
        }
        else
        {
            AddComponent(entity, component);
        }
    }
}

void SceneSystem::UnregisterComponent(Entity* entity, Component* component)
{
    if (IsEntityComponentFitsToSystem(entity, component))
    {
        if (entity->GetComponentCount(component->GetType()) == 1)
        {
            RemoveEntity(entity);
        }
        else
        {
            RemoveComponent(entity, component);
        }
    }
}

void SceneSystem::AddEntity(Entity* entity)
{
}

void SceneSystem::RemoveEntity(Entity* entity)
{
}

void SceneSystem::AddComponent(Entity* entity, Component* component)
{
}

void SceneSystem::RemoveComponent(Entity* entity, Component* component)
{
}

void SceneSystem::SceneDidLoaded()
{
}

void SceneSystem::ImmediateEvent(Component* component, uint32 event)
{
}

void SceneSystem::Process(float32 timeElapsed)
{
}

void SceneSystem::SetLocked(bool locked_)
{
    locked = locked_;
}

bool SceneSystem::IsLocked() const
{
    return locked;
}
};
