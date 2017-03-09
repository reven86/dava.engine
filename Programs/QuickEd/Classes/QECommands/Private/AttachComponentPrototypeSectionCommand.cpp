#include "QECommands/AttachComponentPrototypeSectionCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include <UI/Components/UIComponent.h>

using namespace DAVA;

AttachComponentPrototypeSectionCommand::AttachComponentPrototypeSectionCommand(PackageNode* package, ControlNode* node_, ComponentPropertiesSection* destSection_, ComponentPropertiesSection* prototypeSection_)
    : QEPackageCommand(package, ATTACH_COMPONENT_PROTOTYPE_SECTION_COMMAND, "Attach Component Prototype Section")
    , node(RefPtr<ControlNode>::ConstructWithRetain(node_))
    , destSection(RefPtr<ComponentPropertiesSection>::ConstructWithRetain(destSection_))
    , prototypeSection(RefPtr<ComponentPropertiesSection>::ConstructWithRetain(prototypeSection_))
{
}


void AttachComponentPrototypeSectionCommand::Redo()
{
    package->AttachPrototypeComponent(node.Get(), destSection.Get(), prototypeSection.Get());
}

void AttachComponentPrototypeSectionCommand::Undo()
{
    package->DetachPrototypeComponent(node.Get(), destSection.Get(), prototypeSection.Get());
}
