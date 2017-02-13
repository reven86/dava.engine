#include "TArc/Controls/PropertyPanel/Private/EnumComponentValue.h"
#include "TArc/Controls/ComboBox.h"
#include "TArc/Controls/PropertyPanel/DefaultEditorDrawers.h"
#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"
#include "TArc/Controls/PropertyPanel/Private/PropertyPanelMeta.h"
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

QWidget* EnumComponentValue::AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option)
{
    ControlDescriptorBuilder<ComboBox::Fields> descr;
    descr[ComboBox::Fields::Value] = "value";
    descr[ComboBox::Fields::IsReadOnly] = "readOnly";
    return (new ComboBox(descr, GetWrappersProcessor(), GetReflection(), parent))->ToWidgetCast();
}

bool EnumComponentValue::IsReadOnly() const
{
    if (nodes.empty() == false)
    {
        return nodes.front()->field.ref.IsReadonly();
    }
    return true;
}

DAVA_VIRTUAL_REFLECTION_IMPL(EnumComponentValue)
{
    ReflectionRegistrator<EnumComponentValue>::Begin(CreateComponentStructureWrapper<EnumComponentValue>())
    .Field("value", &EnumComponentValue::GetValueAny, &EnumComponentValue::SetValueAny)[M::ProxyMetaRequire()]
    .Field("readOnly", &EnumComponentValue::IsReadOnly, nullptr)
    .End();
}
} //TArc
} //DAVA
