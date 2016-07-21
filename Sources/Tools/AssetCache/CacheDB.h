#ifndef __DAVAENGINE_ASSET_CACHE_DATA_BASE_H__
#define __DAVAENGINE_ASSET_CACHE_DATA_BASE_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

#include "AssetCache/CacheItemKey.h"

#include <atomic>

namespace DAVA
{
class KeyedArchive;
namespace AssetCache
{
class CachedItemValue;
class ServerCacheEntry;

class CacheDB final
{
    static const String DB_FILE_NAME;
    static const uint32 VERSION;

    using CacheMap = UnorderedMap<CacheItemKey, ServerCacheEntry>;
    using FastCacheMap = UnorderedMap<CacheItemKey, ServerCacheEntry*>;

public:
    CacheDB();
    ~CacheDB();

    void UpdateSettings(const FilePath& folderPath, const uint64 size, const uint32 itemsInMemory, const uint64 autoSaveTimeout);

    void Save();
    void Load();

    ServerCacheEntry* Get(const CacheItemKey& key);

    void Insert(const CacheItemKey& key, const CachedItemValue& value);
    void Remove(const CacheItemKey& key);
    void UpdateAccessTimestamp(const CacheItemKey& key);

    const FilePath& GetPath() const;
    const uint64 GetStorageSize() const;
    const uint64 GetAvailableSize() const;
    const uint64 GetUsedSize() const;

    void Update();

private:
    void Insert(const CacheItemKey& key, ServerCacheEntry&& entry);

    FilePath CreateFolderPath(const CacheItemKey& key) const;

    void Unload();

    ServerCacheEntry* FindInFastCache(const CacheItemKey& key) const;
    ServerCacheEntry* FindInFullCache(const CacheItemKey& key);
    const ServerCacheEntry* FindInFullCache(const CacheItemKey& key) const;

    void InsertInFastCache(const CacheItemKey& key, ServerCacheEntry* entry);

    void UpdateAccessTimestamp(ServerCacheEntry* entry);

    void ReduceFullCacheToSize(uint64 toSize);
    void ReduceFastCacheByCount(uint32 countToRemove);

    void RemoveFromFastCache(const FastCacheMap::iterator& it);
    void RemoveFromFullCache(const CacheMap::iterator& it);

    void Remove(const CacheMap::iterator& it);

private:
    FilePath cacheRootFolder; //path to folder with settings and cache of files
    FilePath cacheSettings; //path to settings

    uint64 maxStorageSize = 0; //maximum cache size
    uint32 maxItemsInMemory = 0; //count of items in memory, to use for fast access

    uint64 occupiedSize = 0; //used by CacheItemValues
    uint64 nextItemID = 0; //item counter, used as last access time token

    uint64 autoSaveTimeout = 0;
    uint64 lastSaveTime = 0;

    FastCacheMap fastCache; //runtime, week storage
    CacheMap fullCache; //stored on disk, strong storage

    std::atomic<bool> dbStateChanged; //flag about changes in db
};

inline const FilePath& CacheDB::GetPath() const
{
    return cacheRootFolder;
}

inline const uint64 CacheDB::GetStorageSize() const
{
    return maxStorageSize;
}

inline const uint64 CacheDB::GetUsedSize() const
{
    return occupiedSize;
}

}; // end of namespace AssetCache
}; // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_DATA_BASE_H__
