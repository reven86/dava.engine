#include "IntComponentValue.h"

#include "SimpleComponentLoader.h"
#include "Reflection/ReflectionRegistrator.h"

#include <QUrl>

namespace DAVA
{
namespace TArc
{
double IntComponentValue::GetValue() const
{
    Any value = nodes.front()->cachedValue;
    for (const std::shared_ptr<const PropertyNode>& node : nodes)
    {
        if (value != node->cachedValue)
        {
            return 0;
        }
    }

    if (value.CanCast<int32>())
        return value.Cast<int32>();

    return value.Cast<uint32>();
}

void IntComponentValue::SetValue(double v)
{
    Any value = nodes.front()->cachedValue;
    if (value.GetType() == Type::Instance<int32>())
    {
        value = Any(static_cast<int32>(v));
    }
    else if (value.GetType() == Type::Instance<uint32>())
    {
        value = Any(static_cast<uint32>(v));
    }

    GetModifyInterface()->ModifyPropertyValue(nodes, value);
}

int IntComponentValue::GetMinValue() const
{
    return std::numeric_limits<int>::min();
}

int IntComponentValue::GetMaxValue() const
{
    return std::numeric_limits<int>::max();
}

bool IntComponentValue::IsReadOnly() const
{
    return nodes.front()->field.ref.IsReadonly();
}

DAVA_REFLECTION_IMPL(IntComponentValue)
{
    ReflectionRegistrator<IntComponentValue>::Begin()
    .Field("value", &IntComponentValue::GetValue, &IntComponentValue::SetValue)
    .Field("minValue", &IntComponentValue::GetMinValue, nullptr)
    .Field("maxValue", &IntComponentValue::GetMaxValue, nullptr)
    .Field("readOnly", &IntComponentValue::IsReadOnly, nullptr)
    .End();
}
}
}