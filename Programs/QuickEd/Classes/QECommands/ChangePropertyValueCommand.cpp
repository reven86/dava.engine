#include "ChangePropertyValueCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/AbstractProperty.h"

#include <QString>
using namespace DAVA;

ChangePropertyValueCommand::ChangePropertyValueCommand(ControlNode* node_, AbstractProperty* prop, const VariantType& newVal)
    : DAVA::Command(DAVA::String("changed property: ") + prop->GetName().c_str())
    , node(node_)
    , property(prop)
    , oldValue(GetValueFromProperty(prop))
    , newValue(newVal)
{
    root = node->GetPackage();
}

void ChangePropertyValueCommand::Redo()
{
    if (newValue.GetType() == VariantType::TYPE_NONE)
    {
        root->ResetControlProperty(node, property);
    }
    else
    {
        root->SetControlProperty(node, property, newValue);
    }
}

void ChangePropertyValueCommand::Undo()
{
    if (oldValue.GetType() == VariantType::TYPE_NONE)
    {
        root->ResetControlProperty(node, property);
    }
    else
    {
        root->SetControlProperty(node, property, oldValue);
    }
}

VariantType ChangePropertyValueCommand::GetValueFromProperty(AbstractProperty* property)
{
    return property->IsOverriddenLocally() ? property->GetValue() : VariantType();
}
