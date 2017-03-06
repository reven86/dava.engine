#include "QECommands/RemoveComponentCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

RemoveComponentCommand::RemoveComponentCommand(PackageNode* package, ControlNode* node_, ComponentPropertiesSection* section_)
    : QEPackageCommand(package, REMOVE_COMPONENT_COMMAND, "RemoveComponent")
    , node(SafeRetain(node_))
    , componentSection(SafeRetain(section_))
{
}

RemoveComponentCommand::~RemoveComponentCommand()
{
    SafeRelease(node);
    SafeRelease(componentSection);
}

void RemoveComponentCommand::Redo()
{
    package->RemoveComponent(node, componentSection);
}

void RemoveComponentCommand::Undo()
{
    package->AddComponent(node, componentSection);
}
