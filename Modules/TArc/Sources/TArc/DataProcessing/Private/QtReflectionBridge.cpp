#include "QtReflectionBridge.h"
#include "Base/Any.h"
#include "Debug/DVAssert.h"
#include "Base/TemplateHelpers.h"

#include "Reflection/ReflectedType.h"

#include <QtCore/private/qmetaobjectbuilder_p.h>
#include <QFileSystemModel>

Q_DECLARE_METATYPE(DAVA::char16);

namespace DAVA
{
namespace TArc
{
namespace ReflBridgeDetails
{
template <typename T>
Any ToAny(const QVariant& v)
{
    Any ret;
    ret.Set(v.value<T>());
    return ret;
}

Any QStringToAny(const QVariant& v)
{
    return Any(v.toString().toStdString());
}

template <typename T>
QVariant ToVariant(const Any& v)
{
    return QVariant::fromValue(v.Get<T>());
}

QVariant StringToVariant(const Any& v)
{
    return QVariant::fromValue(QString::fromStdString(v.Get<String>()));
}

template <typename T>
void FillConverter(UnorderedMap<const Type*, QVariant (*)(const Any&)>& anyToVar,
                   UnorderedMap<int, Any (*)(const QVariant&)>& varToAny)
{
    anyToVar.emplace(Type::Instance<T>(), &ToVariant<T>);
    varToAny.emplace(qMetaTypeId<T>(), &ToAny<T>);
}
} // namespace ReflBridgeDetails

#define FOR_ALL_BUILTIN_TYPES(F, ATV, VTA) \
    F(void*, ATV, VTA) \
    F(bool, ATV, VTA) \
    F(int8, ATV, VTA) \
    F(uint8, ATV, VTA) \
    F(int16, ATV, VTA) \
    F(uint16, ATV, VTA) \
    F(int32, ATV, VTA) \
    F(uint32, ATV, VTA) \
    F(int64, ATV, VTA) \
    F(uint64, ATV, VTA) \
    F(char8, ATV, VTA) \
    F(char16, ATV, VTA) \
    F(float32, ATV, VTA) \
    F(float64, ATV, VTA)

#define FOR_ALL_QT_SPECIFIC_TYPES(F, ATV, VTA) \
    F(QFileSystemModel*, ATV, VTA) \
    F(QModelIndex, ATV, VTA)

#define FOR_ALL_STATIC_TYPES(F, ATV, VTA) \
    FOR_ALL_BUILTIN_TYPES(F, ATV, VTA) \
    FOR_ALL_QT_SPECIFIC_TYPES(F, ATV, VTA)

#define FILL_CONVERTERS_FOR_TYPE(T, ATV, VTA) \
    ReflBridgeDetails::FillConverter<T>(ATV, VTA);

#define FILL_CONVERTES_FOR_CUSTOM_TYPE(ATV, VTA, ANY_TYPE, VAR_TYPE)\
    ATV.emplace(Type::Instance<ANY_TYPE>(), &ReflBridgeDetails::ANY_TYPE##ToVariant); \
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

int QtReflected::qt_metacall(QMetaObject::Call c, int id, void** argv)
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
        CallMethod(id, argv);
        int methodCount = qtMetaObject->methodCount() - qtMetaObject->methodOffset();
        id -= methodCount;
        break;
    }
    case QMetaObject::ReadProperty:
    case QMetaObject::WriteProperty:
    {
        int propertyCount = qtMetaObject->propertyCount() - qtMetaObject->propertyOffset();
        if (wrapper.HasData())
        {
            Reflection reflection = wrapper.GetData();
            Vector<Reflection::Field> fields = reflection.GetFields();
            const Reflection::Field& field = fields[id];
            QVariant* value = reinterpret_cast<QVariant*>(argv[0]);
            Any davaValue = field.ref.GetValue();
            if (c == QMetaObject::ReadProperty)
            {
                *value = reflectionBridge->Convert(davaValue);
            }
            else
            {
                Any newValue = reflectionBridge->Convert(*value);
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

void QtReflected::OnDataChanged(const DataWrapper& dataWrapper, const Set<String>& fields)
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
        for (const String& fieldName : fields)
        {
            FirePropertySignal(fieldName);
        }
    }
}

void QtReflected::CreateMetaObject()
{
    DVASSERT(reflectionBridge != nullptr);
    DVASSERT(wrapper.HasData());
    Reflection reflectionData = wrapper.GetData();

    const ReflectedType* type = reflectionData.GetReflectedType();

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

    Vector<Reflection::Field> fields = reflectionData.GetFields();
    for (const Reflection::Field& f : fields)
    {
        QByteArray propertyName = QByteArray(f.key.Cast<String>().c_str());
        QMetaPropertyBuilder propertybuilder = builder.addProperty(propertyName, "QVariant");
        propertybuilder.setWritable(!f.ref.IsReadonly());

        QByteArray notifySignal = propertyName + "Changed";
        propertybuilder.setNotifySignal(builder.addSignal(notifySignal));
    }

    Vector<Reflection::Method> methods = reflectionData.GetMethods();
    for (const Reflection::Method& method : methods)
    {
        String signature = method.key + "(";
        const AnyFn::Params& params = method.fn.GetInvokeParams();
        size_t paramsCount = params.argsType.size();
        for (size_t i = 0; i < paramsCount; ++i)
        {
            if (i == paramsCount - 1)
            {
                signature += "QVariant";
            }
            else
            {
                signature += "QVariant,";
            }
        }
        signature += ")";

        String retValue = "QVariant";
        if (params.retType == Type::Instance<void>())
        {
            retValue = "void";
        }

        builder.addMethod(signature.c_str(), retValue.c_str());
    }

    qtMetaObject = builder.toMetaObject();

    reflectionBridge->metaObjects.emplace(type, qtMetaObject);
}

void QtReflected::FirePropertySignal(const String& propertyName)
{
    String signalName = propertyName + "Changed";
    int id = qtMetaObject->indexOfSignal(signalName.c_str());
    FirePropertySignal(id);
}

void QtReflected::FirePropertySignal(int signalId)
{
    DVASSERT(signalId != -1);
    void* argv[] = { nullptr };
    qtMetaObject->activate(this, signalId, argv);
}

void QtReflected::CallMethod(int id, void** argv)
{
    int propertyCount = qtMetaObject->propertyCount() - qtMetaObject->propertyOffset();

    // Qt store "PropertyChanged signals" as methods at the start of table.
    // argument "id" can be translated to qt table by this operation "id + qtMetaObject->methodOffset()"
    // So to calculate method index in our table we deduct property count from id,
    // because on every property we created signal.
    int methodIndexToCall = id - propertyCount;

    Reflection reflectedData = wrapper.GetData();
    Vector<Reflection::Method> methods = reflectedData.GetMethods();
    DVASSERT(methodIndexToCall < methods.size());
    Reflection::Method method = methods[methodIndexToCall];
    const AnyFn::Params& args = method.fn.GetInvokeParams();

    QVariant* qtResult = reinterpret_cast<QVariant*>(argv[0]);
    // first element in argv is pointer on return value
    size_t firstArgumentIndex = 1;
    size_t argumentsCount = args.argsType.size() + 1;

    Vector<Any> davaArguments;
    davaArguments.reserve(args.argsType.size());

    for (size_t i = firstArgumentIndex; i < argumentsCount; ++i)
    {
        davaArguments.push_back(reflectionBridge->Convert(*reinterpret_cast<QVariant*>(argv[i])));
    }

    Any davaResult;
    switch (davaArguments.size())
    {
    case 0:
        davaResult = method.fn.Invoke();
        break;
    case 1:
        davaResult = method.fn.Invoke(davaArguments[0]);
        break;
    case 2:
        davaResult = method.fn.Invoke(davaArguments[0], davaArguments[1]);
        break;
    case 3:
        davaResult = method.fn.Invoke(davaArguments[0], davaArguments[1], davaArguments[2]);
        break;
    case 4:
        davaResult = method.fn.Invoke(davaArguments[0], davaArguments[1], davaArguments[2], davaArguments[3]);
        break;
    case 5:
        davaResult = method.fn.Invoke(davaArguments[0], davaArguments[1], davaArguments[2], davaArguments[3],
                                      davaArguments[4]);
        break;
    case 6:
        davaResult = method.fn.Invoke(davaArguments[0], davaArguments[1], davaArguments[2], davaArguments[3],
                                      davaArguments[4], davaArguments[5]);
        break;
        //case 7:
        //    davaResult = method.fn.Invoke(davaArguments[0], davaArguments[1], davaArguments[2], davaArguments[3],
        //                                  davaArguments[4], davaArguments[5], davaArguments[6]);
        //    break;
        //case 8:
        //    davaResult = method.fn.Invoke(davaArguments[0], davaArguments[1], davaArguments[2], davaArguments[3],
        //                                  davaArguments[4], davaArguments[5], davaArguments[6], davaArguments[7]);
        //    break;
        //case 9:
        //    davaResult = method.fn.Invoke(davaArguments[0], davaArguments[1], davaArguments[2], davaArguments[3],
        //                                  davaArguments[4], davaArguments[5], davaArguments[6], davaArguments[7],
        //                                  davaArguments[8]);
        break;
    default:
        DVASSERT_MSG(false, "Qt Reflection bridge support maximum 6 arguments in methods");
        break;
    }

    if (qtResult)
    {
        *qtResult = reflectionBridge->Convert(davaResult);
    }
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

QVariant QtReflectionBridge::Convert(const Any& value) const
{
    auto iter = anyToQVariant.find(value.GetType());
    if (iter == anyToQVariant.end())
    {
        DVASSERT_MSG(false, Format("Converted (Any->QVariant) has not been registered for type : %s", value.GetType()->GetName()).c_str());
        return QVariant();
    }

    return iter->second(value);
}

Any QtReflectionBridge::Convert(const QVariant& value) const
{
    auto iter = qvariantToAny.find(value.userType());
    if (iter == qvariantToAny.end())
    {
        DVASSERT_MSG(false, Format("Converted (QVariant->Any) has not been registered for userType : %d", value.userType()).c_str());
        return Any();
    }

    return iter->second(value);
}

} // namespace TArc
} // namespace DAVA
