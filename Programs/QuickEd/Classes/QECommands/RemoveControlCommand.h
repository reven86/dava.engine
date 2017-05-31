#pragma once

#include "QECommands/Private/QEPackageCommand.h"
#include "Model/PackageHierarchy/PackageNode.h"

class ControlNode;
class ControlsContainerNode;

class RemoveControlCommand : public QEPackageCommand
{
public:
    RemoveControlCommand(PackageNode* package, ControlNode* node, ControlsContainerNode* from, int index);

    void Redo() override;
    void Undo() override;

private:
    DAVA::RefPtr<ControlNode> node;
    PackageNode::Guides nodeGuides;

    DAVA::RefPtr<ControlsContainerNode> from;
    const int index;
};
