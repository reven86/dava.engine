#pragma once

#include <core_variant/variant.hpp>

#include "Base/BaseTypes.h"
#include "Base/StaticSingleton.h"
#include "FileSystem/VariantType.h"
#include "Functional/Function.h"

namespace NGTLayer
{
namespace VariantConverter
{
DAVA::VariantType Convert(wgt::Variant const& v, DAVA::MetaInfo const* info);
wgt::Variant Convert(DAVA::VariantType const& value);
}
}
