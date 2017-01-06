#ifndef __QUICKED_REMOVE_COMPONENT_COMMAND_H__
#define __QUICKED_REMOVE_COMPONENT_COMMAND_H__

#include "Command/Command.h"

class PackageNode;
class ControlNode;
class ComponentPropertiesSection;

class RemoveComponentCommand : public DAVA::Command
{
public:
    RemoveComponentCommand(PackageNode* _root, ControlNode* _node, ComponentPropertiesSection* _section);
    virtual ~RemoveComponentCommand();

    void Redo() override;
    void Undo() override;

private:
    PackageNode* root;
    ControlNode* node;
    ComponentPropertiesSection* componentSection;
};

#endif // __QUICKED_REMOVE_COMPONENT_COMMAND_H__
