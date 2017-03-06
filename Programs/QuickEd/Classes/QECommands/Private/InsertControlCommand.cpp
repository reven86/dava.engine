#include "QECommands/InsertControlCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/PackageNode.h"

using namespace DAVA;

InsertControlCommand::InsertControlCommand(PackageNode* package, ControlNode* node_, ControlsContainerNode* dest_, int index_)
    : QEPackageCommand(package, INSERT_CONTROL_COMMAND, "InsertControl")
    , node(SafeRetain(node_))
    , dest(SafeRetain(dest_))
    , index(index_)
{
}

InsertControlCommand::~InsertControlCommand()
{
    SafeRelease(node);
    SafeRelease(dest);
}

void InsertControlCommand::Redo()
{
    package->InsertControl(node, dest, index);
}

void InsertControlCommand::Undo()
{
    package->RemoveControl(node, dest);
}
