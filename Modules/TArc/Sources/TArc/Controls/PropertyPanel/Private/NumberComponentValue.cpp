#include "TArc/Controls/PropertyPanel/Private/NumberComponentValue.h"
#include "TArc/Controls/DoubleSpinBox.h"
#include "TArc/Controls/IntSpinBox.h"
#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"
#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
namespace TArc
{
template <typename T>
Any NumberComponentValue<T>::GetMultipleValue() const
{
    return Any();
}

template <typename T>
bool NumberComponentValue<T>::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    if (newValue.CanCast<T>() == false)
    {
        return false;
    }

    if (currentValue.CanCast<T>() == false)
    {
        return true;
    }

    return newValue.Cast<T>() != currentValue.Cast<T>();
}

template <typename T>
ControlProxy* NumberComponentValue<T>::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor)
{
    const Type* t = Type::Instance<T>();
    if (t == Type::Instance<float32>() || t == Type::Instance<float64>())
    {
        DoubleSpinBox::Params params(GetAccessor(), GetUI(), GetWindowKey());
        params.fields[DoubleSpinBox::Fields::Value] = "value";
        params.fields[DoubleSpinBox::Fields::IsReadOnly] = readOnlyFieldName;
        return new DoubleSpinBox(params, wrappersProcessor, model, parent);
    }
    else
    {
        IntSpinBox::Params params(GetAccessor(), GetUI(), GetWindowKey());
        params.fields[IntSpinBox::Fields::Value] = "value";
        params.fields[IntSpinBox::Fields::IsReadOnly] = readOnlyFieldName;
        return new IntSpinBox(params, wrappersProcessor, model, parent);
    }
}

template <typename T>
Any NumberComponentValue<T>::GetNumberValue() const
{
    Any v = GetValue();
    if (v.CanCast<T>())
    {
        return v.Cast<T>();
    }
    return v;
}

template <typename T>
void NumberComponentValue<T>::SetNumberValue(const Any& v)
{
    SetValue(v.Cast<T>());
}

DAVA_VIRTUAL_TEMPLATE_REFLECTION_IMPL(NumberComponentValue)
{
    ReflectionRegistrator<NumberComponentValue<T>>::Begin(CreateComponentStructureWrapper<NumberComponentValue<T>>())
    .Field("value", &NumberComponentValue<T>::GetNumberValue, &NumberComponentValue<T>::SetNumberValue)[M::ProxyMetaRequire()]
    .End();
}

#if __clang__
_Pragma("clang diagnostic push")
_Pragma("clang diagnostic ignored \"-Wweak-template-vtables\"")
#endif

template class NumberComponentValue<float64>;
template class NumberComponentValue<float32>;
template class NumberComponentValue<int8>;
template class NumberComponentValue<uint8>;
template class NumberComponentValue<int16>;
template class NumberComponentValue<uint16>;
template class NumberComponentValue<int32>;
template class NumberComponentValue<uint32>;

#if __clang__
_Pragma("clang diagnostic pop")
#endif

} // namespace TArc
} // namespace DAVA
