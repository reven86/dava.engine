#include "AddComponentCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

AddComponentCommand::AddComponentCommand(PackageNode *_root, ControlNode *_node, int _componentType, QUndoCommand *parent)
    : QUndoCommand(parent)
    , root(SafeRetain(_root))
    , node(SafeRetain(_node))
    , componentSection(nullptr)
{
    componentSection = new ComponentPropertiesSection(node->GetControl(), (UIComponent::eType) _componentType, nullptr, AbstractProperty::COPY_VALUES);
}

AddComponentCommand::~AddComponentCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(componentSection);
}

void AddComponentCommand::redo()
{
    root->AddComponent(node, componentSection);
}

void AddComponentCommand::undo()
{
    root->RemoveComponent(node, componentSection);
}
