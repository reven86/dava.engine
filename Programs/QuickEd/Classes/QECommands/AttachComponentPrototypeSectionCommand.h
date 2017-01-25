#ifndef __QUICKED_ATTACH_PROTOTYPE_COMPONENT_SECTION_COMMAND_H__
#define __QUICKED_ATTACH_PROTOTYPE_COMPONENT_SECTION_COMMAND_H__

#include "Command/Command.h"

class PackageNode;
class ControlNode;
class ComponentPropertiesSection;

class AttachComponentPrototypeSectionCommand : public DAVA::Command
{
public:
    AttachComponentPrototypeSectionCommand(PackageNode* root, ControlNode* node, ComponentPropertiesSection* destSection, ComponentPropertiesSection* prototypeSection);
    virtual ~AttachComponentPrototypeSectionCommand();

    void Redo() override;
    void Undo() override;

private:
    PackageNode* root;
    ControlNode* node;
    ComponentPropertiesSection* destSection;
    ComponentPropertiesSection* prototypeSection;
};

#endif // __QUICKED_ATTACH_PROTOTYPE_COMPONENT_SECTION_COMMAND_H__
