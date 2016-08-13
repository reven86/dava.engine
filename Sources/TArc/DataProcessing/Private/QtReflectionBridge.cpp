#include "QtReflectionBridge.h"
#include "Base/Any.h"

#include <QtCore/private/qmetaobjectbuilder_p.h>
#include <QFileSystemModel>

namespace tarc
{

QtReflected::QtReflected(QtReflectionBridge* reflectionBridge_, DataWrapper wrapper_, QObject* parent)
    : QObject(parent)
    , reflectionBridge(reflectionBridge_)
    , wrapper(wrapper_)
{
    DVASSERT(parent != nullptr);
}

const QMetaObject* QtReflected::metaObject() const
{
    DVASSERT(qtMetaObject != nullptr);
    return qtMetaObject;
}

int QtReflected::qt_metacall(QMetaObject::Call c, int id, void **argv)
{
    id = QObject::qt_metacall(c, id, argv);

    if (id < 0)
    {
        return id;
    }

    switch (c)
    {
    case QMetaObject::InvokeMetaMethod:
    {
        /*callMethod(id, argv);
        int methodCount = data_->metaObject_.methodCount() - data_->metaObject_.methodOffset();
        id -= methodCount;*/
        break;
    }
    case QMetaObject::ReadProperty:
    case QMetaObject::WriteProperty:
    {
        int propertyCount = qtMetaObject->propertyCount() - qtMetaObject->propertyOffset();
        if (wrapper.HasData())
        {
            DAVA::Reflection reflection = wrapper.GetData();
            const DAVA::StructureWrapper* structWrapper = reflection.GetStructure();
            DAVA::Ref::FieldsList fields = structWrapper->GetFields(reflection.GetValueObject());
            const DAVA::Ref::Field& field = fields[id];
            QVariant* value = reinterpret_cast<QVariant*>(argv[0]);
            DAVA::Any davaValue = field.valueRef.GetValue();
            if (c == QMetaObject::ReadProperty)
            {
                if (davaValue.CanGet<QFileSystemModel*>())
                    *value = QVariant::fromValue(davaValue.Get<QFileSystemModel*>());
                else
                    *value = QVariant::fromValue(QString::fromStdString(davaValue.Get<DAVA::String>()));
            }
            else
            {
                DAVA::Any newValue;
                if (value->canConvert<QFileSystemModel*>())
                    newValue = DAVA::Any(value->value<QFileSystemModel*>());
                else
                    newValue = DAVA::Any(value->value<QString>().toStdString());

                if (newValue != davaValue)
                {
                    field.valueRef.SetValue(newValue);
                    wrapper.Sync(false);
                }
            }
        }

        id -= propertyCount;
    }
    break;
    }

    return id;
}

void QtReflected::Init()
{
    wrapper.AddListener(this);
    if (wrapper.HasData())
    {
        CreateMetaObject();
    }
}

void QtReflected::OnDataChanged(const DataWrapper& dataWrapper, const DAVA::Set<DAVA::String>& fields)
{
    if (qtMetaObject == nullptr)
    {
        if (reflectionBridge == nullptr)
        {
            wrapper.RemoveListener(this);
            return;
        }

        CreateMetaObject();
    }

    if (fields.empty())
    {
        for (int i = qtMetaObject->methodOffset(); i < qtMetaObject->methodCount(); ++i)
        {
            QMetaMethod method = qtMetaObject->method(i);
            if (method.methodType() == QMetaMethod::Signal)
            {
                FirePropertySignal(i);
            }
        }
    }
    else
    {
        for (const DAVA::String& fieldName : fields)
        {
            FirePropertySignal(fieldName);
        }
    }
}

void QtReflected::CreateMetaObject()
{
    DVASSERT(reflectionBridge != nullptr);
    DVASSERT(wrapper.HasData());
    DAVA::Reflection reflectionData = wrapper.GetData();
    const DAVA::StructureWrapper* structWrapper = reflectionData.GetStructure();
    DAVA::Ref::FieldsList fields = structWrapper->GetFields(reflectionData.GetValueObject());

    QMetaObjectBuilder builder;

    // TODO with new reflection
    builder.setClassName(reflectionData.GetValueType()->GetName());
    builder.setSuperClass(&QObject::staticMetaObject);
    DAVA::Set<DAVA::String> signalNames;
    for (const DAVA::Ref::Field& f : fields)
    {
        QByteArray propertyName = QByteArray(f.key.Cast<DAVA::String>().c_str());
        QMetaPropertyBuilder propertybuilder = builder.addProperty(propertyName, "QVariant");
        propertybuilder.setWritable(!f.valueRef.IsReadonly());

        QByteArray notifySignal = propertyName + "Changed";
        signalNames.insert(notifySignal.toStdString());
        propertybuilder.setNotifySignal(builder.addSignal(notifySignal));
    }
    qtMetaObject = builder.toMetaObject();

    // TODO
    //reflectionBridge->metaObjects.emplace();
    metaObjectCreated.Emit();
}

void QtReflected::FirePropertySignal(const DAVA::String& propertyName)
{
    DAVA::String signalName = propertyName + "Changed";
    int id = qtMetaObject->indexOfSignal(signalName.c_str());
    FirePropertySignal(id);
}

void QtReflected::FirePropertySignal(int signalId)
{
    if (signalId != -1)
    {
        void* argv[] = { nullptr };
        qtMetaObject->activate(this, signalId, argv);
    }
}

QtReflectionBridge::~QtReflectionBridge()
{
    for (auto node : metaObjects)
    {
        free(node.second);
    }
}

QtReflected* QtReflectionBridge::CreateQtReflected(DataWrapper wrapper, QObject* parent)
{
    return new QtReflected(this, wrapper, parent);
}

}
