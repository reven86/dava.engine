#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"
#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

#include <Reflection/Reflection.h>
#include <Reflection/ReflectedStructure.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/Private/Wrappers/StructureWrapperClass.h>
#include <Base/Type.h>
#include <Base/TypeInheritance.h>

namespace DAVA
{
namespace TArc
{
ComponentStructureWrapper::ComponentStructureWrapper(const Type* type)
    : classWrapper(new StructureWrapperClass(type))
{
}

ComponentStructureWrapper::~ComponentStructureWrapper() = default;

bool ComponentStructureWrapper::HasFields(const ReflectedObject& object, const ValueWrapper* vw) const
{
    return classWrapper->HasFields(object, vw);
}

Reflection ComponentStructureWrapper::GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const
{
    const Type* objType = obj.GetReflectedType()->GetType();
    const Type* componentType = Type::Instance<BaseComponentValue>();
    Reflection field = classWrapper->GetField(obj, vw, key);
    if (nullptr != field.GetMeta<M::ProxyMetaRequire>() && TypeInheritance::CanCast(objType, componentType))
    {
        BaseComponentValue* componentValue = obj.GetPtr<BaseComponentValue>();
        if (!componentValue->nodes.empty())
        {
            const Reflection& metaProvider = componentValue->nodes.front()->field.ref;
            field = Reflection::Create(field, metaProvider);
        }
    }

    return field;
}

size_t ComponentStructureWrapper::GetFieldsCount(const ReflectedObject& object, const ValueWrapper* vw) const
{
    return 0;
}

Vector<Reflection::Field> ComponentStructureWrapper::GetFields(const ReflectedObject& obj, const ValueWrapper* vw) const
{
    const Type* objType = obj.GetReflectedType()->GetType();
    const Type* componentType = Type::Instance<BaseComponentValue>();
    Vector<Reflection::Field> fields = classWrapper->GetFields(obj, vw);
    if (TypeInheritance::CanCast(objType, componentType))
    {
        BaseComponentValue* componentValue = obj.GetPtr<BaseComponentValue>();
        if (!componentValue->nodes.empty())
        {
            const Reflection& metaProvider = componentValue->nodes.front()->field.ref;
            for (Reflection::Field& f : fields)
            {
                if (nullptr != f.ref.GetMeta<M::ProxyMetaRequire>())
                {
                    f.ref = Reflection::Create(f.ref, metaProvider);
                }
            }
        }
    }

    return fields;
}

Vector<Reflection::Field> ComponentStructureWrapper::GetFields(const ReflectedObject& object, const ValueWrapper* vw, size_t, size_t) const
{
    return Vector<Reflection::Field>();
}

bool ComponentStructureWrapper::HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const
{
    return classWrapper->HasMethods(object, vw);
}

AnyFn ComponentStructureWrapper::GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    return classWrapper->GetMethod(object, vw, key);
}

Vector<Reflection::Method> ComponentStructureWrapper::GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const
{
    return classWrapper->GetMethods(object, vw);
}

const Reflection::FieldCaps& ComponentStructureWrapper::GetFieldsCaps(const ReflectedObject& object, const ValueWrapper* vw) const
{
    return classWrapper->GetFieldsCaps(object, vw);
}

AnyFn ComponentStructureWrapper::GetFieldCreator(const ReflectedObject& object, const ValueWrapper* vw) const
{
    return classWrapper->GetFieldCreator(object, vw);
}

bool ComponentStructureWrapper::AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const
{
    return classWrapper->AddField(object, vw, key, value);
}

bool ComponentStructureWrapper::InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const
{
    return classWrapper->InsertField(object, vw, beforeKey, key, value);
}

bool ComponentStructureWrapper::RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    return classWrapper->RemoveField(object, vw, key);
}

void ComponentStructureWrapper::Update()
{
    classWrapper->Update();
}

} // namespace TArc
} // namespace DAVA