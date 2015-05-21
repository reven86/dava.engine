#ifndef __QUICKED_INSERT_CONTROL_COMMAND_H__
#define __QUICKED_INSERT_CONTROL_COMMAND_H__

#include <QUndoCommand>

class PackageNode;
class ControlNode;
class ControlsContainerNode;

class InsertControlCommand : public QUndoCommand
{
public:
    InsertControlCommand(PackageNode *_root, ControlNode *_node, ControlsContainerNode *_dest, int _index, QUndoCommand *parent = nullptr);
    virtual ~InsertControlCommand();
 
    void redo() override;
    void undo() override;

private:
    PackageNode *root;
    ControlNode *node;
    ControlsContainerNode *dest;
    int index;
};

#endif // __QUICKED_INSERT_CONTROL_COMMAND_H__
