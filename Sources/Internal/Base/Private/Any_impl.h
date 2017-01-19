#pragma once
#ifndef __Dava_Any__
#include "Base/Any.h"
#endif

namespace DAVA
{
namespace AnyDetail
{
using CanGetDefault = std::integral_constant<int, 0>;
using CanGetPointer = std::integral_constant<int, 1>;
using CanGetVoidPointer = std::integral_constant<int, 2>;
//using CanGetEnumOrIntegral = std::integral_constant<int, 2>;

template <typename T>
inline bool CanGetImpl(const Type* t, CanGetDefault)
{
    return (t == Type::Instance<T>());
}

template <typename T>
inline bool CanGetImpl(const Type* t, CanGetPointer)
{
    // We should allow cast from "T*" into "const T*".
    // For any type, that is "const T*" type->Decay() will return "T*".
    const Type* rt = Type::Instance<T>();
    return (t == rt || t == rt->Decay());
}

template <typename T>
inline bool CanGetImpl(const Type* t, CanGetVoidPointer)
{
    // any pointer can be get as "void *"
    return t->IsPointer();
}

} // AnyDetail

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
    return (nullptr == type);
}

inline void Any::Clear()
{
    anyStorage.Clear();
    type = nullptr;
}

inline const Type* Any::GetType() const
{
    return type;
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
    using CanGetPointerPolicy = typename std::conditional<std::is_void<std::remove_pointer_t<T>>::value, AnyDetail::CanGetVoidPointer, AnyDetail::CanGetPointer>::type;
    using CanGetPolicy = typename std::conditional<!std::is_pointer<U>::value, AnyDetail::CanGetDefault, CanGetPointerPolicy>::type;

    if (nullptr == type)
    {
        return false;
    }

    return AnyDetail::CanGetImpl<U>(type, CanGetPolicy());

    /*
    else if (type == Type::Instance<U>())
    {
        return true;
    }
    else if (type->IsPointer() && std::is_pointer<U>::value)
    {
        static const bool isVoidPtr = std::is_void<std::remove_pointer_t<U>>::value;

        // pointers can be get as void*
        if (isVoidPtr)
        {
            return true;
        }

        const Type* utype = Type::Instance<U>();

        // for any utype, that is "const T*"
        // utype->Decay() will return "T*"
        if (type == utype->Decay())
        {
            // if type is "T*" and ttype is "const T*"
            // we should allow cast from "T*" into "const T*"
            return true;
        }
    }

    return false;
    */
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
    std::swap(type, any.type);
    std::swap(anyStorage, any.anyStorage);
    std::swap(compareFn, any.compareFn);
}

inline void Any::Set(const Any& any)
{
    anyStorage = any.anyStorage;
    type = any.type;
    compareFn = any.compareFn;
}

inline void Any::Set(Any&& any)
{
    type = any.type;
    compareFn = any.compareFn;
    anyStorage = std::move(any.anyStorage);

    any.type = nullptr;
}

template <typename T>
void Any::Set(T&& value, NotAny<T>)
{
    using U = AnyStorage::StorableType<T>;

    type = Type::Instance<U>();
    anyStorage.SetAuto(std::forward<T>(value));
    compareFn = &AnyCompare<U>::IsEqual;
}

template <typename T>
bool Any::CanCast() const
{
    return CanGet<T>() || AnyDetail::AnyCastImpl<T>::CanCast(*this);
}

template <typename T>
T Any::Cast() const
{
    if (CanGet<T>())
        return anyStorage.GetAuto<T>();

    return AnyDetail::AnyCastImpl<T>::Cast(*this);
}

template <typename T>
T Any::Cast(const T& defaultValue) const
{
    if (CanGet<T>())
        return anyStorage.GetAuto<T>();

    return AnyDetail::AnyCastImpl<T>::template Cast<T>(*this, defaultValue);
}

} // namespace DAVA
