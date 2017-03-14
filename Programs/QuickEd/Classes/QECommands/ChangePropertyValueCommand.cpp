#include "ChangePropertyValueCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/AbstractProperty.h"

#include <QString>
using namespace DAVA;

ChangePropertyValueCommand::ChangePropertyValueCommand(ControlNode* node_, AbstractProperty* property_, const Any& newVal_)
    : DAVA::Command(DAVA::String("changed property: ") + property_->GetName().c_str())
    , node(node_)
    , property(property_)
    , oldValue(GetValueFromProperty(property_))
    , newValue(newVal_)
{
    root = node->GetPackage();
}

void ChangePropertyValueCommand::Redo()
{
    if (newValue.IsEmpty())
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
    if (oldValue.IsEmpty())
    {
        root->ResetControlProperty(node, property);
    }
    else
    {
        root->SetControlProperty(node, property, oldValue);
    }
}

Any ChangePropertyValueCommand::GetValueFromProperty(AbstractProperty* property)
{
    return property->IsOverriddenLocally() ? property->GetValue() : Any();
}
