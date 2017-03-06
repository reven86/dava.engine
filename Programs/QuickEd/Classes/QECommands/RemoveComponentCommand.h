#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class ControlNode;
class ComponentPropertiesSection;

class RemoveComponentCommand : public QEPackageCommand
{
public:
    RemoveComponentCommand(PackageNode* package, ControlNode* node, ComponentPropertiesSection* section);
    ~RemoveComponentCommand() override;

    void Redo() override;
    void Undo() override;

private:
    ControlNode* node = nullptr;
    ComponentPropertiesSection* componentSection = nullptr;
};
