#ifndef __ENTITY_REMOVE_COMMAND_H__
#define __ENTITY_REMOVE_COMMAND_H__

#include "Commands2/Base/RECommand.h"

class EntityRemoveCommand : public RECommand
{
public:
    EntityRemoveCommand(DAVA::Entity* entity);
    ~EntityRemoveCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const;

    DAVA::Entity* entity;
    DAVA::Entity* before;
    DAVA::Entity* parent;
};

#endif // __ENTITY_REMOVE_COMMAND_H__
