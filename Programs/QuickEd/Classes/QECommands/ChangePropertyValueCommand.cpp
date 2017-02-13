#include "ChangePropertyValueCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/AbstractProperty.h"

#include <QString>
using namespace DAVA;

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode* root_, ControlNode* node_, AbstractProperty* _property, const Any& _newVal)
    : DAVA::Command(DAVA::String("changed property: ") + _property->GetName().c_str())
    , root(root_)
    , node(node_)
    , property(_property)
    , oldValue(GetValueFromProperty(_property))
    , newValue(_newVal)
{
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
