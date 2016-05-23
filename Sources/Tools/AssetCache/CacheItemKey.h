#ifndef __DAVAENGINE_ASSET_CACHE_ITEM_KEY_H__
#define __DAVAENGINE_ASSET_CACHE_ITEM_KEY_H__

#include "AssetCache/DoubleMD5Key.h"
#include "Base/Hash.h"

namespace DAVA
{
namespace AssetCache
{
using CacheItemKey = DAVA::AssetCache::DoubleMD5Key;

String KeyToString(const CacheItemKey& key);
void StringToKey(const String& string, CacheItemKey& key);

void SerializeKey(const CacheItemKey& key, KeyedArchive* archieve);
void DeserializeKey(CacheItemKey& key, const KeyedArchive* archieve);

} // end of namespace AssetCache
} // end of namespace DAVA

namespace std
{
template <>
struct hash<DAVA::AssetCache::CacheItemKey>
{
    size_t operator()(const DAVA::AssetCache::CacheItemKey& key) const DAVA_NOEXCEPT
    {
        return DAVA::BufferHash(key.data(), static_cast<DAVA::uint32>(key.size()));
    }
}; //end of struct hash

} // end of namespace std

#endif // __DAVAENGINE_ASSET_CACHE_ITEM_KEY_H__
