#pragma once
#ifndef __Dava_Any__
#include "Base/Any.h"
#endif

namespace DAVA
{
inline Any::Any(Any&& any)
{
    Set(std::move(any));
}

template <typename T>
inline Any::Any(T&& value, NotAny<T>)
{
    Set(std::forward<T>(value));
}

inline bool Any::IsEmpty() const
{
    return (nullptr == rtType);
}

inline void Any::Clear()
{
    anyStorage.Clear();
    rtType = nullptr;
}

inline const RtType* Any::GetRtType() const
{
    return rtType;
}

inline Any& Any::operator=(Any&& any)
{
    if (this != &any)
    {
        Set(std::move(any));
    }

    return *this;
}

inline bool Any::operator!=(const Any& any) const
{
    return !operator==(any);
}

template <typename T>
bool Any::CanGet() const
{
    using U = AnyStorage::StorableType<T>;

    if (nullptr == rtType)
    {
        return false;
    }
    else if (rtType == RtType::Instance<U>())
    {
        return true;
    }
    else if (rtType->IsPointer() && std::is_pointer<U>::value)
    {
        static const bool isVoidPtr = std::is_void<std::remove_pointer_t<U>>::value;

        // pointers can be get as void*
        if (isVoidPtr)
        {
            return true;
        }

        const RtType* utype = RtType::Instance<U>();

        // for any utype, that is "const T*"
        // utype->Decay() will return "T*"
        if (rtType == utype->Decay())
        {
            // if type is "T*" and ttype is "const T*"
            // we should allow cast from "T*" into "const T*"
            return true;
        }
    }

    return false;
}

template <typename T>
const T& Any::Get() const
{
    if (CanGet<T>())
        return anyStorage.GetAuto<T>();

    DAVA_THROW(Exception, "Any:: can't be get as requested T");
}

template <typename T>
inline const T& Any::Get(const T& defaultValue) const
{
    return CanGet<T>() ? anyStorage.GetAuto<T>() : defaultValue;
}

inline const void* Any::GetData() const
{
    return anyStorage.GetData();
}

inline void Any::Swap(Any& any)
{
    std::swap(rtType, any.rtType);
    std::swap(anyStorage, any.anyStorage);
    std::swap(compareFn, any.compareFn);
}

inline void Any::Set(const Any& any)
{
    anyStorage = any.anyStorage;
    rtType = any.rtType;
    compareFn = any.compareFn;
}

inline void Any::Set(Any&& any)
{
    rtType = any.rtType;
    compareFn = any.compareFn;
    anyStorage = std::move(any.anyStorage);

    any.rtType = nullptr;
}

template <typename T>
void Any::Set(T&& value, NotAny<T>)
{
    using U = AnyStorage::StorableType<T>;

    rtType = RtType::Instance<U>();
    anyStorage.SetAuto(std::forward<T>(value));
    compareFn = &AnyCompare<T>::IsEqual;
}

template <typename T>
bool Any::CanCast() const
{
    return CanGet<T>() || AnyCast<T>::CanCast(*this);
}

template <typename T>
T Any::Cast() const
{
    if (CanGet<T>())
        return anyStorage.GetAuto<T>();

    return AnyCast<T>::Cast(*this);
}

template <typename T>
T Any::Cast(const T& defaultValue) const
{
    if (CanGet<T>())
        return anyStorage.GetAuto<T>();

    try
    {
        return AnyCast<T>::Cast(*this);
    }
    catch (const Exception&)
    {
        return defaultValue;
    }
}

} // namespace DAVA
