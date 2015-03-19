#ifndef __QUICKED_INSERT_CONTROL_COMMAND_H__
#define __QUICKED_INSERT_CONTROL_COMMAND_H__

#include <QUndoStack>

class PackageModel;
class ControlNode;
class ControlsContainerNode;

class InsertControlCommand : public QUndoCommand
{
public:
    InsertControlCommand(PackageModel *_model, ControlNode *node, ControlsContainerNode *dest, int index, QUndoCommand *parent = nullptr);
    virtual ~InsertControlCommand();
 
    void undo() override;
    void redo() override;

private:
    PackageModel *model;
    ControlNode *node;
    ControlsContainerNode *dest;
    int index;
};

#endif // __QUICKED_INSERT_CONTROL_COMMAND_H__
