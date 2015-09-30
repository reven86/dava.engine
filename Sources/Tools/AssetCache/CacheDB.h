/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

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
    void InvalidateAccessToken(const CacheItemKey& key);

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

    void IncreaseUsedSize(DAVA::uint64 size);

    void InsertInFastCache(const CacheItemKey& key, ServerCacheEntry* entry);

    void InvalidateAccessToken(ServerCacheEntry* entry);

    void ReduceFullCacheBySize(uint64 toSize);
    void ReduceFastCacheByCount(uint32 countToRemove);

    void RemoveFromFastCache(const FastCacheMap::iterator& it);
    void RemoveFromFullCache(const CacheMap::iterator& it);

    void Remove(const CacheMap::iterator& it);

private:
    FilePath cacheRootFolder; //path to folder with settings and cache of files
    FilePath cacheSettings; //path to settings

    uint64 storageSize = 0; //maximum cache size
    uint32 maxItemsInMemory = 0; //count of items in memory, to use for fast access

    uint64 usedSize = 0; //used by CacheItemValues
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
    return storageSize;
}

inline const uint64 CacheDB::GetUsedSize() const
{
    return usedSize;
}

}; // end of namespace AssetCache
}; // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_DATA_BASE_H__
