#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

namespace DAVA
{
template <typename T, typename U>
template <typename... Args>
Meta<T, U>::Meta(Args&&... args)
    : ptr(new T(std::forward<Args>(args)...), [](void* p) { delete static_cast<T*>(p); })
{
}

inline ReflectedMeta::ReflectedMeta(ReflectedMeta&& rm)
    : metas(std::move(rm.metas))
{
}

template <typename T, typename U>
inline ReflectedMeta::ReflectedMeta(Meta<T, U>&& meta)
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

template <typename T, typename U>
void ReflectedMeta::Emplace(Meta<T, U>&& meta)
{
    metas.emplace(Type::Instance<U>(), std::move(meta.ptr));
}

template <typename T, typename TS, typename U, typename US>
inline ReflectedMeta operator, (Meta<T, TS> && metaa, Meta<U, US>&& metab)
{
    ReflectedMeta ret;

    ret.Emplace(std::move(metaa));
    ret.Emplace(std::move(metab));

    return ret;
}

template <typename T, typename U>
ReflectedMeta&& operator, (ReflectedMeta && rmeta, Meta<T, U>&& meta)
{
    rmeta.Emplace(std::move(meta));
    return std::move(rmeta);
}

} // namespace DAVA
