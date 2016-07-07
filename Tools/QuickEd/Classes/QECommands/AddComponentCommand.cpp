#include "AddComponentCommand.h"

#include "Document/CommandsBase/QECommandIDs.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

AddComponentCommand::AddComponentCommand(PackageNode* _root, ControlNode* _node, ComponentPropertiesSection* _section)
    : QECommand(CMDID_ADD_COMPONENT, "Add component")
    , root(SafeRetain(_root))
    , node(SafeRetain(_node))
    , section(SafeRetain(_section))
{
}

AddComponentCommand::~AddComponentCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(section);
}

void AddComponentCommand::Redo()
{
    root->AddComponent(node, section);
}

void AddComponentCommand::Undo()
{
    root->RemoveComponent(node, section);
}
