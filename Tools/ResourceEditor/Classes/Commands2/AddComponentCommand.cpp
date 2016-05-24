#include "Commands2/AddComponentCommand.h"
#include "DAVAEngine.h"

AddComponentCommand::AddComponentCommand(DAVA::Entity* _entity, DAVA::Component* _component)
    : Command2(CMDID_COMPONENT_ADD, "Add Component")
    , entity(_entity)
    , component(_component)
{
    DVASSERT(entity);
    DVASSERT(component);
}

AddComponentCommand::~AddComponentCommand()
{
    SafeDelete(backup);
}

void AddComponentCommand::Redo()
{
    entity->AddComponent(component);
    backup = nullptr;
}

void AddComponentCommand::Undo()
{
    backup = component;
    entity->DetachComponent(component);
}

DAVA::Entity* AddComponentCommand::GetEntity() const
{
    return entity;
}

const DAVA::Component* AddComponentCommand::GetComponent() const
{
    return component;
}
