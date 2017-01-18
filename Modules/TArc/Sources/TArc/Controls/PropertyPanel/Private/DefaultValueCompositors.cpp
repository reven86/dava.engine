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

bool EmptyValueCompositor::IsValidValue(const Any& value) const
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

bool TextValueCompositor::IsValidValue(const Any& value) const
{
    return value.Cast<String>() != multipleValuesValue;
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

bool BoolValueCompositor::IsValidValue(const Any& value) const
{
    Qt::CheckState checkedState = value.Cast<Qt::CheckState>(Qt::PartiallyChecked);
    return checkedState != Qt::PartiallyChecked;
}

} // namespace TArc
} // namespace DAVA
