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
    {
        int propertyCount = qtMetaObject->propertyCount() - qtMetaObject->propertyOffset();
        if (wrapper.HasData())
        {
            DAVA::Reflection reflection = wrapper.GetData();
            const DAVA::StructureWrapper* structWrapper = reflection.GetStructure();
            DAVA::Ref::FieldsList fields = structWrapper->GetFields(reflection.GetValueObject());
            const DAVA::Ref::Field& field = fields[id];
            QVariant* returnValue = reinterpret_cast<QVariant*>(argv[0]);
            *returnValue = QVariant::fromValue(field.valueRef.GetValue().Get<QFileSystemModel*>());
        }

        id -= propertyCount;
    }
    break;
    case QMetaObject::WriteProperty:
    {

    }
    break;
    default:
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

void QtReflected::OnDataChanged(const DataWrapper&)
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
    for (const DAVA::Ref::Field& f : fields)
    {
        QByteArray propertyName = QByteArray(f.key.Cast<DAVA::String>().c_str());
        QMetaPropertyBuilder propertybuilder = builder.addProperty(propertyName, "QVariant");
        propertybuilder.setWritable(f.valueRef.IsReadonly());

        QByteArray notifySignal = propertyName + "Changed(QVariant)";
        propertybuilder.setNotifySignal(builder.addSignal(notifySignal));
    }
    qtMetaObject = builder.toMetaObject();
    metaObjectCreated.Emit();
}

QtReflectionBridge::~QtReflectionBridge()
{
    for (auto node : metaObjects)
    {
        free(node.second);
    }
}

QtReflected* QtReflectionBridge::createQtReflected(DataWrapper wrapper, QObject* parent)
{
    return new QtReflected(this, wrapper, parent);
}

}
