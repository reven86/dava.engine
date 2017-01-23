#include "TArc/Controls/PropertyPanel/Private/BoolComponentValue.h"
#include "TArc/Controls/CheckBox.h"
#include "TArc/Controls/PropertyPanel/DefaultEditorDrawers.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Base/FastName.h>

namespace DAVA
{
namespace TArc
{
Qt::CheckState BoolComponentValue::GetCheckState() const
{
    return GetValue().Cast<Qt::CheckState>();
}

void BoolComponentValue::SetCheckState(Qt::CheckState checkState)
{
    SetValue(checkState);
}

QWidget* BoolComponentValue::AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option)
{
    ControlDescriptorBuilder<CheckBox::Fields> descr;
    descr[CheckBox::Checked] = "bool";
    return (new CheckBox(descr, GetWrappersProcessor(), GetReflection(), parent))->ToWidgetCast();
}

void BoolComponentValue::ReleaseEditorWidget(QWidget* editor)
{
    editor->deleteLater();
}

bool BoolComponentValue::IsReadOnly() const
{
    return nodes.front()->field.ref.IsReadonly();
}

bool BoolComponentValue::IsEnabled() const
{
    return true;
}

DAVA_REFLECTION_IMPL(BoolComponentValue)
{
    ReflectionRegistrator<BoolComponentValue>::Begin()
    .Field("bool", &BoolComponentValue::GetCheckState, &BoolComponentValue::SetCheckState)
    .Field("readOnly", &BoolComponentValue::IsReadOnly, nullptr)
    .Field("enabled", &BoolComponentValue::IsEnabled, nullptr)
    .End();
}
}
}
