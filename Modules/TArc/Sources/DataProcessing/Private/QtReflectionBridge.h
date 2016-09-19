#pragma once

#include "DataProcessing/DataWrapper.h"
#include "DataProcessing/DataListener.h"
#include "Functional/Signal.h"
#include "Base/BaseTypes.h"

#include <QObject>
#include <QPointer>

namespace DAVA
{
namespace TArc
{
class QtReflectionBridge;
class QtReflected final: public QObject, private DataListener
{
public:
    const QMetaObject* metaObject() const override;
    int qt_metacall(QMetaObject::Call c, int id, void **argv) override;
    void Init();

    Signal<> metaObjectCreated;

private:
    friend class QtReflectionBridge;
    QtReflected(QtReflectionBridge* reflectionBridge, DataWrapper&& wrapper, QObject* parent);

    void OnDataChanged(const DataWrapper& wrapper, const Set<String>& fields) override;
    void CreateMetaObject();

    void FirePropertySignal(const String& propertyName);
    void FirePropertySignal(int signalId);
    void CallMethod(int id, void** argv);

private:
    QPointer<QtReflectionBridge> reflectionBridge;
    DataWrapper wrapper;
    QMetaObject* qtMetaObject = nullptr;
};

class QtReflectionBridge final: public QObject
{
public:
    QtReflectionBridge();
    ~QtReflectionBridge();
    QtReflected* CreateQtReflected(DataWrapper&& wrapper, QObject* parent);

private:
    QVariant Convert(const Any& value);
    Any Convert(const QVariant& value);
    friend class QtReflected;
    UnorderedMap<const ReflectedType*, QMetaObject*> metaObjects;

    UnorderedMap<const Type*, QVariant(*)(const Any&)> anyToQVariant;
    UnorderedMap<int, Any(*)(const QVariant&)> qvariantToAny;
};
} // namespace TArc
} // namespace DAVA
