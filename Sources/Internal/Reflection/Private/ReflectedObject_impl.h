#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

namespace DAVA
{
template <typename T>
inline ReflectedObject::ReflectedObject(T* ptr_)
    : ptr(ptr_)
    , reflectedType(ReflectedTypeDB::GetByPointer(ptr_))
{
}

template <typename T>
inline ReflectedObject::ReflectedObject(const T* ptr_)
    : ptr(const_cast<T*>(ptr_))
    , reflectedType(ReflectedTypeDB::GetByPointer(ptr_))
    , isConst(true)
{
}

/*
inline ReflectedObject::ReflectedObject(void* ptr_, const ReflectedType* reflectedType_)
    : ptr(ptr_)
    , reflectedType(reflectedType_)
{
}

inline ReflectedObject::ReflectedObject(const void* ptr_, const ReflectedType* reflectedType_)
    : ptr(const_cast<void*>(ptr_))
    , reflectedType(reflectedType_)
    , isConst(true)
{
}
*/

inline const ReflectedType* ReflectedObject::GetReflectedType() const
{
    return reflectedType;
}

inline bool ReflectedObject::IsValid() const
{
    return ((nullptr != ptr) && (nullptr != reflectedType));
}

inline bool ReflectedObject::IsConst() const
{
    return isConst;
}

template <typename T>
inline T* ReflectedObject::GetPtr() const
{
    const Type* reqType = Type::Instance<T>();
    const Type* curType = reflectedType->GetType();

    bool canGet = (reqType == curType) || (reqType->Decay() == curType);

    if (canGet)
    {
        return static_cast<T*>(ptr);
    }

    void* tmp = nullptr;
    bool canCast = TypeInheritance::DownCast(curType, reqType, ptr, &tmp);

    DVASSERT(canCast);

    return static_cast<T*>(tmp);
}

inline void* ReflectedObject::GetVoidPtr() const
{
    return ptr;
}

} // namespace DAVA
