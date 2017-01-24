#include "TArc/Controls/PropertyPanel/Private/EnumComponentValue.h"
#include "TArc/Controls/ComboBox.h"
#include "TArc/Controls/PropertyPanel/DefaultEditorDrawers.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Base/FastName.h>

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

const M::Enum* EnumComponentValue::GetEnumerator() const
{
    if (nodes.empty() == false)
    {
        return nodes[0]->field.ref.GetMeta<M::Enum>();
    }
    return nullptr;
}

QWidget* EnumComponentValue::AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option)
{
    ControlDescriptorBuilder<ComboBox::Fields> descr;
    descr[ComboBox::Fields::Value] = "value";
    descr[ComboBox::Fields::Enumerator] = "enumerator";
    return (new ComboBox(descr, GetWrappersProcessor(), GetReflection(), parent))->ToWidgetCast();
}

void EnumComponentValue::ReleaseEditorWidget(QWidget* editor)
{
    editor->deleteLater();
}

bool EnumComponentValue::IsReadOnly() const
{
    return nodes.front()->field.ref.IsReadonly();
}

bool EnumComponentValue::IsEnabled() const
{
    return true;
}

DAVA_VIRTUAL_REFLECTION_IMPL(EnumComponentValue)
{
    ReflectionRegistrator<EnumComponentValue>::Begin()
    .Field("value", &EnumComponentValue::GetValueAny, &EnumComponentValue::SetValueAny)
    .Field("enumerator", &EnumComponentValue::GetEnumerator, nullptr)
    .Field("readOnly", &EnumComponentValue::IsReadOnly, nullptr)
    .Field("enabled", &EnumComponentValue::IsEnabled, nullptr)
    .End();
}
} //TArc
} //DAVA
