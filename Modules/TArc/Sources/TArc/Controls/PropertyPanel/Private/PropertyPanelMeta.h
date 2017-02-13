#pragma once

#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
namespace Metas
{
class ProxyMetaRequire
{
};
} // namespace Metas

namespace M
{
using ProxyMetaRequire = Meta<Metas::ProxyMetaRequire>;
} // namespace M
} // namespace DAVA