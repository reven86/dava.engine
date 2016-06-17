#ifndef __QUICKED_INSERT_CONTROL_COMMAND_H__
#define __QUICKED_INSERT_CONTROL_COMMAND_H__

#include "Document/CommandsBase/Command.h"

class PackageNode;
class ControlNode;
class ControlsContainerNode;

class InsertControlCommand : public QECommand
{
public:
    InsertControlCommand(PackageNode* _root, ControlNode* _node, ControlsContainerNode* _dest, int _index);
    virtual ~InsertControlCommand();

    void Redo() override;
    void Undo() override;

private:
    PackageNode* root;
    ControlNode* node;
    ControlsContainerNode* dest;
    int index;
};

#endif // __QUICKED_INSERT_CONTROL_COMMAND_H__
