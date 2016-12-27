#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QJSEngine>

namespace TArcControls
{
class ValueSetHelper: public QObject
{
    Q_OBJECT
public:
    ValueSetHelper(QObject* parent);

    // We use this to set new value into 
    Q_INVOKABLE void SetValue(QObject* object, QString property, QVariant value);
};

QObject* ValueSetHelperSingletonProvider(QQmlEngine* engine, QJSEngine* scriptEngine);
}