#include "IntComponentValue.h"
#include "TArc/Controls/IntSpinBox.h"
#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"
#include "TArc/Controls/PropertyPanel/Private/PropertyPanelMeta.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
namespace TArc
{
int32 IntComponentValue::GetValue() const
{
    return BaseComponentValue::GetValue().Cast<int32>();
}

void IntComponentValue::SetValue(int32 v)
{
    BaseComponentValue::SetValue(v);
}

DAVA::Any IntComponentValue::GetMultipleValue() const
{
    return Any();
}

bool IntComponentValue::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    if (newValue.CanCast<int32>() == false)
    {
        return false;
    }

    if (currentValue.CanCast<int32>() == false)
    {
        return true;
    }

    return newValue.Cast<int32>() != currentValue.Cast<int32>();
}

ControlProxy* IntComponentValue::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const
{
    ControlDescriptorBuilder<IntSpinBox::Fields> descr;
    descr[IntSpinBox::Fields::Value] = "value";
    descr[IntSpinBox::Fields::IsReadOnly] = readOnlyFieldName;
    return new IntSpinBox(descr, wrappersProcessor, model, parent);
}

DAVA_VIRTUAL_REFLECTION_IMPL(IntComponentValue)
{
    ReflectionRegistrator<IntComponentValue>::Begin(CreateComponentStructureWrapper<IntComponentValue>())
    .Field("value", &IntComponentValue::GetValue, &IntComponentValue::SetValue)[M::ProxyMetaRequire()]
    .End();
}
}
}