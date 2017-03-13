#pragma once

#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
namespace Metas
{
class ProxyMetaRequire
{
};

class FieldExpanded
{
public:
    bool isExpanded = true;
};
} // namespace Metas

namespace M
{
using ProxyMetaRequire = Meta<Metas::ProxyMetaRequire>;
using FieldExpanded = Meta<Metas::FieldExpanded>;
} // namespace M
} // namespace DAVA