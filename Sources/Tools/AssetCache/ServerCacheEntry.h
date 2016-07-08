#ifndef __DAVAENGINE_ASSET_CACHE_SERVER_CACHE_ENTRY_H__
#define __DAVAENGINE_ASSET_CACHE_SERVER_CACHE_ENTRY_H__

#include "Base/BaseTypes.h"
#include "AssetCache/CachedItemValue.h"

namespace DAVA
{
class KeyedArchive;

namespace AssetCache
{
class ServerCacheEntry final
{
public:
    ServerCacheEntry();
    explicit ServerCacheEntry(const CachedItemValue& value);

    ServerCacheEntry(const ServerCacheEntry& right) = delete;
    ServerCacheEntry(ServerCacheEntry&& right);

    ~ServerCacheEntry() = default;

    ServerCacheEntry& operator=(const ServerCacheEntry& right) = delete;
    ServerCacheEntry& operator=(ServerCacheEntry&& right);

    bool operator==(const ServerCacheEntry& right) const;

    void Serialize(KeyedArchive* archieve) const;
    void Deserialize(KeyedArchive* archieve);

    void InvalidateAccesToken(uint64 accessID);
    const uint64 GetAccesID() const;

    const CachedItemValue& GetValue() const;

    bool Fetch(const FilePath& folder);
    void Free();

private:
    CachedItemValue value;

private:
    uint64 accessID = 0;
};

inline void ServerCacheEntry::InvalidateAccesToken(uint64 newID)
{
    accessID = newID;
}

inline const uint64 ServerCacheEntry::GetAccesID() const
{
    return accessID;
}

inline const CachedItemValue& ServerCacheEntry::GetValue() const
{
    return value;
}

} // end of namespace AssetCache
} // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_SERVER_CACHE_ENTRY_H__
