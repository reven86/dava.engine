#include "TextComponentValue.h"
#include "TArc/Controls/LineEdit.h"
#include "TArc/Controls/PropertyPanel/TextEditorDrawer.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Base/FastName.h"

namespace DAVA
{
namespace TArc
{
const String multipleValuesValue("<multiple values>");

QString TextComponentValue::GetObjectName() const
{
    return QString::fromStdString(nodes.front()->field.key.Cast<String>());
}

DAVA::String TextComponentValue::GetText() const
{
    Any value = nodes.front()->cachedValue;
    for (const std::shared_ptr<const PropertyNode>& node : nodes)
    {
        if (value != node->cachedValue)
        {
            return multipleValuesValue;
        }
    }

    if (value.GetType() == Type::Instance<DAVA::FastName>())
    {
        return DAVA::String(value.Cast<DAVA::FastName>().c_str());
    }

    return value.Cast<String>();
}

void TextComponentValue::SetText(const DAVA::String& text)
{
    if (text == multipleValuesValue)
    {
        return;
    }

    GetModifyInterface()->ModifyPropertyValue(nodes, Convert(text));
}

QWidget* TextComponentValue::AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    LineEdit::FieldsDescriptor descr;
    descr.valueFieldName = FastName("text");

    return (new LineEdit(descr, GetWrappersProcessor(), GetReflection(), parent))->ToWidgetCast();
}

void TextComponentValue::ReleaseEditorWidget(QWidget* editor, const QModelIndex& index)
{
    editor->deleteLater();
}

void TextComponentValue::StaticEditorPaint(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options)
{
    TextEditorDrawer().Draw(style, painter, options, GetText());
}

Any TextComponentValue::Convert(const DAVA::String& text) const
{
    return Any(text);
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

Any FastNameComponentValue::Convert(const DAVA::String& text) const
{
    return Any(FastName(text));
}

DAVA_REFLECTION_IMPL(FastNameComponentValue)
{
    ReflectionRegistrator<FastNameComponentValue>::Begin()
    .End();
}
}
}
