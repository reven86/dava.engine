#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class ControlNode;
class ControlsContainerNode;

class RemoveControlCommand : public QEPackageCommand
{
public:
    RemoveControlCommand(PackageNode* package, ControlNode* node, ControlsContainerNode* from, int index);
    ~RemoveControlCommand() override;

    void Redo() override;
    void Undo() override;

private:
    ControlNode* node = nullptr;
    ControlsContainerNode* from = nullptr;
    const int index;
};
