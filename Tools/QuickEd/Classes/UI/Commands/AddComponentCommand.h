#ifndef __QUICKED_ADD_COMPONENT_COMMAND_H__
#define __QUICKED_ADD_COMPONENT_COMMAND_H__

#include <QUndoStack>

class PropertiesContext;
class ControlNode;
class ComponentPropertiesSection;

class AddComponentCommand : public QUndoCommand
{
public:
    AddComponentCommand(PropertiesContext *_context, ControlNode *node, int componentType, QUndoCommand *parent = nullptr);
    virtual ~AddComponentCommand();
    
    void redo() override;
    void undo() override;
    
private:
    PropertiesContext *context;
    ControlNode *node;
    ComponentPropertiesSection *componentSection;
};

#endif // __QUICKED_ADD_COMPONENT_COMMAND_H__
