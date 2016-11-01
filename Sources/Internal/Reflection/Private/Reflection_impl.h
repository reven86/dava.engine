#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

#include "Reflection/Private/ValueWrapperDefault.h"

namespace DAVA
{

inline bool Reflection::IsReadonly() const
{
    return valueWrapper->IsReadonly();
}

inline const RttiType* Reflection::GetValueType() const
{
    return valueWrapper->GetType();
}

inline ReflectedObject Reflection::GetValueObject() const
{
    return valueWrapper->GetValueObject(object);
}

inline Any Reflection::GetValue() const
{
    return valueWrapper->GetValue(object);
}

inline bool Reflection::SetValue(const Any& value) const
{
    return valueWrapper->SetValue(object, value);
}

inline bool Reflection::IsValid() const
{
    return (nullptr != valueWrapper && object.IsValid());
}

inline const ReflectedType* Reflection::GetReflectedType() const
{
    return objectType;
}

template <typename Meta>
inline bool Reflection::HasMeta() const
{
    return (nullptr != objectMeta) ? objectMeta->HasMeta<Meta>() : false;
}

template <typename Meta>
inline const Meta* Reflection::GetMeta() const
{
    return (nullptr != objectMeta) ? objectMeta->GetMeta<Meta>() : nullptr;
}

template <typename T>
Reflection Reflection::Create(T& object, const ReflectedMeta* objectMeta)
{
    static ValueWrapperDefault<T> objectValueWrapper;

    const ReflectedType* objectType = ReflectedTypeDB::GetByPointer(&object);
    return Reflection(ReflectedObject(&object), objectType, objectMeta, &objectValueWrapper);
}
} // namespace DAVA
