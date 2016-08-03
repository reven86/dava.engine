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
void DefaultSaveOP(const Any::AnyStorage& storage, void* dst)
{
    T* data = static_cast<T*>(dst);
    *data = storage.GetAuto<T>();
}

template <typename T>
std::pair<const Type*, Any::AnyOPs> DefaultOPs()
{
    Any::AnyOPs ops;
    ops.compare = &DefaultCompareOP<T>;
    ops.load = &DefaultLoadOP<T>;
    ops.save = &DefaultSaveOP<T>;

    return std::make_pair(Type::Instance<T>(), std::move(ops));
}

} // namespace AnyDetails

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
inline Any::Any(T&& value, NotAny<T>)
{
    Set(std::forward<T>(value));
}

template <typename T>
bool Any::CanGet() const
{
    if (type->IsPointer())
    {
        using PT = typename std::remove_pointer<T>::type;

        static const bool is_void_ptr = std::is_pointer<T>::value && std::is_void<PT>::value;

        if (is_void_ptr)
        {
            return true;
        }
        else
        {
            const Type* ptype = Type::Instance<typename std::decay<PT>::type>();

            if (type->Deref()->IsDerivedFrom(ptype))
            {
                return true;
            }
        }
    }

    return (type == Type::Instance<T>());
}

template <typename T>
bool Any::CanCast() const
{
    if (CanGet<T>())
        return true;

    // TODO: implement
    assert(false);
    return false;
}

template <typename T>
inline T Any::Cast() const
{
    if (CanGet<T>())
        return anyStorage.GetAuto<T>();

    // TODO: implement
    // ...

    throw Exception(Exception::BadCast, "Value can't be casted into requested T");
}

template <typename T>
inline void Any::Set(T&& value, NotAny<T>)
{
    using U = AnyStorage::StorableType<T>;

    type = Type::Instance<U>();
    anyStorage.SetAuto(std::forward<T>(value));
}

template <typename T>
inline const T& Any::Get() const
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
inline void Any::RegisterDefaultOPs(const LoadOP& lop, const SaveOP& sop, const CompareOP& cop)
{
    if (nullptr == operations)
    {
        operations = new UnorderedMap<Type*, Any::AnyOPs>({
        AnyDetails::DefaultOPs<void*>(),
        AnyDetails::DefaultOPs<bool>(),
        AnyDetails::DefaultOPs<DAVA::int8>(),
        AnyDetails::DefaultOPs<DAVA::uint8>(),
        AnyDetails::DefaultOPs<DAVA::float32>(),
        AnyDetails::DefaultOPs<DAVA::float64>(),
        AnyDetails::DefaultOPs<DAVA::int16>(),
        AnyDetails::DefaultOPs<DAVA::uint16>(),
        AnyDetails::DefaultOPs<DAVA::int32>(),
        AnyDetails::DefaultOPs<DAVA::uint32>(),
        AnyDetails::DefaultOPs<DAVA::int64>(),
        AnyDetails::DefaultOPs<DAVA::uint64>(),
        AnyDetails::DefaultOPs<DAVA::String>()
        });
    }

    AnyOPs ops;

    ops.load = lop;
    ops.save = sop;
    ops.compare = cop;

    const Type* type = Type::Instance<T>();
    *operations[type] = ops;
}

} // namespace DAVA
