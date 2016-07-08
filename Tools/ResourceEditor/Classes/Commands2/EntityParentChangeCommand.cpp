#include "Commands2/EntityParentChangeCommand.h"

EntityParentChangeCommand::EntityParentChangeCommand(DAVA::Entity* _entity, DAVA::Entity* _newParent, DAVA::Entity* _newBefore /* = NULL */)
    : Command2(CMDID_ENTITY_CHANGE_PARENT, "Move entity")
    , entity(_entity)
    , oldParent(NULL)
    , oldBefore(NULL)
    , newParent(_newParent)
    , newBefore(_newBefore)
{
    SafeRetain(entity);

    if (NULL != entity)
    {
        oldParent = entity->GetParent();

        if (NULL != oldParent)
        {
            oldBefore = oldParent->GetNextChild(entity);
        }
    }
}

EntityParentChangeCommand::~EntityParentChangeCommand()
{
    SafeRelease(entity);
}

void EntityParentChangeCommand::Undo()
{
    if (NULL != entity)
    {
        if (NULL != oldParent)
        {
            if (NULL != oldBefore)
            {
                oldParent->InsertBeforeNode(entity, oldBefore);
            }
            else
            {
                oldParent->AddNode(entity);
            }
        }
        else
        {
            newParent->RemoveNode(entity);
        }
    }
}

void EntityParentChangeCommand::Redo()
{
    if (NULL != entity && NULL != newParent)
    {
        if (NULL != newBefore)
        {
            newParent->InsertBeforeNode(entity, newBefore);
        }
        else
        {
            newParent->AddNode(entity);
        }
    }
}

DAVA::Entity* EntityParentChangeCommand::GetEntity() const
{
    return entity;
}
