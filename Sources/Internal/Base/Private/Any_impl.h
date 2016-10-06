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
    std::swap(type, any.type);
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
        type = any.type;
        anyStorage = std::move(any.anyStorage);
        any.type = nullptr;
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

    if (nullptr == type)
    {
        return false;
    }
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
} // namespace DAVA
