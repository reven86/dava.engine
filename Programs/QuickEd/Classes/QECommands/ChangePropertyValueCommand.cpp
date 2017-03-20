#include "ChangePropertyValueCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/AbstractProperty.h"

#include <UI/Layouts/UILayoutSourceRectComponent.h>
#include <QString>

using namespace DAVA;

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode* root_, ControlNode* node_, AbstractProperty* property_, const VariantType& newValue_)
    : ChangePropertyValueCommand(root_, node_, property_, newValue_, GetValueFromProperty(property_))
{
}

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode* root_, ControlNode* node_, AbstractProperty* property_, const DAVA::VariantType& newValue_, const DAVA::VariantType& oldValue_)
    : DAVA::Command(DAVA::String("changed property: ") + property_->GetName().c_str())
    , root(root_)
    , node(node_)
    , property(property_)
    , oldValue(oldValue_)
    , newValue(newValue_)
{
    
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
