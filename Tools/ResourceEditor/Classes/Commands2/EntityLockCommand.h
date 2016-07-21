#ifndef __ENTITY_LOCK_COMMAND_H__
#define __ENTITY_LOCK_COMMAND_H__

#include "Commands2/Base/RECommand.h"

namespace DAVA
{
class Entity;
}

class EntityLockCommand : public RECommand
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
