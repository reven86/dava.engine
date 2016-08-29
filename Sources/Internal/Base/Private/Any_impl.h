#pragma once
#ifndef DAVAENGINE_ANY__H
#include "Base/Any.h"
#endif

namespace DAVA
{
inline bool Any::CastOPKey::operator==(const Any::CastOPKey& key) const
{
    return from == key.from && to == key.to;
}

inline size_t Any::CastOPKeyHasher::operator()(const DAVA::Any::CastOPKey& key) const
{
    return std::hash<const Type*>()(key.from) ^ std::hash<const Type*>()(key.to);
}

namespace AnyDetails
{
template <typename T>
bool DefaultCompareOP(const void* lp, const void* rp)
{
    auto _lp = static_cast<const T*>(lp);
    auto _rp = static_cast<const T*>(rp);

    return (*_lp == *_rp);
}

template <typename T>
void DefaultLoadOP(Any::AnyStorage& storage, const void* src)
{
    const T* data = static_cast<const T*>(src);
    storage.SetAuto(*data);
}

template <typename T>
void DefaultStoreOP(const Any::AnyStorage& storage, void* dst)
{
    T* data = static_cast<T*>(dst);
    *data = storage.GetAuto<T>();
}

template <typename From, typename To>
Any DefaultCastOP(const Any& from)
{
    return Any(static_cast<To>(from.Get<From>()));
}

template <typename T>
std::pair<const Type*, Any::AnyOPs> MakeDefaultOPs()
{
    Any::AnyOPs ops;
    ops.load = &DefaultLoadOP<T>;
    ops.store = &DefaultStoreOP<T>;
    ops.compare = &DefaultCompareOP<T>;

    return std::make_pair(Type::Instance<T>(), std::move(ops));
}

} // namespace AnyDetails

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
        return GetImpl<T>();

    throw Exception(Exception::BadGet, "Value can't be get as requested T");
}

template <typename T>
inline const T& Any::Get(const T& defaultValue) const
{
    return CanGet<T>() ? GetImpl<T>() : defaultValue;
}

template <typename T>
inline const T& Any::GetImpl() const
{
    return anyStorage.GetAuto<T>();
}

inline void Any::Set(const Any& any)
{
    anyStorage = any.anyStorage;
    type = any.type;
}

inline void Any::Set(Any&& any)
{
    anyStorage = std::move(any.anyStorage);
    type = any.type;
    any.type = nullptr;
}

template <typename T>
void Any::Set(T&& value, NotAny<T>)
{
    using U = AnyStorage::StorableType<T>;

    type = Type::Instance<U>();
    anyStorage.SetAuto(std::forward<T>(value));
}

template <typename T>
bool Any::CanCast() const
{
    static const std::integral_constant<bool, std::is_pointer<T>::value> isPointer{};
    return CanGet<T>() || CanCastImpl<T>(isPointer);
}

template <typename T>
inline bool Any::CanCastImpl(std::true_type isPointer) const
{
    using P = std::remove_cv_t<std::remove_pointer_t<T>>;
    const Type* ptype = Type::Instance<P>();

    return TypeCast::CanCast(type->Deref(), ptype);
}

template <typename T>
inline bool Any::CanCastImpl(std::false_type isPointer) const
{
    using U = AnyStorage::StorableType<std::remove_cv_t<T>>;

    auto it = castOPsMap->find(CastOPKey{ type, Type::Instance<T>() });
    if (it != castOPsMap->end())
    {
        return true;
    }

    return false;
}

template <typename T>
T Any::Cast() const
{
    static const std::integral_constant<bool, std::is_pointer<T>::value> isPointer{};
    if (CanGet<T>())
        return GetImpl<T>();

    return CastImpl<T>(isPointer);
}

template <typename T>
inline T Any::CastImpl(std::true_type isPointer) const
{
    using P = std::remove_cv_t<std::remove_pointer_t<T>>;
    const Type* ptype = Type::Instance<P>();

    void* inPtr = GetImpl<void*>();
    void* outPtr = nullptr;

    if (TypeCast::Cast(type->Deref(), inPtr, ptype, &outPtr))
    {
        return static_cast<T>(outPtr);
    }

    throw Exception(Exception::BadCast, "Pointer value can't be casted into requested T");
}

template <typename T>
inline T Any::CastImpl(std::false_type isPointer) const
{
    using U = AnyStorage::StorableType<std::remove_cv_t<T>>;

    auto it = castOPsMap->find(CastOPKey{ type, Type::Instance<T>() });
    if (it != castOPsMap->end())
    {
        CastOP op = it->second;
        if (nullptr != op)
        {
            return op(*this).Get<T>();
        }
    }

    throw Exception(Exception::BadCast, "Value can't be casted into requested T");
}

template <typename T>
void Any::RegisterDefaultOPs()
{
    anyOPsMap->emplace(AnyDetails::MakeDefaultOPs<T>());
}

template <typename T>
void Any::RegisterOPs(AnyOPs&& ops)
{
    anyOPsMap->emplace(std::make_pair(Type::Instance<T>(), std::move(ops)));
}

template <typename T1, typename T2>
void Any::RegisterDefaultCastOP()
{
    const Type* t1 = Type::Instance<T1>();
    const Type* t2 = Type::Instance<T2>();
    castOPsMap->operator[](CastOPKey{ t1, t2 }) = &AnyDetails::DefaultCastOP<T1, T2>;
    castOPsMap->operator[](CastOPKey{ t2, t1 }) = &AnyDetails::DefaultCastOP<T2, T1>;
}

template <typename T1, typename T2>
void Any::RegisterCastOP(CastOP& castT1T2, CastOP& castT2T1)
{
    const Type* t1 = Type::Instance<T1>();
    const Type* t2 = Type::Instance<T2>();
    castOPsMap->operator[](CastOPKey{ t1, t2 }) = castT1T2;
    castOPsMap->operator[](CastOPKey{ t2, t1 }) = castT2T1;
}

} // namespace DAVA
