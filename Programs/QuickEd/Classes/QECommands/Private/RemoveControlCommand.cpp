#include "QECommands/RemoveControlCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

using namespace DAVA;

RemoveControlCommand::RemoveControlCommand(PackageNode* package, ControlNode* node_, ControlsContainerNode* from_, int index_)
    : QEPackageCommand(package, "Remove Control")
    , node(RefPtr<ControlNode>::ConstructWithRetain(node_))
    , from(RefPtr<ControlsContainerNode>::ConstructWithRetain(from_))
    , index(index_)
{
}

void RemoveControlCommand::Redo()
{
    if (dynamic_cast<ControlsContainerNode*>(node->GetParent()) != nullptr)
    {
        nodeGuides = package->GetAllGuides(node->GetName());
    }
    package->RemoveControl(node.Get(), from.Get());
}

void RemoveControlCommand::Undo()
{
    package->InsertControl(node.Get(), from.Get(), index);
    if (nodeGuides.verticalGuides.empty() == false && nodeGuides.horizontalGuides.empty() == false)
    {
        package->SetAllGuides(node->GetName(), nodeGuides);
    }
}
