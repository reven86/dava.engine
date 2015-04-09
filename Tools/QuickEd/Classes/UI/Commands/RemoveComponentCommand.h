#ifndef __QUICKED_REMOVE_COMPONENT_COMMAND_H__
#define __QUICKED_REMOVE_COMPONENT_COMMAND_H__

#include <QUndoCommand>

class PropertiesContext;
class ControlNode;
class ComponentPropertiesSection;

class RemoveComponentCommand : public QUndoCommand
{
public:
    RemoveComponentCommand(PropertiesContext *_context, ControlNode *node, int componentType, int componentIndex, QUndoCommand *parent = nullptr);
    virtual ~RemoveComponentCommand();
    
    void redo() override;
    void undo() override;
    
private:
    PropertiesContext *context;
    ControlNode *node;
    int componentType;
    int componentIndex;
    ComponentPropertiesSection *componentSection;
};

#endif // __QUICKED_REMOVE_COMPONENT_COMMAND_H__
