#include "AttachComponentPrototypeSectionCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

AttachComponentPrototypeSectionCommand::AttachComponentPrototypeSectionCommand(PackageNode* aRoot, ControlNode* aNode, ComponentPropertiesSection* aDestSection, ComponentPropertiesSection* aPrototypeSection)
    : Command("AttachComponentPrototypeSection")
    , root(SafeRetain(aRoot))
    , node(SafeRetain(aNode))
    , destSection(SafeRetain(aDestSection))
    , prototypeSection(SafeRetain(aPrototypeSection))
{
}

AttachComponentPrototypeSectionCommand::~AttachComponentPrototypeSectionCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(destSection);
    SafeRelease(prototypeSection);
}

void AttachComponentPrototypeSectionCommand::Redo()
{
    root->AttachPrototypeComponent(node, destSection, prototypeSection);
}

void AttachComponentPrototypeSectionCommand::Undo()
{
    root->DetachPrototypeComponent(node, destSection, prototypeSection);
}
