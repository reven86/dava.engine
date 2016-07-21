#ifndef __REMOVE_COMPONENT_COMMAND_H__
#define __REMOVE_COMPONENT_COMMAND_H__

#include "Commands2/Base/RECommand.h"

namespace DAVA
{
class Entity;
class Component;
}

class RemoveComponentCommand : public RECommand
{
public:
    RemoveComponentCommand(DAVA::Entity* entity, DAVA::Component* component);
    ~RemoveComponentCommand() override;

    void Undo() override;
    void Redo() override;

    DAVA::Entity* GetEntity() const;
    const DAVA::Component* GetComponent() const;

private:
    DAVA::Entity* entity = nullptr;
    DAVA::Component* component = nullptr;
    DAVA::Component* backup = nullptr;
};

#endif // __REMOVE_COMPONENT_COMMAND_H__
