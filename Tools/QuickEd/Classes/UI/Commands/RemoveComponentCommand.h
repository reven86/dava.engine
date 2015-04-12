#ifndef __QUICKED_REMOVE_COMPONENT_COMMAND_H__
#define __QUICKED_REMOVE_COMPONENT_COMMAND_H__

#include <QUndoCommand>

class PackageNode;
class ControlNode;
class ComponentPropertiesSection;

class RemoveComponentCommand : public QUndoCommand
{
public:
    RemoveComponentCommand(PackageNode *_root, ControlNode *_node, int componentType, QUndoCommand *parent = nullptr);
    virtual ~RemoveComponentCommand();
    
    void redo() override;
    void undo() override;
    
private:
    PackageNode *root;
    ControlNode *node;
    ComponentPropertiesSection *componentSection;
};

#endif // __QUICKED_REMOVE_COMPONENT_COMMAND_H__
