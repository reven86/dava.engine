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

inline void Any::Swap(Any& any)
{
    std::swap(anyStorage, any.anyStorage);
    std::swap(rttiType, any.rttiType);
}

inline bool Any::IsEmpty() const
{
    return (nullptr == rttiType);
}

inline void Any::Clear()
{
    anyStorage.Clear();
    rttiType = nullptr;
}

inline const RttiType* Any::GetRttiType() const
{
    return rttiType;
}

inline Any& Any::operator=(Any&& any)
{
    if (this != &any)
    {
        rttiType = any.rttiType;
        anyStorage = std::move(any.anyStorage);
        any.rttiType = nullptr;
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

    if (nullptr == rttiType)
    {
        return false;
    }
    else if (rttiType == RttiType::Instance<U>())
    {
        return true;
    }
    else if (rttiType->IsPointer() && std::is_pointer<U>::value)
    {
        static const bool isVoidPtr = std::is_void<std::remove_pointer_t<U>>::value;

        // pointers can be get as void*
        if (isVoidPtr)
        {
            return true;
        }

        const RttiType* utype = RttiType::Instance<U>();

        // for any utype, that is "const T*"
        // utype->Decay() will return "T*"
        if (rttiType == utype->Decay())
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

inline void Any::Set(const Any& any)
{
    anyStorage = any.anyStorage;
    rttiType = any.rttiType;
    compareFn = any.compareFn;
}

inline void Any::Set(Any&& any)
{
    rttiType = any.rttiType;
    compareFn = any.compareFn;
    anyStorage = std::move(any.anyStorage);

    any.rttiType = nullptr;
}

template <typename T>
void Any::Set(T&& value, NotAny<T>)
{
    using U = AnyStorage::StorableType<T>;

    rttiType = RttiType::Instance<U>();
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
