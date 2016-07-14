#ifndef __ENTITY_LOCK_COMMAND_H__
#define __ENTITY_LOCK_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"

namespace DAVA
{
class Entity;
}

class EntityLockCommand : public CommandWithoutExecute
{
public:
    EntityLockCommand(DAVA::Entity* entity, bool lock);
    ~EntityLockCommand();

    void Undo() override;
    void Redo() override;
    DAVA::Entity* GetEntity() const;

    DAVA::Entity* entity;
    bool oldState;
    bool newState;
};

#endif // __ENTITY_LOCK_COMMAND_H__
