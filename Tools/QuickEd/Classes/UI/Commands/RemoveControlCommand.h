#ifndef __QUICKED_REMOVE_CONTROL_COMMAND_H__
#define __QUICKED_REMOVE_CONTROL_COMMAND_H__

#include <QUndoCommand>

class PackageModel;
class ControlNode;
class ControlsContainerNode;

class RemoveControlCommand : public QUndoCommand
{
public:
    RemoveControlCommand(PackageModel *_model, ControlNode *node, ControlsContainerNode *dest, int index, QUndoCommand *parent = nullptr);
    virtual ~RemoveControlCommand();
    
    void undo() override;
    void redo() override;
    
private:
    PackageModel *model;
    ControlNode *node;
    ControlsContainerNode *dest;
    int index;
};

#endif // __QUICKED_REMOVE_CONTROL_COMMAND_H__
