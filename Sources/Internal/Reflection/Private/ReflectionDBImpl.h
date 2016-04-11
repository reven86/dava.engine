#pragma once

#ifndef DAVA_REFLECTION_DB__H
#include "Reflection/ReflectionDB.h"
#endif

#include "Reflection/ReflectionWrappersDefault.h"

namespace DAVA
{
namespace Ref
{
template <typename T>
const ReflectionDB* AutoGetReflectionDB(const T* this_)
{
    return ReflectionDB::GetGlobalDB<T>();
}

template <typename T>
ReflectionDB* GetUniqueReflectionDB()
{
    static ReflectionDB db;
    return &db;
}
} // namespace Ref

template <typename T>
const ReflectionDB* ReflectionDB::GetGlobalDB()
{
    return ReflectionDB::EditGlobalDB<T>();
}

template <typename T>
inline const ReflectionDB* ReflectionDB::GetGlobalDB(T* object)
{
    const ReflectionDB* ret = nullptr;

    if (nullptr != object)
    {
        ret = VirtualReflectionDBGetter::Get<T>(object);
        if (nullptr == ret)
        {
            ret = ReflectionDB::EditGlobalDB<T>();
        }
    }

    return ret;
}

template <typename T>
ReflectionDB* ReflectionDB::EditGlobalDB()
{
    using DecayT = typename std::decay<T>::type;

    ReflectionDB* db = Ref::GetUniqueReflectionDB<DecayT>();
    const Type* type = Type::Instance<DecayT>();
    if (nullptr == type->reflectionDb)
    {
        db->structureWrapper = std::make_unique<StructureWrapperDefault<DecayT>>();
        // TODO:
        // more default wrappers set here
        // ...

        type->reflectionDb = db;
    }

    return db;
}

template <typename B, typename D>
void ReflectionDB::RegisterBaseClass()
{
    static_assert(std::is_base_of<B, D>::value, "D should be derived from B");
    Type::Instance<D>()->baseTypes.insert(Type::Instance<B>());
}

} // namespace DAVA
