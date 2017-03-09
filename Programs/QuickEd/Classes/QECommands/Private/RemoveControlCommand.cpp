#include "QECommands/RemoveControlCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

using namespace DAVA;

RemoveControlCommand::RemoveControlCommand(PackageNode* package, ControlNode* node_, ControlsContainerNode* from_, int index_)
    : QEPackageCommand(package, REMOVE_CONTROL_COMMAND, "Remove Control")
    , node(RefPtr<ControlNode>::ConstructWithRetain(node_))
    , from(RefPtr<ControlsContainerNode>::ConstructWithRetain(from_))
    , index(index_)
{
}

void RemoveControlCommand::Redo()
{
    package->RemoveControl(node.Get(), from.Get());
}

void RemoveControlCommand::Undo()
{
    package->InsertControl(node.Get(), from.Get(), index);
}
