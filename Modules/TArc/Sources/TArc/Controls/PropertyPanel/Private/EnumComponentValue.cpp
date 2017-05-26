#include "TArc/Controls/PropertyPanel/Private/EnumComponentValue.h"
#include "TArc/Controls/ComboBox.h"
#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"
#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
namespace TArc
{
void EnumComponentValue::SetValueAny(const Any& newValue)
{
    SetValue(newValue);
}

Any EnumComponentValue::GetValueAny() const
{
    return GetValue();
}

Any EnumComponentValue::GetMultipleValue() const
{
    return Any();
}

bool EnumComponentValue::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    if (newValue.IsEmpty())
    {
        return false;
    }

    if (currentValue.IsEmpty())
    {
        return true;
    }

    int newIntValue = newValue.Cast<int>();
    int currentIntValue = currentValue.Cast<int>();

    return newIntValue != currentIntValue;
}

ControlProxy* EnumComponentValue::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor)
{
    ComboBox::Params params(GetAccessor(), GetUI(), GetWindowKey());
    params.fields[ComboBox::Fields::Value] = "value";
    params.fields[ComboBox::Fields::IsReadOnly] = readOnlyFieldName;

    return new ComboBox(params, wrappersProcessor, model, parent);
}

DAVA_VIRTUAL_REFLECTION_IMPL(EnumComponentValue)
{
    ReflectionRegistrator<EnumComponentValue>::Begin(CreateComponentStructureWrapper<EnumComponentValue>())
    .Field("value", &EnumComponentValue::GetValueAny, &EnumComponentValue::SetValueAny)[M::ProxyMetaRequire()]
    .End();
}
} //TArc
} //DAVA
