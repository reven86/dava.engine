#ifndef __QTTOOLS_VARIANTCONVERTER_H__
#define __QTTOOLS_VARIANTCONVERTER_H__

#include "Base/BaseTypes.h"
#include "Base/StaticSingleton.h"
#include "FileSystem/VariantType.h"
#include "Functional/Function.h"

#include "core_variant/variant.hpp"

namespace NGTLayer
{
namespace VariantConverter
{
DAVA::VariantType Convert(Variant const& v, DAVA::MetaInfo const* info);
Variant Convert(DAVA::VariantType const& value);
}
}

#endif // __QTTOOLS_VARIANTCONVERTER_H__