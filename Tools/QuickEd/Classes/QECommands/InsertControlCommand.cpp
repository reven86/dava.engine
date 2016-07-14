#include "InsertControlCommand.h"

#include "Document/CommandsBase/QECommandIDs.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/PackageNode.h"

using namespace DAVA;

InsertControlCommand::InsertControlCommand(PackageNode* _root, ControlNode* _node, ControlsContainerNode* _dest, int _index)
    : CommandWithoutExecute(CMDID_INSERT_CONTROL, "InsertControl")
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

void InsertControlCommand::Redo()
{
    root->InsertControl(node, dest, index);
}

void InsertControlCommand::Undo()
{
    root->RemoveControl(node, dest);
}
