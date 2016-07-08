#ifndef __QUICKED_ATTACH_PROTOTYPE_COMPONENT_SECTION_COMMAND_H__
#define __QUICKED_ATTACH_PROTOTYPE_COMPONENT_SECTION_COMMAND_H__

#include <QUndoCommand>

class PackageNode;
class ControlNode;
class ComponentPropertiesSection;

class AttachComponentPrototypeSectionCommand : public QUndoCommand
{
public:
    AttachComponentPrototypeSectionCommand(PackageNode* root, ControlNode* node, ComponentPropertiesSection* destSection, ComponentPropertiesSection* prototypeSection, QUndoCommand* parent = nullptr);
    virtual ~AttachComponentPrototypeSectionCommand();

    void redo() override;
    void undo() override;

private:
    PackageNode* root;
    ControlNode* node;
    ComponentPropertiesSection* destSection;
    ComponentPropertiesSection* prototypeSection;
};

#endif // __QUICKED_ATTACH_PROTOTYPE_COMPONENT_SECTION_COMMAND_H__
