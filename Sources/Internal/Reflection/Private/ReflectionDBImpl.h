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
static const ReflectionDB* AutoGetReflectionDB(const T* this_)
{
    return ReflectionDB::GetGlobalDB<T>();
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
            ret = ReflectionDB::GetGlobalDB<T>();
        }
    }

    return ret;
}

template <typename T>
ReflectionDB* ReflectionDB::EditGlobalDB()
{
    static std::set<std::unique_ptr<ReflectionDB>> allDBs;

    using DecayT = typename std::decay<T>::type;
    const Type* type = Type::Instance<DecayT>();

    if (nullptr == type->reflectionDb)
    {
        ReflectionDB* db = allDBs.emplace(new ReflectionDB()).first->get();
        db->structureWrapper = std::make_unique<StructureWrapperDefault<DecayT>>();

        type->reflectionDb = db;
    }

    return type->reflectionDb;
}

template <typename B, typename D>
void ReflectionDB::RegisterBaseClass()
{
    static_assert(std::is_base_of<B, D>::value, "D should be derived from B");
    Type::Instance<D>()->baseTypes.insert(Type::Instance<B>());
}

} // namespace DAVA
