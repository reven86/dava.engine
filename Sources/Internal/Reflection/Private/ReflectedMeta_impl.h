#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

namespace DAVA
{
template <typename T, typename U>
template <typename... Args>
Meta<T, U>::Meta(Args&&... args)
    : T(std::forward<Args>(args)...)
{
    static_assert(std::is_same<U, T>::value || std::is_base_of<U, T>::value, "T should be derived from U or same as U");
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
    const T* meta = nullptr;

    auto it = metas.find(Type::Instance<T>());
    if (it != metas.end())
    {
        // Here we know, that requested type T == Meta<U>, in other situation we will fail on search
        // As we store value in metas as Any(Meta<T, U>) and we know that Meta derived from T and T derived from U or same as T
        // we can get raw pointer from Any and cast it to const T*
        meta = static_cast<const T*>(it->second.GetData());
    }

    return meta;
}

template <typename T, typename U>
void ReflectedMeta::Emplace(Meta<T, U>&& meta)
{
    metas.emplace(Type::Instance<Meta<U>>(), std::move(meta));
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
