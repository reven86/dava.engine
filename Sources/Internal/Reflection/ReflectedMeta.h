#pragma once
#include "Base/Type.h"

namespace DAVA
{
template <typename T, typename U = T>
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

    template <typename T, typename U>
    ReflectedMeta(Meta<T, U>&& meta);

    template <typename T>
    bool HasMeta() const;

    template <typename T>
    const T* GetMeta() const;

    template <typename T, typename U>
    void Emplace(Meta<T, U>&& meta);

protected:
    UnorderedMap<const Type*, decltype(Meta<void>::ptr)> metas;
};

template <typename T, typename TS, typename U, typename US>
ReflectedMeta operator, (Meta<T, TS> && metaa, Meta<US>&& metab);

template <typename T, typename U>
ReflectedMeta&& operator, (ReflectedMeta && rmeta, Meta<T, U>&& meta);

} // namespace DAVA
