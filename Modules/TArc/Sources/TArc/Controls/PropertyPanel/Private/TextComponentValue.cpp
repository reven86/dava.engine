#include "TextComponentValue.h"
#include "SimpleComponentLoader.h"

#include "Reflection/ReflectionRegistrator.h"
#include <QQmlComponent>

namespace DAVA
{
namespace TArc
{
const QString multipleValuesValue = QStringLiteral("<multiple values>");

QQmlComponent* DAVA::TArc::TextComponentValue::GetComponent(QQmlEngine* engine) const
{
    static SimpleComponentLoader componentLoader(engine, QUrl("qrc:/TArc/PropertyPanel/Component/TextComponent.qml"));
    return componentLoader.GetComponent();
}

QString TextComponentValue::GetObjectName() const
{
    return QString::fromStdString(nodes.front()->fieldName.Cast<String>());
}

QString TextComponentValue::GetText() const
{
    Any value = nodes.front()->cachedValue;
    for (const std::shared_ptr<const PropertyNode>& node : nodes)
    {
        if (value != node->cachedValue)
        {
            return multipleValuesValue;
        }
    }

    return QString::fromStdString(value.Cast<String>());
}

void TextComponentValue::SetText(const QString& text)
{
    if (text == multipleValuesValue)
    {
        return;
    }

    Any newValue = Any(text.toStdString());
    for (std::shared_ptr<PropertyNode>& node : nodes)
    {
        node->SetValue(newValue);
    }
}

bool TextComponentValue::IsReadOnly() const
{
    return nodes.front()->reflectedObject.IsReadonly();
}

bool TextComponentValue::IsEnabled() const
{
    return true;
}

DAVA_REFLECTION_IMPL(TextComponentValue)
{
    ReflectionRegistrator<TextComponentValue>::Begin()
    .Field("objectName", &TextComponentValue::GetObjectName, nullptr)
    .Field("text", &TextComponentValue::GetText, &TextComponentValue::SetText)
    .Field("readOnly", &TextComponentValue::IsReadOnly, nullptr)
    .Field("enabled", &TextComponentValue::IsEnabled, nullptr)
    .End();
}
}
}
