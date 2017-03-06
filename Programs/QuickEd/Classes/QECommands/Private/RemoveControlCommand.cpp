#include "QECommands/RemoveControlCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

using namespace DAVA;

RemoveControlCommand::RemoveControlCommand(PackageNode* package, ControlNode* node_, ControlsContainerNode* from_, int index_)
    : QEPackageCommand(package, REMOVE_CONTROL_COMMAND, "RemoveControl")
    , node(SafeRetain(node_))
    , from(SafeRetain(from_))
    , index(index_)
{
}

RemoveControlCommand::~RemoveControlCommand()
{
    SafeRelease(node);
    SafeRelease(from);
}

void RemoveControlCommand::Redo()
{
    package->RemoveControl(node, from);
}

void RemoveControlCommand::Undo()
{
    package->InsertControl(node, from, index);
}
