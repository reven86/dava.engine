#ifndef __ENTITY_ADD_COMMAND_H__
#define __ENTITY_ADD_COMMAND_H__

#include "Commands2/Base/Command2.h"

class EntityAddCommand : public Command2
{
public:
    EntityAddCommand(DAVA::Entity* entityToAdd, DAVA::Entity* toParent);
    ~EntityAddCommand();

    virtual void Undo();
    virtual void Redo();

    virtual DAVA::Entity* GetEntity() const;

    DAVA::Entity* entityToAdd;
    DAVA::Entity* parentToAdd;
};

#endif // __ENTITY_ADD_COMMAND_H__
