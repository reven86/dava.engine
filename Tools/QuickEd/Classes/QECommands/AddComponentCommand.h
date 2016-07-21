#ifndef __QUICKED_ADD_COMPONENT_COMMAND_H__
#define __QUICKED_ADD_COMPONENT_COMMAND_H__

#include "Command/Command.h"

class PackageNode;
class ControlNode;
class ComponentPropertiesSection;

class AddComponentCommand : public DAVA::Command
{
public:
    AddComponentCommand(PackageNode* root, ControlNode* node, ComponentPropertiesSection* section);

    virtual ~AddComponentCommand();

    void Redo() override;
    void Undo() override;

private:
    PackageNode* root;
    ControlNode* node;
    ComponentPropertiesSection* section;
};

#endif // __QUICKED_ADD_COMPONENT_COMMAND_H__
