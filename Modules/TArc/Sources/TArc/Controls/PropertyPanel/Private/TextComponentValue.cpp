#include "TextComponentValue.h"
#include "SimpleComponentLoader.h"
#include "Base/FastName.h"

#include "Reflection/ReflectionRegistrator.h"
#include <QQmlComponent>

namespace DAVA
{
namespace TArc
{
const String multipleValuesValue("<multiple values>");

QQmlComponent* DAVA::TArc::TextComponentValue::GetComponent(QQmlEngine* engine) const
{
    static SimpleComponentLoader componentLoader(engine, QUrl("qrc:/TArc/PropertyPanel/Component/TextComponent.qml"));
    return componentLoader.GetComponent();
}

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
