#pragma once
#ifndef DAVAENGINE_ANY__H
#include "Base/Any.h"
#endif

namespace DAVA
{
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

} // namespace AnyDetails

template <typename T>
inline Any::Any(T&& value, NotAny<T>)
{
    Set(std::forward<T>(value));
}

inline Any::Any(Any&& any)
{
    anyStorage = std::move(any.anyStorage);
    type = any.type;

    any.type = nullptr;
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
    if (type->IsPointer())
    {
        using P = typename std::remove_pointer<T>::type;

        static const bool is_void_ptr = std::is_pointer<T>::value && std::is_void<P>::value;

        if (is_void_ptr)
        {
            return true;
        }
        else
        {
            const Type* ptype = Type::Instance<U>();

            if (type->Deref()->IsDerivedFrom(ptype))
            {
                return true;
            }
        }
    }

    return (type == Type::Instance<U>());
}

template <typename T>
bool Any::CanCast() const
{
    if (CanGet<T>())
        return true;

#ifdef ANY_EXPERIMENTAL_CAST_IMPL
    CastOPKey castOPKey{ type, Type::Instance<T>() };
    return (castOPsMap->count(castOPKey) > 0);
#else
    return false;
#endif
}

template <typename T>
T Any::Cast() const
{
    if (CanGet<T>())
        return anyStorage.GetAuto<T>();

#ifdef ANY_EXPERIMENTAL_CAST_IMPL
    CastOPKey castOPKey{ type, Type::Instance<T>() };

    auto it = castOPsMap->find(castOPKey);
    if (it == castOPsMap->end())
    {
        throw Exception(Exception::BadCast, "Value can't be casted into requested T");
    }

    T ret;
    CastOP castOP = it->second;

    castOP(anyStorage.GetData(), &ret);

    return ret;
#else
    throw Exception(Exception::BadCast, "Value can't be casted into requested T");
#endif
}

template <typename T>
void Any::Set(T&& value, NotAny<T>)
{
    using U = AnyStorage::StorableType<T>;

    type = Type::Instance<U>();
    anyStorage.SetAuto(std::forward<T>(value));
}

template <typename T>
const T& Any::Get() const
{
    if (!CanGet<T>())
        throw Exception(Exception::BadGet, "Value can't be get as requested T");
    return anyStorage.GetAuto<T>();
}

template <typename T>
inline const T& Any::Get(const T& defaultValue) const
{
    return CanGet<T>() ? anyStorage.GetAuto<T>() : defaultValue;
}

template <typename T>
void Any::RegisterDefaultOPs()
{
    if (nullptr == anyOPsMap)
    {
        anyOPsMap.reset(new AnyOPsMap());
    }

    Any::AnyOPs ops;
    ops.compare = &AnyDetails::DefaultCompareOP<T>;
    ops.load = &AnyDetails::DefaultLoadOP<T>;
    ops.store = &AnyDetails::DefaultStoreOP<T>;

    anyOPsMap->emplace(std::make_pair(Type::Instance<T>(), std::move(ops)));
}

template <typename T>
void Any::RegisterOPs(AnyOPs&& ops)
{
    if (nullptr == anyOPsMap)
    {
        anyOPsMap.reset(new AnyOPsMap());
    }

    const Type* type = Type::Instance<T>();
    anyOPsMap->operator[](type) = std::move(ops);
}

#ifdef ANY_EXPERIMENTAL_CAST_IMPL

template <typename From, typename To>
static void Any::RegisterCastOP(CastOP& castOP)
{
    if (nullptr == castOPsMap)
    {
        castOPsMap.reset(new CastOPsMap());
    }

    const Type* fromType = Type::Instance<From>();
    const Type* toType = Type::Instance<To>();
    castOPsMap->operator[](CastOPKey{ fromType, toType }) = castOP;
}

#endif

} // namespace DAVA
