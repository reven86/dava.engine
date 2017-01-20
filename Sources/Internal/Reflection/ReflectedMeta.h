#pragma once
#include "Base/Type.h"

namespace DAVA
{
/**
 template T - Base of Meta
 template U - Find helper type. In ReflectedMeta we store search index by Meta<U, U> and value as Any(Meta<T, U>)
 T should be the same as U, of should be derived from U
*/
template <typename T, typename U = T>
struct Meta : public T
{
    template <typename... Args>
    Meta(Args&&... args);
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
    UnorderedMap<const Type*, Any> metas;
};

template <typename T, typename TS, typename U, typename US>
ReflectedMeta operator, (Meta<T, TS> && metaa, Meta<US>&& metab);

template <typename T, typename U>
ReflectedMeta&& operator, (ReflectedMeta && rmeta, Meta<T, U>&& meta);

} // namespace DAVA
