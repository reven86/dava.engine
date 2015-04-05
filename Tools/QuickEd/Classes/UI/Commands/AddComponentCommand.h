#ifndef __QUICKED_ADD_COMPONENT_COMMAND_H__
#define __QUICKED_ADD_COMPONENT_COMMAND_H__

#include <QUndoStack>

class PropertiesModel;
class ControlNode;

class AddComponentCommand : public QUndoCommand
{
public:
    AddComponentCommand(PropertiesModel *_model, ControlNode *node, int componentType, QUndoCommand *parent = nullptr);
    virtual ~AddComponentCommand();
    
    void redo() override;
    void undo() override;
    
private:
    PropertiesModel *model;
    ControlNode *node;
    int componentType;
};

#endif // __QUICKED_ADD_COMPONENT_COMMAND_H__
