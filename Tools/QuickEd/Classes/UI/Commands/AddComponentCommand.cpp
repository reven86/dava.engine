#include "AddComponentCommand.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "UI/Properties/PropertiesModel.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

AddComponentCommand::AddComponentCommand(PropertiesModel *_model, ControlNode *_node, int _componentType, QUndoCommand *parent)
    : QUndoCommand(parent)
    , model(_model)
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
    if (true) // if model selected
        model->AddComponentSection(node, componentSection);
    else
        node->GetPropertiesRoot()->AddComponentPropertiesSection(componentSection);
}

void AddComponentCommand::undo()
{
    if (true)
        model->RemoveComponentSection(node, componentSection);
    else
        node->GetPropertiesRoot()->RemoveComponentPropertiesSection(componentSection);

}
