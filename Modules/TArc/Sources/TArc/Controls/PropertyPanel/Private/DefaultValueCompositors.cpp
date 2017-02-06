#include "TArc/Controls/PropertyPanel/DefaultValueCompositors.h"

#include <Base/FastName.h>
#include <Base/Type.h>
#include <Base/BaseTypes.h>

#include <Qt>

namespace DAVA
{
namespace TArc
{
const String multipleValuesValue("<multiple values>");

Any EmptyValueCompositor::Compose(const Vector<std::shared_ptr<PropertyNode>>& nodes) const
{
    return Any();
}

bool EmptyValueCompositor::IsValidValue(const Any& newValue, const Any& currentValue) const
{
    return false;
}

Any TextValueCompositor::Compose(const Vector<std::shared_ptr<PropertyNode>>& nodes) const
{
    Any value = nodes.front()->cachedValue;
    for (const std::shared_ptr<const PropertyNode>& node : nodes)
    {
        if (value != node->cachedValue)
        {
            return multipleValuesValue;
        }
    }

    return value.Cast<String>();
}

bool TextValueCompositor::IsValidValue(const Any& newValue, const Any& currentValue) const
{
    String newStrignValue = newValue.Cast<String>();
    String currentStringValue = currentValue.Cast<String>();
    return newStrignValue != multipleValuesValue && newStrignValue != currentStringValue;
}

Any BoolValueCompositor::Compose(const Vector<std::shared_ptr<PropertyNode>>& nodes) const
{
    Any value = nodes.front()->cachedValue;
    for (const std::shared_ptr<const PropertyNode>& node : nodes)
    {
        if (value != node->cachedValue)
        {
            return Qt::PartiallyChecked;
        }
    }

    return value.Cast<bool>() ? Qt::Checked : Qt::Unchecked;
}

bool BoolValueCompositor::IsValidValue(const Any& newValue, const Any& currentValue) const
{
    Qt::CheckState newCheckedState = newValue.Cast<Qt::CheckState>(Qt::PartiallyChecked);
    Qt::CheckState currentCheckedState = currentValue.Cast<Qt::CheckState>(Qt::PartiallyChecked);
    return newCheckedState != Qt::PartiallyChecked && newCheckedState != currentCheckedState;
}

} // namespace TArc
} // namespace DAVA
