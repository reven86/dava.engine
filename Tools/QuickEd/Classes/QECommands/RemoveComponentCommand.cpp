#include "RemoveComponentCommand.h"

#include "Document/CommandsBase/QECommandIDs.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

RemoveComponentCommand::RemoveComponentCommand(PackageNode* _root, ControlNode* _node, ComponentPropertiesSection* _section)
    : CommandWithoutExecute(CMDID_REMOVE_COMPONENT, "RemoveComponent")
    , root(SafeRetain(_root))
    , node(SafeRetain(_node))
    , componentSection(SafeRetain(_section))
{
}

RemoveComponentCommand::~RemoveComponentCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(componentSection);
}

void RemoveComponentCommand::Redo()
{
    root->RemoveComponent(node, componentSection);
}

void RemoveComponentCommand::Undo()
{
    root->AddComponent(node, componentSection);
}
