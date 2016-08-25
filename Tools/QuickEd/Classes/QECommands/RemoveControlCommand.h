#ifndef __QUICKED_REMOVE_CONTROL_COMMAND_H__
#define __QUICKED_REMOVE_CONTROL_COMMAND_H__

#include "Command/Command.h"

class PackageNode;
class ControlNode;
class ControlsContainerNode;

class RemoveControlCommand : public DAVA::Command
{
public:
    RemoveControlCommand(PackageNode* _root, ControlNode* _node, ControlsContainerNode* _from, int _index);
    virtual ~RemoveControlCommand();

    void Redo() override;
    void Undo() override;

private:
    PackageNode* root;
    ControlNode* node;
    ControlsContainerNode* from;
    int index;
};

#endif // __QUICKED_REMOVE_CONTROL_COMMAND_H__
