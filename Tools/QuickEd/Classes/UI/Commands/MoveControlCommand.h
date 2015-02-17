#ifndef __QUICKED_MOVE_CONTROL_COMMAND_H__
#define __QUICKED_MOVE_CONTROL_COMMAND_H__

#include <QUndoStack>

class PackageModel;
class ControlNode;
class ControlsContainerNode;

class MoveControlCommand : public QUndoCommand
{
public:
    MoveControlCommand(PackageModel *_model, ControlNode *node, ControlsContainerNode *src, int srcIndex, ControlsContainerNode *dest, int destIndex, QUndoCommand *parent = nullptr);
    virtual ~MoveControlCommand();
    
    void undo() override;
    void redo() override;
    
private:
    PackageModel *model;
    
    ControlNode *node;
    
    ControlsContainerNode *src;
    int srcIndex;
    
    ControlsContainerNode *dest;
    int destIndex;
};

#endif // __QUICKED_MOVE_CONTROL_COMMAND_H__
