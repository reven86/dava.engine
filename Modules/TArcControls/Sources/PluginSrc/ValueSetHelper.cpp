#include "ValueSetHelper.h"

namespace TArcControls
{

ValueSetHelper::ValueSetHelper(QObject* parent)
    : QObject(parent)
{
}

void ValueSetHelper::SetValue(QObject* object, QString property, QVariant value)
{
    object->setProperty(property.toUtf8(), value);
}

QObject* ValueSetHelperSingletonProvider(QQmlEngine* engine, QJSEngine* scriptEngine)
{
    ValueSetHelper* helper = new ValueSetHelper(engine);
    return helper;
}

}
