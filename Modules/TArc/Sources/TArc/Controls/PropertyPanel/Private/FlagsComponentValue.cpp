#include "TArc/Controls/PropertyPanel/Private/FlagsComponentValue.h"
#include "TArc/Controls/PropertyPanel/DefaultEditorDrawers.h"
#include "TArc/Controls/PropertyPanel/Private/PropertyPanelMeta.h"
#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"
#include "TArc/Controls/ComboBoxCheckable.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Base/FastName.h>

namespace DAVA
{
namespace TArc
{
void FlagsComponentValue::SetValueAny(const Any& newValue)
{
    SetValue(newValue);
}

Any FlagsComponentValue::GetValueAny() const
{
    return GetValue();
}

QWidget* FlagsComponentValue::AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option)
{
    ControlDescriptorBuilder<ComboBoxCheckable::Fields> descr;
    descr[ComboBoxCheckable::Fields::Value] = "value";
    descr[ComboBoxCheckable::Fields::IsReadOnly] = "readOnly";
    return (new ComboBoxCheckable(descr, GetWrappersProcessor(), GetReflection(), parent))->ToWidgetCast();
}

bool FlagsComponentValue::IsReadOnly() const
{
    if (nodes.empty() == false)
    {
        return nodes.front()->field.ref.IsReadonly();
    }
    return true;
}

DAVA_VIRTUAL_REFLECTION_IMPL(FlagsComponentValue)
{
    ReflectionRegistrator<FlagsComponentValue>::Begin(CreateComponentStructureWrapper<FlagsComponentValue>())
    .Field("value", &FlagsComponentValue::GetValueAny, &FlagsComponentValue::SetValueAny)[M::ProxyMetaRequire()]
    .Field("readOnly", &FlagsComponentValue::IsReadOnly, nullptr)
    .End();
}
} //TArc
} //DAVA
