#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class ControlNode;
class ComponentPropertiesSection;

class AttachComponentPrototypeSectionCommand : public QEPackageCommand
{
public:
    AttachComponentPrototypeSectionCommand(PackageNode* package, ControlNode* node, ComponentPropertiesSection* destSection, ComponentPropertiesSection* prototypeSection);
    ~AttachComponentPrototypeSectionCommand() override;

    void Redo() override;
    void Undo() override;

private:
    ControlNode* node = nullptr;
    ComponentPropertiesSection* destSection = nullptr;
    ComponentPropertiesSection* prototypeSection = nullptr;
};
