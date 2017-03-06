#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class ControlNode;
class ControlsContainerNode;

class InsertControlCommand : public QEPackageCommand
{
public:
    InsertControlCommand(PackageNode* package, ControlNode* node, ControlsContainerNode* dest, int index);
    ~InsertControlCommand() override;

    void Redo() override;
    void Undo() override;

private:
    ControlNode* node = nullptr;
    ControlsContainerNode* dest = nullptr;
    const int index;
};
