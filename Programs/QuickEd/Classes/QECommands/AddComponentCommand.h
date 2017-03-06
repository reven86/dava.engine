#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class ControlNode;
class ComponentPropertiesSection;

class AddComponentCommand : public QEPackageCommand
{
public:
    AddComponentCommand(PackageNode* package, ControlNode* node, ComponentPropertiesSection* section);

    ~AddComponentCommand() override;

    void Redo() override;
    void Undo() override;

private:
    ControlNode* node = nullptr;
    ComponentPropertiesSection* section = nullptr;
};
