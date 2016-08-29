#include "QtReflectionBridge.h"
#include "Base/Any.h"
#include "Debug/DVAssert.h"

#include "Reflection/Public/ReflectedType.h"

#include <QtCore/private/qmetaobjectbuilder_p.h>
#include <QFileSystemModel>

namespace DAVA
{
namespace TArc
{
QtReflected::QtReflected(QtReflectionBridge* reflectionBridge_, DataWrapper&& wrapper_, QObject* parent)
    : QObject(parent)
    , reflectionBridge(reflectionBridge_)
    , wrapper(std::move(wrapper_))
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
            DAVA::Vector<DAVA::Reflection::Field> fields = reflection.GetFields();
            const DAVA::Reflection::Field& field = fields[id];
            QVariant* value = reinterpret_cast<QVariant*>(argv[0]);
            DAVA::Any davaValue = field.ref.GetValue();
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
                    field.ref.SetValue(newValue);
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

    // TODO how to get Real Value Type???
    const DAVA::ReflectedType* type = DAVA::ReflectedType::GetByPointer(reflectionData.GetValueObject().GetPtr<TArc::DataNode>());

    SCOPE_EXIT
    {
        metaObjectCreated.Emit();
    };

    auto iter = reflectionBridge->metaObjects.find(type);
    if (iter != reflectionBridge->metaObjects.end())
    {
        qtMetaObject = iter->second;
        return;
    }

    QMetaObjectBuilder builder;

    builder.setClassName(type->GetPermanentName().c_str());
    builder.setSuperClass(&QObject::staticMetaObject);

    DAVA::Vector<DAVA::Reflection::Field> fields = reflectionData.GetFields();
    for (const DAVA::Reflection::Field& f : fields)
    {
        QByteArray propertyName = QByteArray(f.key.Cast<DAVA::String>().c_str());
        QMetaPropertyBuilder propertybuilder = builder.addProperty(propertyName, "QVariant");
        propertybuilder.setWritable(!f.ref.IsReadonly());

        QByteArray notifySignal = propertyName + "Changed";
        propertybuilder.setNotifySignal(builder.addSignal(notifySignal));
    }
    qtMetaObject = builder.toMetaObject();

    reflectionBridge->metaObjects.emplace(type, qtMetaObject);
}

void QtReflected::FirePropertySignal(const DAVA::String& propertyName)
{
    DAVA::String signalName = propertyName + "Changed";
    int id = qtMetaObject->indexOfSignal(signalName.c_str());
    FirePropertySignal(id);
}

void QtReflected::FirePropertySignal(int signalId)
{
    DVASSERT(signalId != -1);
    void* argv[] = { nullptr };
    qtMetaObject->activate(this, signalId, argv);
}

QtReflectionBridge::~QtReflectionBridge()
{
    for (auto node : metaObjects)
    {
        free(node.second);
    }
}

QtReflected* QtReflectionBridge::CreateQtReflected(DataWrapper&& wrapper, QObject* parent)
{
    return new QtReflected(this, std::move(wrapper), parent);
}
} // namespace TArc
} // namespace DAVA
