#pragma once
#include "Base/Type.h"

namespace DAVA
{
template <typename T>
struct Meta
{
    template <typename... Args>
    Meta(Args&&... args);

    std::unique_ptr<void, void (*)(void*)> ptr;
};

class Type;
class ReflectedMeta final
{
public:
    ReflectedMeta() = default;

    ReflectedMeta(const ReflectedMeta&) = delete;
    ReflectedMeta& operator=(const ReflectedMeta&) = delete;

    DAVA_DEPRECATED(ReflectedMeta(ReflectedMeta&& rm)); // visual studio 2013 require this

    template <typename T>
    ReflectedMeta(Meta<T>&& meta);

    template <typename Meta>
    bool HasMeta() const;

    template <typename T>
    const T* GetMeta() const;

    template <typename T>
    void Emplace(Meta<T>&& meta);

protected:
    UnorderedMap<const Type*, decltype(Meta<void>::ptr)> metas;
};

template <typename T, typename U>
ReflectedMeta operator, (Meta<T> && metaa, Meta<U>&& metab);

template <typename T>
ReflectedMeta&& operator, (ReflectedMeta && rmeta, Meta<T>&& meta);

} // namespace DAVA
