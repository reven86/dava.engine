#include "Reflection/ReflectedTypeDB.h"
#include "Reflection/Private/Wrappers/StructureWrapperClass.h"

namespace DAVA
{
StructureWrapperClass::StructureWrapperClass(const Type* type)
{
    FillCache(type);
}

void StructureWrapperClass::FillCache(const Type* type)
{
    FillCacheEntries(type);

    const TypeInheritance* inheritance = type->GetInheritance();
    if (nullptr != inheritance)
    {
        const Vector<TypeInheritance::Info>& baseTypesInfo = inheritance->GetBaseTypes();
        for (auto& baseInfo : baseTypesInfo)
        {
            FillCache(baseInfo.type);
        }
    }
}

void StructureWrapperClass::FillCacheEntries(const Type* type)
{
    const ReflectedType* reflectedType = ReflectedTypeDB::GetByType(type);
    const ReflectedStructure* structure = reflectedType->GetStrucutre();

    if (nullptr != structure)
    {
        for (auto& f : structure->fields)
        {
            const ReflectedStructure::Field* field = f.get();

            fieldsCache.push_back({ field, reflectedType });
            fieldsNameIndexes[field->name] = fieldsCache.size() - 1;
        }

        for (auto& m : structure->methods)
        {
            const ReflectedStructure::Method* method = m.get();

            methodsCache.push_back({ method, reflectedType });
            methodsNameIndexes[method->name] = methodsCache.size() - 1;
        }
    }
}

bool StructureWrapperClass::HasFields(const ReflectedObject& object, const ValueWrapper* vw) const
{
    return !fieldsCache.empty();
}

Reflection StructureWrapperClass::GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    if (!fieldsCache.empty())
    {
        String name = key.Cast<String>(String());
#if !defined(__DAVAENGINE_COREV2__)
        if (name.empty())
        {
            name = key.Cast<const char*>();
        }
#endif
        if (!name.empty())
        {
            auto it = fieldsNameIndexes.find(name);
            if (it != fieldsNameIndexes.end())
            {
                const ReflectedStructure::Field* field = fieldsCache[it->second].field;
                return Reflection(vw->GetValueObject(object), field->valueWrapper.get(), nullptr, field->meta.get());
            }
        }
    }

    return Reflection();
}

Vector<Reflection::Field> StructureWrapperClass::GetFields(const ReflectedObject& object, const ValueWrapper* vw) const
{
    Vector<Reflection::Field> ret;

    ret.reserve(fieldsCache.size());
    for (const CachedFieldEntry& fieldEntry : fieldsCache)
    {
        const ReflectedStructure::Field* field = fieldEntry.field;

        Any key(field->name);
        Reflection ref(vw->GetValueObject(object), field->valueWrapper.get(), nullptr, field->meta.get());
        const ReflectedType* owner = fieldEntry.owner;

        ret.emplace_back(std::move(key), std::move(ref), owner);
    }

    return ret;
}

bool StructureWrapperClass::HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const
{
    return !methodsCache.empty();
}

AnyFn StructureWrapperClass::GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    String name = key.Cast<String>(String());
    if (!name.empty())
    {
        void* this_ = vw->GetValueObject(object).GetVoidPtr();

        auto it = methodsNameIndexes.find(name);
        if (it != methodsNameIndexes.end())
        {
            return methodsCache[it->second].method->fn.BindThis(this_);
        }
    }
    return AnyFn();
}

Vector<Reflection::Method> StructureWrapperClass::GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const
{
    Vector<Reflection::Method> ret;

    void* this_ = vw->GetValueObject(object).GetVoidPtr();

    ret.reserve(methodsCache.size());
    for (const CachedMethodEntry& methodEntry : methodsCache)
    {
        const ReflectedStructure::Method* method = methodEntry.method;

        String key(method->name);
        AnyFn fn = method->fn.BindThis(this_);
        const ReflectedType* owner = methodEntry.owner;

        ret.emplace_back(std::move(key), std::move(fn), owner);
    }

    return ret;
}

} //namespace DAVA
