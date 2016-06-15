#ifndef __ENTITY_LOCK_COMMAND_H__
#define __ENTITY_LOCK_COMMAND_H__

#include "Commands2/Base/Command2.h"

class EntityLockCommand : public Command2
{
public:
    EntityLockCommand(DAVA::Entity* entity, bool lock);
    ~EntityLockCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const;

    DAVA::Entity* entity;
    bool oldState;
    bool newState;
};

#endif // __ENTITY_LOCK_COMMAND_H__
