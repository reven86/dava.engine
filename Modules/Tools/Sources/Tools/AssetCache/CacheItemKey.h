#pragma once

#include "FileSystem/KeyedArchive.h"
#include "Base/Hash.h"
#include "Utils/MD5.h"

namespace DAVA
{
class KeyedArchive;

namespace AssetCache
{
static const uint32 HASH_SIZE = MD5::MD5Digest::DIGEST_SIZE * 2;

struct CacheItemKey : public Array<uint8, HASH_SIZE>
{
    void SetPrimaryKey(const MD5::MD5Digest& digest);
    void SetSecondaryKey(const MD5::MD5Digest& digest);

    String ToString() const;
    void FromString(const String& string);

    void Serialize(KeyedArchive* archive) const;
    void Deserialize(const KeyedArchive* archive);
};

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
