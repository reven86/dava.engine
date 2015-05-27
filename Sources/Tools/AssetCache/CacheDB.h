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


namespace std
{
    template<>
    struct hash<DAVA::AssetCache::CacheItemKey>
    {
        size_t operator()(const DAVA::AssetCache::CacheItemKey & key) const
        {
            size_t value = 0;
            for(auto i = 0; i < DAVA::AssetCache::CacheItemKey::INTERNAL_DATA_SIZE; ++i)
            {
                auto byte = key.keyData.internalData[i];
                
                value *= 0x10;
                value += ((byte & 0xF0) >> 4);
                
                value *= 0x10;
                value += (byte & 0x0F);
            }
            
            return value;
        }
    };
}



namespace DAVA
{
    
class KeyedArchive;
namespace AssetCache
{

class CacheItemKey;
class CachedFiles;
class ServerCacheEntry;
    
class CacheDB
{
    static const String DB_FILE_NAME;
    
    using CACHE = UnorderedMap<CacheItemKey, ServerCacheEntry>;
    using FASTCACHE = UnorderedMap<CacheItemKey, ServerCacheEntry *>;

public:
    
    CacheDB() = default;
    virtual ~CacheDB();

    void UpdateSettings(const FilePath &folderPath, uint64 size, uint32 itemsInMemory);
    
    void Save() const;
    void Load();
    
    ServerCacheEntry * Get(const CacheItemKey &key);

    void Insert(const CacheItemKey &key, const ServerCacheEntry &entry);
    void Insert(const CacheItemKey &key, const CachedFiles &files);
    
    void Remove(const CacheItemKey &key);
    
    const FilePath & GetPath() const;
    const uint64 GetStorageSize() const;
    const uint64 GetAvailableSize() const;
    const uint64 GetUsedSize() const;

    void Dump();
    
private:
    
    FilePath CreateFolderPath(const CacheItemKey &key) const;
    
    void Unload();
    
    void IncreaseUsedSize(const CachedFiles &files);
    
    void InsertInFastCache(const CacheItemKey &key, ServerCacheEntry * entry);
    
    void ReduceFastCacheByCount(uint32 toCount);
    void ReduceFullCacheBySize(uint64 toSize);
    
private:
    
    FilePath cacheRootFolder;           //path to folder with settings and cache of files
    FilePath cacheSettings;             //path to settings

    uint64 storageSize = 0;             //maximum cache size
    uint32 itemsInMemory = 0;           //count of items in memory, to use for fast access

    uint64 usedSize = 0;                //used by files
    uint64 nextItemID = 0;              //item counter, used as last access time token
    
    FASTCACHE fastCache;                //runtime, week storage
    CACHE fullCache;                    //stored on disk, strong storage
};
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_DATA_BASE_H__

