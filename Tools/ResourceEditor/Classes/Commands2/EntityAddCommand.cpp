#include "Commands2/EntityAddCommand.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

EntityAddCommand::EntityAddCommand(DAVA::Entity* _entityToAdd, DAVA::Entity* toParent)
    : Command2(CMDID_ENTITY_ADD, "Add Entity")
    , entityToAdd(_entityToAdd)
    , parentToAdd(toParent)
{
    SafeRetain(entityToAdd);
}

EntityAddCommand::~EntityAddCommand()
{
    SafeRelease(entityToAdd);
}

void EntityAddCommand::Undo()
{
    if (NULL != parentToAdd && NULL != entityToAdd)
    {
        parentToAdd->RemoveNode(entityToAdd);
    }
}

void EntityAddCommand::Redo()
{
    if (parentToAdd)
    {
        parentToAdd->AddNode(entityToAdd);

        //Workaround for correctly adding of switch
        DAVA::SwitchComponent* sw = GetSwitchComponent(entityToAdd);
        if (sw)
        {
            sw->SetSwitchIndex(sw->GetSwitchIndex());
        }
    }
}

DAVA::Entity* EntityAddCommand::GetEntity() const
{
    return entityToAdd;
}