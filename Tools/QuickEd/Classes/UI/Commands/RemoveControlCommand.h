#ifndef __QUICKED_REMOVE_CONTROL_COMMAND_H__
#define __QUICKED_REMOVE_CONTROL_COMMAND_H__

#include <QUndoCommand>

class PackageNode;
class ControlNode;
class ControlsContainerNode;

class RemoveControlCommand : public QUndoCommand
{
public:
    RemoveControlCommand(PackageNode *_root, ControlNode *_node, ControlsContainerNode *_from, int _index, QUndoCommand *parent = nullptr);
    virtual ~RemoveControlCommand();
    
    void redo() override;
    void undo() override;
    
private:
    PackageNode *root;
    ControlNode *node;
    ControlsContainerNode *from;
    int index;
};

#endif // __QUICKED_REMOVE_CONTROL_COMMAND_H__
