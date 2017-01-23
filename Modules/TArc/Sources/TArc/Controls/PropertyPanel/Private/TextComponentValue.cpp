#include "TextComponentValue.h"
#include "TArc/Controls/LineEdit.h"
#include "TArc/Controls/PropertyPanel/DefaultEditorDrawers.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Base/FastName.h"

namespace DAVA
{
namespace TArc
{
String TextComponentValue::GetText() const
{
    return GetValue().Cast<String>();
}

void TextComponentValue::SetText(const DAVA::String& text)
{
    SetValue(text);
}

QWidget* TextComponentValue::AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option)
{
    ControlDescriptorBuilder<LineEdit::Fields> descr;
    descr[LineEdit::Text] = "text";
    return (new LineEdit(descr, GetWrappersProcessor(), GetReflection(), parent))->ToWidgetCast();
}

void TextComponentValue::ReleaseEditorWidget(QWidget* editor)
{
    editor->deleteLater();
}

bool TextComponentValue::IsReadOnly() const
{
    return nodes.front()->field.ref.IsReadonly();
}

bool TextComponentValue::IsEnabled() const
{
    return true;
}

DAVA_REFLECTION_IMPL(TextComponentValue)
{
    ReflectionRegistrator<TextComponentValue>::Begin()
    .Field("text", &TextComponentValue::GetText, &TextComponentValue::SetText)
    .Field("readOnly", &TextComponentValue::IsReadOnly, nullptr)
    .Field("enabled", &TextComponentValue::IsEnabled, nullptr)
    .End();
}
}
}
