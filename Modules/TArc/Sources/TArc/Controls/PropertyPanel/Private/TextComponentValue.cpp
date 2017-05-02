#include "TextComponentValue.h"
#include "TArc/Controls/LineEdit.h"
#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"
#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"
#include "TArc/Controls/CommonStrings.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
namespace TArc
{
Any TextComponentValue::GetMultipleValue() const
{
    static Any multValue = String(MultipleValuesString);
    return multValue;
}

bool TextComponentValue::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    String newStrignValue = newValue.Cast<String>();
    String currentStringValue = currentValue.Cast<String>();
    return newStrignValue != GetMultipleValue().Cast<String>() && newStrignValue != currentStringValue;
}

String TextComponentValue::GetText() const
{
    return GetValue().Cast<String>();
}

void TextComponentValue::SetText(const String& text)
{
    SetValue(text);
}

ControlProxy* TextComponentValue::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const
{
    ControlDescriptorBuilder<LineEdit::Fields> descr;
    descr[LineEdit::Fields::Text] = "text";
    descr[LineEdit::Fields::IsReadOnly] = readOnlyFieldName;
    return new LineEdit(descr, wrappersProcessor, model, parent);
}

DAVA_VIRTUAL_REFLECTION_IMPL(TextComponentValue)
{
    ReflectionRegistrator<TextComponentValue>::Begin(CreateComponentStructureWrapper<TextComponentValue>())
    .Field("text", &TextComponentValue::GetText, &TextComponentValue::SetText)[M::ProxyMetaRequire()]
    .End();
}
}
}
