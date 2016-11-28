#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

namespace DAVA
{
template <typename T>
template <typename... Args>
Meta<T>::Meta(Args&&... args)
    : ptr(new T(std::forward<Args>(args)...), [](void* p) { delete static_cast<T*>(p); })
{
}

inline ReflectedMeta::ReflectedMeta(ReflectedMeta&& rm)
    : metas(std::move(rm.metas))
{
}

template <typename T>
inline ReflectedMeta::ReflectedMeta(Meta<T>&& meta)
{
    Emplace(std::move(meta));
}

template <typename T>
bool ReflectedMeta::HasMeta() const
{
    return metas.count(Type::Instance<T>()) > 0;
}

template <typename T>
const T* ReflectedMeta::GetMeta() const
{
    T* meta = nullptr;

    auto it = metas.find(Type::Instance<T>());
    if (it != metas.end())
    {
        meta = static_cast<T*>(it->second.get());
    }

    return meta;
}

template <typename T>
void ReflectedMeta::Emplace(Meta<T>&& meta)
{
    metas.emplace(Type::Instance<T>(), std::move(meta.ptr));
}

template <typename T, typename U>
inline ReflectedMeta operator, (Meta<T> && metaa, Meta<U>&& metab)
{
    ReflectedMeta ret;

    ret.Emplace(std::move(metaa));
    ret.Emplace(std::move(metab));

    return ret;
}

template <typename T>
ReflectedMeta&& operator, (ReflectedMeta && rmeta, Meta<T>&& meta)
{
    rmeta.Emplace(std::move(meta));
    return std::move(rmeta);
}

} // namespace DAVA
