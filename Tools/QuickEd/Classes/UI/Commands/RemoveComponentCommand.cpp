#include "RemoveComponentCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

RemoveComponentCommand::RemoveComponentCommand(PackageNode *_root, ControlNode *_node, int _componentType, QUndoCommand *parent)
    : QUndoCommand(parent)
    , root(SafeRetain(_root))
    , node(SafeRetain(_node))
    , componentSection(nullptr) // comment
{
    componentSection = SafeRetain(node->GetRootProperty()->FindComponentPropertiesSection(_componentType));
}

RemoveComponentCommand::~RemoveComponentCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(componentSection);
}

void RemoveComponentCommand::redo()
{
    root->RemoveComponent(node, componentSection);
}

void RemoveComponentCommand::undo()
{
    root->AddComponent(node, componentSection);
}
