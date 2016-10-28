#pragma once

#include <cassert>
#include <type_traits>
#include "Base/RttiType.h"

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

    ReflectedObject(void* ptr, const RttiType* type);

    bool IsValid() const;

    const RttiType* GetType() const;

    template <typename T>
    T* GetPtr() const;

    void* GetVoidPtr() const;

    ReflectedObject Deref() const;

protected:
    void* ptr = nullptr;
    const RttiType* type = nullptr;
};

template <typename T>
inline ReflectedObject::ReflectedObject(T* ptr_)
    : ptr(ptr_)
    , type(RttiType::Instance<T*>())
{
}

template <typename T>
inline ReflectedObject::ReflectedObject(const T* ptr_)
    : ptr(const_cast<T*>(ptr_))
    , type(RttiType::Instance<T*>())
{
}

inline ReflectedObject::ReflectedObject(void* ptr_, const RttiType* type_)
    : ptr(ptr_)
    , type(type_)
{
    assert(nullptr != type_);
    assert(type_->IsPointer());
}

inline const RttiType* ReflectedObject::GetType() const
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
    const RttiType* reqType = RttiType::Instance<T*>();

    assert(reqType == type || reqType->Decay() == type || RttiInheritance::CanCast(reqType, type));

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

    const RttiType* derefType = type->Deref();

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
