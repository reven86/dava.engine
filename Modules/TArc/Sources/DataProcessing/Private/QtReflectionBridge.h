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

    DAVA::Signal<> metaObjectCreated;

private:
    friend class QtReflectionBridge;
    QtReflected(QtReflectionBridge* reflectionBridge, DataWrapper&& wrapper, QObject* parent);

    void OnDataChanged(const DataWrapper& wrapper, const DAVA::Set<DAVA::String>& fields) override;
    void CreateMetaObject();

    void FirePropertySignal(const DAVA::String& propertyName);
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
    QVariant Convert(const DAVA::Any& value);
    DAVA::Any Convert(const QVariant& value);
    friend class QtReflected;
    DAVA::UnorderedMap<const DAVA::ReflectedType*, QMetaObject*> metaObjects;

    DAVA::UnorderedMap<const DAVA::Type*, QVariant(*)(const DAVA::Any&)> anyToQVariant;
    DAVA::UnorderedMap<int, DAVA::Any(*)(const QVariant&)> qvariantToAny;
};
} // namespace TArc
} // namespace DAVA
