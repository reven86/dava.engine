#ifndef __DAVAENGINE_ASSET_CACHE_SERVER_CACHE_ENTRY_H__
#define __DAVAENGINE_ASSET_CACHE_SERVER_CACHE_ENTRY_H__

#include "Base/BaseTypes.h"
#include "AssetCache/CachedItemValue.h"
#include <chrono>

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

    void UpdateAccessTimestamp();
    uint64 GetTimestamp() const;

    const CachedItemValue& GetValue() const;

    bool Fetch(const FilePath& folder);
    void Free();

private:
    CachedItemValue value;

private:
    uint64 accessTimestamp = 0;
};

inline void ServerCacheEntry::UpdateAccessTimestamp()
{
    accessTimestamp = std::chrono::steady_clock::now().time_since_epoch().count();
}

inline uint64 ServerCacheEntry::GetTimestamp() const
{
    return accessTimestamp;
}

inline const CachedItemValue& ServerCacheEntry::GetValue() const
{
    return value;
}

} // end of namespace AssetCache
} // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_SERVER_CACHE_ENTRY_H__
