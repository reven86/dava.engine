#pragma once

#include "DataProcessing/DataWrapper.h"
#include "Functional/Signal.h"
#include "Base/BaseTypes.h"

#include <QObject>
#include <QPointer>

namespace tarc
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

private:
    QPointer<QtReflectionBridge> reflectionBridge;
    DataWrapper wrapper;
    QMetaObject* qtMetaObject = nullptr;
};

class QtReflectionBridge final: public QObject
{
public:
    ~QtReflectionBridge();
    QtReflected* CreateQtReflected(DataWrapper&& wrapper, QObject* parent);

private:
    friend class QtReflected;
    DAVA::UnorderedMap<const DAVA::ReflectedType*, QMetaObject*> metaObjects;
};

}
