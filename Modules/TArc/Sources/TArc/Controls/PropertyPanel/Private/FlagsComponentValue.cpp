#include "TArc/Controls/PropertyPanel/Private/FlagsComponentValue.h"
#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"
#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"
#include "TArc/Controls/ComboBoxCheckable.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
namespace TArc
{
void FlagsComponentValue::SetValueAny(const Any& newValue)
{
    SetValue(newValue);
}

DAVA::Any FlagsComponentValue::GetMultipleValue() const
{
    return Any();
}

bool FlagsComponentValue::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    if (newValue.IsEmpty() || currentValue.IsEmpty())
    {
        return false;
    }

    int newIntValue = newValue.Cast<int>();
    int currentIntValue = currentValue.Cast<int>();

    return newIntValue != currentIntValue;
}

ControlProxy* FlagsComponentValue::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor)
{
    ComboBoxCheckable::Params params(GetAccessor(), GetUI(), GetWindowKey());
    params.fields[ComboBoxCheckable::Fields::Value] = "value";
    params.fields[ComboBoxCheckable::Fields::IsReadOnly] = readOnlyFieldName;
    return new ComboBoxCheckable(params, wrappersProcessor, model, parent);
}

Any FlagsComponentValue::GetValueAny() const
{
    return GetValue();
}

DAVA_VIRTUAL_REFLECTION_IMPL(FlagsComponentValue)
{
    ReflectionRegistrator<FlagsComponentValue>::Begin(CreateComponentStructureWrapper<FlagsComponentValue>())
    .Field("value", &FlagsComponentValue::GetValueAny, &FlagsComponentValue::SetValueAny)[M::ProxyMetaRequire()]
    .End();
}
} //TArc
} //DAVA
