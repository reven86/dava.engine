#include "Scene3D/Entity.h"
#include "Platform/SystemTimer.h"
#include "Scene3D/Systems/ActionUpdateSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Debug/Stats.h"

namespace DAVA
{
ActionUpdateSystem::ActionUpdateSystem(Scene* scene)
    : SceneSystem(scene)
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
    for (int i = 0; i < ActionComponent::Action::EVENTS_COUNT; i++)
        eventBlocked[i] = false;
}

void ActionUpdateSystem::AddEntity(Entity* entity)
{
    SceneSystem::AddEntity(entity);
    ActionComponent* actionComponent = static_cast<ActionComponent*>(entity->GetComponent(Component::ACTION_COMPONENT));
    actionComponent->StartAdd();
}

void ActionUpdateSystem::RemoveEntity(Entity* entity)
{
    ActionComponent* actionComponent = static_cast<ActionComponent*>(entity->GetComponent(Component::ACTION_COMPONENT));
    if (actionComponent->IsStarted())
        UnWatch(actionComponent);
    SceneSystem::RemoveEntity(entity);
}

void ActionUpdateSystem::Process(float32 timeElapsed)
{
    PROFILER_TIMING("ActionUpdateSystem::Process");

    DelayedDeleteActions();

    uint32 size = static_cast<uint32>(activeActions.size());
    for (uint32 index = 0; index < size; ++index)
    {
        ActionComponent* component = activeActions[index];
        component->Update(timeElapsed);
    }
}

void ActionUpdateSystem::DelayedDeleteActions()
{
    Vector<ActionComponent*>::iterator end = deleteActions.end();
    for (Vector<ActionComponent*>::iterator it = deleteActions.begin(); it != end; ++it)
    {
        Vector<ActionComponent*>::iterator i = std::find(activeActions.begin(), activeActions.end(), *it);

        if (i != activeActions.end())
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