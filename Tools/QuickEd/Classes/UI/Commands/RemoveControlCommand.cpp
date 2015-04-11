#include "RemoveControlCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

using namespace DAVA;

RemoveControlCommand::RemoveControlCommand(PackageNode *_root, ControlNode *_node, ControlsContainerNode *_from, int _index, QUndoCommand *parent)
    : QUndoCommand(parent)
    , root(SafeRetain(_root))
    , node(SafeRetain(_node))
    , from(SafeRetain(_from))
    , index(_index)
{
    
}

RemoveControlCommand::~RemoveControlCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(from);
}

void RemoveControlCommand::redo()
{
    root->RemoveControl(node, from);
}

void RemoveControlCommand::undo()
{
    root->InsertControl(node, from, index);
}

