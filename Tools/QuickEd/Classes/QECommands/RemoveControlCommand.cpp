#include "RemoveControlCommand.h"

#include "Document/CommandsBase/QECommandIDs.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

using namespace DAVA;

RemoveControlCommand::RemoveControlCommand(PackageNode* _root, ControlNode* _node, ControlsContainerNode* _from, int _index)
    : CommandWithoutExecute(CMDID_REMOVE_CONTROL, "RemoveControl")
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

void RemoveControlCommand::Redo()
{
    root->RemoveControl(node, from);
}

void RemoveControlCommand::Undo()
{
    root->InsertControl(node, from, index);
}
