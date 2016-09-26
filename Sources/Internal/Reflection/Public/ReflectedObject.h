#pragma once

#include <cassert>
#include <type_traits>
#include "Base/Type.h"

namespace DAVA
{
class ReflectedObject final
{
public:
    ReflectedObject() = default;

    template <typename T>
    ReflectedObject(T* ptr);

    template <typename T>
    ReflectedObject(const T* ptr);

    ReflectedObject(void* ptr, const Type* type);

    bool IsValid() const;

    const Type* GetType() const;

    template <typename T>
    T* GetPtr() const;

    void* GetVoidPtr() const;

    ReflectedObject Deref() const;

protected:
    void* ptr = nullptr;
    const Type* type = nullptr;
};

template <typename T>
inline ReflectedObject::ReflectedObject(T* ptr_)
    : ptr(ptr_)
    , type(Type::Instance<T*>())
{
}

template <typename T>
inline ReflectedObject::ReflectedObject(const T* ptr_)
    : ptr(const_cast<T*>(ptr_))
    , type(Type::Instance<T*>())
{
}

inline ReflectedObject::ReflectedObject(void* ptr_, const Type* type_)
    : ptr(ptr_)
    , type(type_)
{
    assert(nullptr != type_);
    assert(type_->IsPointer());
}

inline const Type* ReflectedObject::GetType() const
{
    return type;
}

inline bool ReflectedObject::IsValid() const
{
    return ((nullptr != ptr) && (nullptr != type));
}

template <typename T>
inline T* ReflectedObject::GetPtr() const
{
    const Type* reqType = Type::Instance<T*>();

    assert(reqType == type
           || reqType->Decay() == type
           || TypeCast::CanCast(reqType->Deref(), type->Deref())
           );

    return static_cast<T*>(ptr);
}

inline void* ReflectedObject::GetVoidPtr() const
{
    return ptr;
}

inline ReflectedObject ReflectedObject::Deref() const
{
    ReflectedObject ret;

    if (nullptr == ptr)
        return ret;

    const Type* derefType = type->Deref();

    if (nullptr == derefType)
        return ret;

    if (derefType->IsPointer())
    {
        void* inPtr = *(static_cast<void**>(ptr));
        ret = ReflectedObject(inPtr, derefType);
    }

    return ret;
}

} // namespace DAVA
