#ifndef __QUICKED_ADD_COMPONENT_COMMAND_H__
#define __QUICKED_ADD_COMPONENT_COMMAND_H__

#include <QUndoCommand>

class PackageNode;
class ControlNode;
class ComponentPropertiesSection;

class AddComponentCommand : public QUndoCommand
{
public:
    AddComponentCommand(PackageNode *root, ControlNode *node, int componentType, QUndoCommand *parent = nullptr);
    virtual ~AddComponentCommand();
    
    void redo() override;
    void undo() override;
    
private:
    PackageNode *root;
    ControlNode *node;
    ComponentPropertiesSection *componentSection;
};

#endif // __QUICKED_ADD_COMPONENT_COMMAND_H__
