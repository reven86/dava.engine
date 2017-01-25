#pragma once
#include "Base/Any.h"
#include "Base/Type.h"
#include "Reflection/Private/Metas.h"

namespace DAVA
{
/**
 template T - Base of Meta
 template IndexT - Find helper type. In ReflectedMeta we store search index by Meta<IndexT, IndexT> and value as Any(Meta<T, IndexT>)
 T should be the same as IndexT, of should be derived from IndexT
*/
template <typename T, typename IndexT = T>
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

    template <typename T, typename IndexT>
    ReflectedMeta(Meta<T, IndexT>&& meta);

    template <typename T>
    bool HasMeta() const;

    template <typename T>
    const T* GetMeta() const;

    template <typename T, typename IndexT>
    void Emplace(Meta<T, IndexT>&& meta);

protected:
    UnorderedMap<const Type*, Any> metas;
};

template <typename T, typename IndexT, typename U, typename IndexU>
ReflectedMeta operator, (Meta<T, IndexT> && metaa, Meta<IndexU>&& metab);

template <typename T, typename IndexT>
ReflectedMeta&& operator, (ReflectedMeta && rmeta, Meta<T, IndexT>&& meta);

namespace M
{
using ReadOnly = Meta<Metas::ReadOnly>;
using Range = Meta<Metas::Range>;

using ValidatorResult = Metas::ValidationResult;
using TValidationFn = Metas::TValidationFn;
using Validator = Meta<Metas::Validator>;

using Enum = Meta<Metas::Enum>;
template <typename T>
using EnumT = Meta<Metas::EnumT<T>, Metas::Enum>;

using Flags = Meta<Metas::Flags>;
template <typename T>
using FlagsT = Meta<Metas::FlagsT<T>, Metas::Flags>;

using File = Meta<Metas::File>;
using Directory = Meta<Metas::Directory>;
using Group = Meta<Metas::Group>;
using ValueDescription = Meta<Metas::ValueDescription>;
}

} // namespace DAVA
