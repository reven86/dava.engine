#pragma once

#ifndef DAVA_REFLECTION_DB__H
#include "../ReflectionDB.h"
#endif

#include "../ReflectionWrappersDefault.h"

namespace DAVA
{
template <typename T>
ReflectionDB* ReflectionDB::CreateDB()
{
    ReflectionDB* db = allDBs.emplace(new ReflectionDB()).first->get();
    return db;
}

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
    using DecayT = typename std::decay<T>::type;
    const Type* type = Type::Instance<DecayT>();

    if (nullptr == type->reflectionDb)
    {
        ReflectionDB* db = ReflectionDB::CreateDB<DecayT>();
        db->structureWrapper = std::make_unique<StructureWrapperDefault<DecayT>>();

        type->reflectionDb = db;
    }

    return type->reflectionDb;
}

} // namespace DAVA
