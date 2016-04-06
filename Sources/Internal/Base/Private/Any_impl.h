#pragma once
#ifndef DAVAENGINE_ANY__H
#include "../Any.h"
#endif

namespace DAVA
{
namespace AnyDetails
{
template <typename T>
bool DefaultCompare(const void* lp, const void* rp)
{
    auto _lp = static_cast<const T*>(lp);
    auto _rp = static_cast<const T*>(rp);

    return (*_lp == *_rp);
}

template <typename T>
void DefaultLoad(Any::Storage& storage, const void* src)
{
    const T* data = static_cast<const T*>(src);
    storage.SetAuto(*data);
}

template <typename T>
void DefaultSave(const Any::Storage& storage, void* dst)
{
    T* data = static_cast<T*>(dst);
    *data = storage.GetAuto<T>();
}

template <typename T>
std::pair<const Type*, Any::AnyOP> DefaultOP()
{
    Any::AnyOP op;
    op.compare = &DefaultCompare<T>;
    op.load = &DefaultLoad<T>;
    op.save = &DefaultSave<T>;

    return std::pair<const Type*, Any::AnyOP>(Type::Instance<T>(), op);
}
};

template <typename T>
Any::Any(T&& value, NotAny<T>)
{
    Set(std::forward<T>(value));
}

template <typename T>
inline bool Any::CanGet() const
{
    static const bool is_void_ptr = std::is_pointer<T>::value && std::is_void<typename std::remove_pointer<T>::type>::value;
    return type == Type::Instance<T>() || (is_void_ptr && type->IsPointer());
}

template <typename T>
inline bool Any::CanCast() const
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
        return Get<T>();

    // TODO: implement
    // ...

    throw Exception::BadCast;
}

template <typename T>
void Any::Set(T&& value, NotAny<T>)
{
    using U = Storage::StorableType<T>;

    type = Type::Instance<U>();
    storage.SetAuto(std::forward<T>(value));
}

template <typename T>
const T& Any::Get() const
{
    if (!CanGet<T>())
        throw Exception::BadGet;
    return storage.GetAuto<T>();
}

template <typename T>
const T& Any::Get(const T& defaultValue) const
{
    return CanGet<T>() ? storage.GetAuto<T>() : defaultValue;
}

template <typename T>
void Any::RegisterOP()
{
    const Type* type = Type::Instance<T>();
    operations[type] = AnyDetails::DefaultOP<T>();
}

template <typename T>
void Any::RegisterCustomOP(const AnyOP& ops)
{
    const Type* type = Type::Instance<T>();
    operations[type] = ops;
}

} // namespace DAVA
