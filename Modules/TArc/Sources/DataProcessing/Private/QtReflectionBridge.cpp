#include "QtReflectionBridge.h"
#include "Base/Any.h"
#include "Debug/DVAssert.h"

#include "Reflection/Public/ReflectedType.h"

#include <QtCore/private/qmetaobjectbuilder_p.h>
#include <QFileSystemModel>

Q_DECLARE_METATYPE(DAVA::char16);

namespace tarc
{

namespace ReflBridgeDetails
{

template<typename T>
DAVA::Any ToAny(const QVariant& v)
{
    DAVA::Any ret;
    ret.Set(v.value<T>());
    return ret;
}

DAVA::Any QStringToAny(const QVariant& v)
{
    return DAVA::Any(v.toString().toStdString());
}

template<typename T>
QVariant ToVariant(const DAVA::Any& v)
{
    return QVariant::fromValue(v.Get<T>());
}

QVariant StringToVariant(const DAVA::Any& v)
{
    return QVariant::fromValue(QString::fromStdString(v.Get<DAVA::String>()));
}


template<typename T>
void FillConverter(DAVA::UnorderedMap<const DAVA::Type*, QVariant(*)(const DAVA::Any&)> & anyToVar,
                   DAVA::UnorderedMap<int, DAVA::Any(*)(const QVariant&)> & varToAny)
{
    anyToVar.emplace(DAVA::Type::Instance<T>(), &ToVariant<T>);
    varToAny.emplace(qMetaTypeId<T>(), &ToAny<T>);
}

}

#define FOR_ALL_BUILDIN_TYPES(F, ATV, VTA) \
    F(void*, ATV, VTA) \
    F(bool, ATV, VTA) \
    F(DAVA::int8, ATV, VTA) \
    F(DAVA::uint8, ATV, VTA) \
    F(DAVA::int16, ATV, VTA) \
    F(DAVA::uint16, ATV, VTA) \
    F(DAVA::int32, ATV, VTA) \
    F(DAVA::uint32, ATV, VTA) \
    F(DAVA::int64, ATV, VTA) \
    F(DAVA::uint64, ATV, VTA) \
    F(DAVA::char8, ATV, VTA) \
    F(DAVA::char16, ATV, VTA) \
    F(DAVA::float32, ATV, VTA) \
    F(DAVA::float64, ATV, VTA)

#define FOR_ALL_QT_SPECIFIC_TYPES(F, ATV, VTA) \
    F(QFileSystemModel*, ATV, VTA)

#define FOR_ALL_STATIC_TYPES(F, ATV, VTA) \
    FOR_ALL_BUILDIN_TYPES(F, ATV, VTA) \
    FOR_ALL_QT_SPECIFIC_TYPES(F, ATV, VTA)

#define FILL_CONVERTERS_FOR_TYPE(T, ATV, VTA) \
    ReflBridgeDetails::FillConverter<T>(ATV, VTA);

#define FILL_CONVERTES_FOR_CUSTOM_TYPE(ATV, VTA, ANY_TYPE, VAR_TYPE)\
    ATV.emplace(DAVA::Type::Instance<DAVA::ANY_TYPE>(), &ReflBridgeDetails::ANY_TYPE##ToVariant); \
    VTA.emplace(qMetaTypeId<VAR_TYPE>(), &ReflBridgeDetails::VAR_TYPE##ToAny);

QtReflected::QtReflected(QtReflectionBridge* reflectionBridge_, DataWrapper&& wrapper_, QObject* parent)
    : QObject(parent)
    , reflectionBridge(reflectionBridge_)
    , wrapper(std::move(wrapper_))
{
    DVASSERT(parent != nullptr);
}

const QMetaObject* QtReflected::metaObject() const
{
    if (qtMetaObject == nullptr)
        return &QObject::staticMetaObject;

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
                auto iter = reflectionBridge->anyToQVariant.find(davaValue.GetType());
                if (iter == reflectionBridge->anyToQVariant.end())
                {
                    DVASSERT_MSG(false, DAVA::Format("Converted (Any->QVariant) has not been registered for type : %s", davaValue.GetType()->GetName()).c_str());
                }
                else
                {
                    *value = iter->second(davaValue);
                }
            }
            else
            {
                DAVA::Any newValue;
                auto iter = reflectionBridge->qvariantToAny.find(value->userType());
                if (iter == reflectionBridge->qvariantToAny.end())
                {
                    DVASSERT_MSG(false, DAVA::Format("Converted (QVariant->Any) has not been registered for userType : %d", value->userType()).c_str());
                }
                else
                {
                    newValue = iter->second(*value);
                }

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
    const DAVA::ReflectedType* type = DAVA::ReflectedType::GetByPointer(reflectionData.GetValueObject().GetPtr<tarc::DataNode>());

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

QtReflectionBridge::QtReflectionBridge()
{
    FOR_ALL_STATIC_TYPES(FILL_CONVERTERS_FOR_TYPE, anyToQVariant, qvariantToAny);
    FILL_CONVERTES_FOR_CUSTOM_TYPE(anyToQVariant, qvariantToAny, String, QString);
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

}
