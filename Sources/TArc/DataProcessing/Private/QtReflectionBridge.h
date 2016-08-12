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

    QtReflected(QtReflectionBridge* reflectionBridge, DataWrapper wrapper, QObject* parent);

private:
    void OnDataChanged(const DataWrapper& wrapper) override;
    void CreateMetaObject();

private:
    QPointer<QtReflectionBridge> reflectionBridge;
    DataWrapper wrapper;
    QMetaObject* qtMetaObject = nullptr;
};

class QtReflectionBridge final: public QObject
{
public:
    ~QtReflectionBridge();
    QtReflected* createQtReflected(DataWrapper wrapper, QObject* parent);

private:
    friend class QtReflected;
    DAVA::UnorderedMap<const DAVA::Type*, QMetaObject*> metaObjects;
};

}
