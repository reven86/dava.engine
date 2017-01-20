#pragma once

#include "Reflection/ReflectedMeta.h"
#include "Reflection/MetaObjects.h"

namespace DAVA
{
namespace M
{
using ReadOnly = Meta<Metas::ReadOnly>;
using Range = Meta<Metas::Range>;

using ValidatorResult = Metas::ValidationResult;
using TValidationFn = Metas::TValidationFn;
using Validator = Meta<Metas::Validator>;

using Enum = Metas::Enum;
template <typename T>
using EnumT = Meta<Metas::EnumT<T>, Enum>;

using Flags = Metas::Flags;
template <typename T>
using FlagsT = Meta<Metas::FlagsT<T>, Flags>;

using File = Meta<Metas::File>;
using Directory = Meta<Metas::Directory>;
using Group = Meta<Metas::Group>;
using ValueDescription = Meta<Metas::ValueDescription>;
}
} // namespace DAVA