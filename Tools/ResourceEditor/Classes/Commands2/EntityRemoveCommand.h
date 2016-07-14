#ifndef __ENTITY_REMOVE_COMMAND_H__
#define __ENTITY_REMOVE_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"

namespace DAVA
{
    class Entity;
}

class EntityRemoveCommand : public CommandWithoutExecute
{
public:
    EntityRemoveCommand(DAVA::Entity* entity);
    ~EntityRemoveCommand();

    void Undo() override;
    void Redo() override;
    DAVA::Entity* GetEntity() const;

    DAVA::Entity* entity;
    DAVA::Entity* before;
    DAVA::Entity* parent;
};

#endif // __ENTITY_REMOVE_COMMAND_H__
