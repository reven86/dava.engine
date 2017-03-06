#include "QECommands/ChangePivotCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/AbstractProperty.h"

#include <Utils/StringFormat.h>

namespace ChangePivotCommandDetails
{
DAVA::VariantType GetValueFromProperty(AbstractProperty* property)
{
    return property->IsOverriddenLocally() ? property->GetValue() : DAVA::VariantType();
}
}

ChangePivotCommand::ChangePivotCommand(PackageNode* package)
    : QEPackageCommand(package, CHANGE_PIVOT_COMMAND, DAVA::Format("Change pivot"))
{
}

void ChangePivotCommand::AddNodePropertyValue(ControlNode* node, AbstractProperty* pivotProperty, const DAVA::VariantType& pivotValue, AbstractProperty* positionProperty, const DAVA::VariantType& positionValue)
{
    DVASSERT(node != nullptr);
    DVASSERT(pivotProperty != nullptr);
    DVASSERT(positionProperty != nullptr);
    items.emplace_back(node, pivotProperty, pivotValue, positionProperty, positionValue);
}

void ChangePivotCommand::Redo()
{
    for (const Item& item : items)
    {
        package->SetControlProperty(item.node, item.pivotProperty, item.pivotNewValue);
        package->SetControlProperty(item.node, item.positionProperty, item.positionNewValue);
    }
}

void ChangePivotCommand::Undo()
{
    for (const Item& item : items)
    {
        if (item.pivotOldValue.GetType() == DAVA::VariantType::TYPE_NONE)
        {
            package->ResetControlProperty(item.node, item.pivotProperty);
        }
        else
        {
            package->SetControlProperty(item.node, item.pivotProperty, item.pivotOldValue);
        }

        if (item.pivotOldValue.GetType() == DAVA::VariantType::TYPE_NONE)
        {
            package->ResetControlProperty(item.node, item.positionProperty);
        }
        else
        {
            package->SetControlProperty(item.node, item.positionProperty, item.positionOldValue);
        }
    }
}

bool ChangePivotCommand::MergeWith(const DAVA::Command* command)
{
    DVASSERT(GetID() == command->GetID());
    const ChangePivotCommand* other = static_cast<const ChangePivotCommand*>(command);
    DVASSERT(other != nullptr);
    if (package != other->package)
    {
        return false;
    }
    const size_t itemsSize = items.size();
    if (itemsSize != other->items.size())
    {
        return false;
    }
    for (size_t i = 0; i < itemsSize; ++i)
    {
        const Item& item = items.at(i);
        const Item& otherItem = other->items.at(i);
        if (item.node != otherItem.node ||
            item.pivotProperty != otherItem.pivotProperty ||
            item.positionProperty != otherItem.positionProperty)
        {
            return false;
        }
    }
    for (size_t i = 0; i < itemsSize; ++i)
    {
        items.at(i).pivotNewValue = other->items.at(i).pivotNewValue;
        items.at(i).positionNewValue = other->items.at(i).positionNewValue;
    }
    return true;
}

ChangePivotCommand::Item::Item(ControlNode* node_, AbstractProperty* pivotProperty_, const DAVA::VariantType& pivotValue, AbstractProperty* positionProperty_, const DAVA::VariantType& positionValue)
    : node(node_)
    , pivotProperty(pivotProperty_)
    , pivotNewValue(pivotValue)
    , pivotOldValue(ChangePivotCommandDetails::GetValueFromProperty(pivotProperty))
    , positionProperty(positionProperty_)
    , positionNewValue(pivotValue)
    , positionOldValue(ChangePivotCommandDetails::GetValueFromProperty(positionProperty))
{
}
