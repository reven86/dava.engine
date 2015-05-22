#include "InsertControlCommand.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/PackageNode.h"

using namespace DAVA;

InsertControlCommand::InsertControlCommand(PackageNode *_root, ControlNode *_node, ControlsContainerNode *_dest, int _index, QUndoCommand *parent)
    : QUndoCommand(parent)
    , root(SafeRetain(_root))
    , node(SafeRetain(_node))
    , dest(SafeRetain(_dest))
    , index(_index)
{
    
}

InsertControlCommand::~InsertControlCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(dest);
}

void InsertControlCommand::redo()
{
    root->InsertControl(node, dest, index);
}

void InsertControlCommand::undo()
{
    root->RemoveControl(node, dest);
}

