#include "RemoveComponentCommand.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "UI/Properties/PropertiesModel.h"
#include "UI/PropertiesContext.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

RemoveComponentCommand::RemoveComponentCommand(PropertiesContext *_context, ControlNode *_node, int _componentType, int _componentIndex, QUndoCommand *parent)
    : QUndoCommand(parent)
    , context(_context)
    , node(SafeRetain(_node))
    , componentType(_componentType)
    , componentIndex(_componentIndex)
    , componentSection(nullptr)
{
}

RemoveComponentCommand::~RemoveComponentCommand()
{
    SafeRelease(node);
    SafeRelease(componentSection);
}

void RemoveComponentCommand::redo()
{
    PropertiesModel *model = context->GetModel();
    if (model && model->GetControlNode() == node)
        model->RemoveComponentSection(componentSection);
    else
        node->GetPropertiesRoot()->RemoveComponentPropertiesSection(componentSection);
}

void RemoveComponentCommand::undo()
{
    PropertiesModel *model = context->GetModel();
    if (model && model->GetControlNode() == node) // if model selected
        model->AddComponentSection(componentSection);
    else
        node->GetPropertiesRoot()->AddComponentPropertiesSection(componentSection);
}
