#ifndef __DAVAENGINE_ASSET_DOUBLE_MD5_KEY_H__
#define __DAVAENGINE_ASSET_DOUBLE_MD5_KEY_H__

#include "Base/BaseTypes.h"
#include "Utils/MD5.h"

namespace DAVA
{
class KeyedArchive;
namespace AssetCache
{
static const uint32 HASH_SIZE = MD5::MD5Digest::DIGEST_SIZE * 2;
using DoubleMD5Key = Array<uint8, HASH_SIZE>;

} // end of namespace AssetCache
} // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_DOUBLE_MD5_KEY_H__
