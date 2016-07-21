#include "Commands2/EntityLockCommand.h"
#include "Commands2/RECommandIDs.h"
#include "Scene3D/Entity.h"

EntityLockCommand::EntityLockCommand(DAVA::Entity* _entity, bool lock)
    : RECommand(CMDID_ENTITY_LOCK, "Lock entity")
    , entity(_entity)
    , newState(lock)
{
    DVASSERT(NULL != entity);
    oldState = entity->GetLocked();
}

EntityLockCommand::~EntityLockCommand()
{
}

void EntityLockCommand::Undo()
{
    DVASSERT(NULL != entity);
    entity->SetLocked(oldState);
}

void EntityLockCommand::Redo()
{
    DVASSERT(NULL != entity);
    entity->SetLocked(newState);
}

DAVA::Entity* EntityLockCommand::GetEntity() const
{
    return entity;
}
