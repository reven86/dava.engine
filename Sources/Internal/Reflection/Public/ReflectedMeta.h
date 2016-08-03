#pragma once

#if !defined(__DAVAENGINE_ANDROID__)

#include "Base/Type.h"

namespace DAVA
{
template <typename T>
struct Meta
{
    using Ptr = std::unique_ptr<void, void (*)(void*)>;

    template <typename... Args>
    Meta(Args&&... args)
        : ptr(new T(std::forward<Args>(args)...), [](void* p) { delete static_cast<T*>(p); })
    {
    }

    Ptr ptr;
};

class ReflectedMeta final
{
public:
    ReflectedMeta() = default;

    ReflectedMeta(const ReflectedMeta&) = delete;
    ReflectedMeta& operator=(const ReflectedMeta&) = delete;

    DAVA_DEPRECATED(ReflectedMeta(ReflectedMeta&& rm)) // visual studio 2013 require this
    : metas(std::move(rm.metas))
    {
    }

    template <typename T>
    ReflectedMeta(Meta<T>&& meta)
    {
        Emplace(meta);
    }

    template <typename Meta>
    bool HasMeta() const
    {
        return metas.count(Type::Instance<Meta>()) > 0;
    }

    template <typename T>
    const T* GetMeta() const
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
    void Emplace(Meta<T>&& meta)
    {
        metas.emplace(Type::Instance<T>(), std::move(meta.ptr));
    }

protected:
    UnorderedMap<const Type*, Meta<void>::Ptr> metas;
};

template <typename T, typename U>
inline ReflectedMeta operator+(Meta<T>&& metaa, Meta<U>&& metab)
{
    ReflectedMeta ret;

    ret.Emplace(std::move(metaa));
    ret.Emplace(std::move(metab));

    return ret;
}

template <typename T>
ReflectedMeta&& operator+(ReflectedMeta&& rmeta, Meta<T>&& meta)
{
    rmeta.Emplace(std::move(meta));
    return std::move(rmeta);
}

} // namespace DAVA

#endif
