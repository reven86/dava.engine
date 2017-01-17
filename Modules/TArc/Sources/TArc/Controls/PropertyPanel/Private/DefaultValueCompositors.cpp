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

    if (value.GetType() == Type::Instance<DAVA::FastName>())
    {
        return String(value.Get<DAVA::FastName>().c_str());
    }

    return value.Get<String>();
}

bool TextValueCompositor::IsValidValue(const Any& value) const
{
    String text;
    if (value.CanGet<FastName>())
    {
        text = value.Get<FastName>().c_str();
    }

    if (value.CanGet<String>())
    {
        text = value.Get<String>();
    }
    return text != multipleValuesValue;
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

    return value.Get<bool>() ? Qt::Checked : Qt::Unchecked;
}

bool BoolValueCompositor::IsValidValue(const Any& value) const
{
    Qt::CheckState checkedState = Qt::PartiallyChecked;
    if (value.CanGet<Qt::CheckState>())
    {
        checkedState = value.Get<Qt::CheckState>();
    }

    return checkedState != Qt::PartiallyChecked;
}

} // namespace TArc
} // namespace DAVA
