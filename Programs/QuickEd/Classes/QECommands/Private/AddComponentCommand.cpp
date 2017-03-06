#include "QECommands/AddComponentCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include <UI/Components/UIComponent.h>

using namespace DAVA;

AddComponentCommand::AddComponentCommand(PackageNode* package, ControlNode* node_, ComponentPropertiesSection* section_)
    : QEPackageCommand(package, ADD_COMPONENT_COMMAND, "Add component")
    , node(SafeRetain(node_))
    , section(SafeRetain(section_))
{
}

AddComponentCommand::~AddComponentCommand()
{
    SafeRelease(node);
    SafeRelease(section);
}

void AddComponentCommand::Redo()
{
    package->AddComponent(node, section);
}

void AddComponentCommand::Undo()
{
    package->RemoveComponent(node, section);
}
