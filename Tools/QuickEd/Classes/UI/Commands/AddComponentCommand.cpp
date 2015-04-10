#include "AddComponentCommand.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "UI/Properties/PropertiesModel.h"
#include "UI/PropertiesContext.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

AddComponentCommand::AddComponentCommand(PropertiesContext *_context, ControlNode *_node, int _componentType, QUndoCommand *parent)
    : QUndoCommand(parent)
    , context(_context)
    , node(SafeRetain(_node))
    , componentSection(nullptr)
{
    componentSection = new ComponentPropertiesSection(node->GetControl(), (UIComponent::eType) _componentType, nullptr, BaseProperty::COPY_VALUES);
}

AddComponentCommand::~AddComponentCommand()
{
    SafeRelease(node);
    SafeRelease(componentSection);
}

void AddComponentCommand::redo()
{
    PropertiesModel *model = context->GetModel();
    if (model && model->GetControlNode() == node) // if model selected
        model->AddComponentSection(componentSection);
    else
        node->GetPropertiesRoot()->AddComponentPropertiesSection(componentSection);
}

void AddComponentCommand::undo()
{
    PropertiesModel *model = context->GetModel();
    if (model && model->GetControlNode() == node)
        model->RemoveComponentSection(componentSection);
    else
        node->GetPropertiesRoot()->RemoveComponentPropertiesSection(componentSection);
}
