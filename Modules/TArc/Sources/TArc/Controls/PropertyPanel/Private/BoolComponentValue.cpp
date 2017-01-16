#pragma once

#include "TArc/Controls/PropertyPanel/Private/BoolComponentValue.h"
#include "Reflection/ReflectionRegistrator.h"

#include <QUrl>

namespace DAVA
{
namespace TArc
{
//int BoolComponentValue::GetValue() const
//{
//    Any value = nodes.front()->cachedValue;
//    for (const std::shared_ptr<const PropertyNode>& node : nodes)
//    {
//        if (value != node->cachedValue)
//        {
//            return Qt::PartiallyChecked;
//        }
//    }
//
//    return value.Cast<bool>() == true ? Qt::Checked : Qt::Unchecked;
//}
//
//void BoolComponentValue::SetValue(int v)
//{
//    if (v == Qt::PartiallyChecked)
//    {
//        return;
//    }
//
//    GetModifyInterface()->ModifyPropertyValue(nodes, Any(v == Qt::Checked));
//}
//
//bool BoolComponentValue::IsReadOnly() const
//{
//    return nodes.front()->field.ref.IsReadonly();
//}
//
//DAVA_REFLECTION_IMPL(BoolComponentValue)
//{
//    ReflectionRegistrator<BoolComponentValue>::Begin()
//    .Field("value", &BoolComponentValue::GetValue, &BoolComponentValue::SetValue)
//    .Field("readOnly", &BoolComponentValue::IsReadOnly, nullptr)
//    .End();
//}
}
}