#include "QECommands/AttachComponentPrototypeSectionCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include <UI/Components/UIComponent.h>

using namespace DAVA;

AttachComponentPrototypeSectionCommand::AttachComponentPrototypeSectionCommand(PackageNode* package, ControlNode* node_, ComponentPropertiesSection* destSection_, ComponentPropertiesSection* prototypeSection_)
    : QEPackageCommand(package, ATTACH_COMPONENT_PROTOTYPE_SECTION_COMMAND, "AttachComponentPrototypeSection")
    , node(SafeRetain(node_))
    , destSection(SafeRetain(destSection_))
    , prototypeSection(SafeRetain(prototypeSection_))
{
}

AttachComponentPrototypeSectionCommand::~AttachComponentPrototypeSectionCommand()
{
    SafeRelease(node);
    SafeRelease(destSection);
    SafeRelease(prototypeSection);
}

void AttachComponentPrototypeSectionCommand::Redo()
{
    package->AttachPrototypeComponent(node, destSection, prototypeSection);
}

void AttachComponentPrototypeSectionCommand::Undo()
{
    package->DetachPrototypeComponent(node, destSection, prototypeSection);
}
