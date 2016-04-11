#pragma once

#ifndef DAVA_REFLECTION__H
#include "Reflection/Reflection.h"
#endif

#include "Reflection/ReflectionWrappers.h"
#include "Reflection/ReflectionDB.h"

namespace DAVA
{
template <typename T>
inline Reflection Reflection::Reflect(T* object)
{
    static ValueWrapperDefault<T> vw;

    const ReflectionDB* db = ReflectionDB::GetGlobalDB(object);
    return Reflection(ReflectedObject(object), &vw, db);
}

inline bool Reflection::IsValid() const
{
    return that.IsValid();
}

inline bool Reflection::IsReadonly() const
{
    return vw->IsReadonly();
}

inline Any Reflection::GetValue() const
{
    return vw->GetValue(that);
}

inline bool Reflection::SetValue(const Any& val) const
{
    return vw->SetValue(that, val);
}

inline const Type* Reflection::GetValueType() const
{
    return vw->GetType();
}

inline ReflectedObject Reflection::GetValueObject() const
{
    return vw->GetValueObject(that);
}

inline const DtorWrapper* Reflection::GetDtor() const
{
    return db->GetDtor();
}

inline const CtorWrapper* Reflection::GetCtor() const
{
    return db->GetCtor();
}

inline const CtorWrapper* Reflection::GetCtor(const Ref::ParamsList& params) const
{
    return db->GetCtor(params);
}

inline std::vector<const CtorWrapper*> Reflection::GetCtors() const
{
    return db->GetCtors();
}

inline const StructureWrapper* Reflection::GetStructure() const
{
    return db->structureWrapper.get();
}

} // namespace DAVA
