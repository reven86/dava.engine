#ifndef __ENTITY_ADD_COMMAND_H__
#define __ENTITY_ADD_COMMAND_H__

#include "Commands2/Base/RECommand.h"

class EntityAddCommand : public RECommand
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
