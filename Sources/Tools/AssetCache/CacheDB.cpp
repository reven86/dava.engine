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



#include "AssetCache/CacheDB.h"

#include "AssetCache/CacheItemKey.h"
#include "AssetCache/CachedFiles.h"
#include "AssetCache/ServerCacheEntry.h"

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/KeyedArchive.h"

#include "Debug/DVAssert.h"

#include "Platform/SystemTimer.h"


namespace DAVA
{
    
namespace AssetCache
{
    
const String CacheDB::DB_FILE_NAME = "cache.dat";
    
CacheDB::CacheDB()
    : dbStateChanged(false)
{
}
    
    
CacheDB::~CacheDB()
{
    Unload();
}

void CacheDB::UpdateSettings(const FilePath &folderPath, const uint64 size, const uint32 _itemsInMemory, const uint64 _autoSaveTimeout)
{
    if(cacheRootFolder != folderPath)
    {
        if(!cacheRootFolder.IsEmpty())
        {
            Unload();
        }

        cacheRootFolder = folderPath;
        cacheRootFolder.MakeDirectoryPathname();
        cacheSettings = cacheRootFolder + DB_FILE_NAME;
        
        Load();
    }
    
    if(storageSize != size)
    {
        ReduceFullCacheBySize(size);
        
        storageSize = size;
    }

    if(itemsInMemory != _itemsInMemory)
    {
        ReduceFastCacheByCount(_itemsInMemory);
        
        itemsInMemory = _itemsInMemory;
        fastCache.reserve(itemsInMemory);
    }
    
    autoSaveTimeout = _autoSaveTimeout;
    Save();
}


void CacheDB::Load()
{
    DVASSERT(fastCache.size() == 0);
    DVASSERT(fullCache.size() == 0);
    
    ScopedPtr<File> file(File::Create(cacheSettings, File::OPEN | File::READ));
    if(static_cast<File*>(file) == nullptr)
    {
        return;
    }
    
    ScopedPtr<KeyedArchive> header(new KeyedArchive());
    header->Load(file);
    
    if(header->GetString("signature") != "cache")
    {
        Logger::Error("[CacheDB::%s] Wrong signature %s", __FUNCTION__, header->GetString("signature").c_str());
        return;
    }

    if(header->GetInt32("version") != 1)
    {
        Logger::Error("[CacheDB::%s] Wrong version %d", __FUNCTION__, header->GetInt32("version"));
        return;
    }
    
    usedSize = header->GetUInt64("usedSize");
    auto cacheSize = header->GetUInt64("itemsCount");
    fullCache.reserve(static_cast<size_t>(cacheSize));

    ScopedPtr<KeyedArchive> cache(new KeyedArchive());
    cache->Load(file);
    
    for(uint64 index = 0; index < cacheSize; ++index)
    {
        KeyedArchive *itemArchieve = cache->GetArchive(Format("item_%d", index));
        DVASSERT(nullptr != itemArchieve);
        
        CacheItemKey key;
        key.Deserialize(itemArchieve);

        ServerCacheEntry entry;
        entry.Deserialize(itemArchieve);
        
        fullCache[key] = entry;
    }
    
    dbStateChanged = false;
}
    
void CacheDB::Unload()
{
    Save();
    
    for(auto & entry: fastCache)
    {
        entry.second->Unload();
    }
    
    fastCache.clear();
    fullCache.clear();
}

    
void CacheDB::Save()
{
    FileSystem::Instance()->CreateDirectory(cacheRootFolder, true);
    
    ScopedPtr<File> file(File::Create(cacheSettings, File::CREATE | File::WRITE));
    if(static_cast<File*>(file) == nullptr)
    {
        Logger::Error("[CacheDB::%s] Cannot create file %s", __FUNCTION__, cacheSettings.GetStringValue().c_str());
        return;
    }
    
    ScopedPtr<KeyedArchive> header(new KeyedArchive());
    header->SetString("signature", "cache");
    header->SetInt32("version", 1);
    header->SetUInt64("usedSize", usedSize);
    header->SetUInt64("itemsCount", fullCache.size());
    header->Save(file);
    
    
    ScopedPtr<KeyedArchive> cache(new KeyedArchive());
    uint64 index = 0;
    for (auto & item: fullCache)
    {
        ScopedPtr<KeyedArchive> itemArchieve(new KeyedArchive());
        item.first.Serialize(itemArchieve);
        item.second.Serialize(itemArchieve);
        
        cache->SetArchive(Format("item_%d", index++), itemArchieve);
    }
    cache->Save(file);
    
    dbStateChanged = false;
    lastSaveTime = SystemTimer::Instance()->AbsoluteMS();
}
    
    
    
void CacheDB::ReduceFastCacheByCount(uint32 toCount)
{
    while(toCount < fastCache.size())
    {
        RemoveOldestFromFastCache();
    }
}


void CacheDB::ReduceFullCacheBySize(uint64 toSize)
{
    while (usedSize > toSize)
    {
        const auto & found = std::min_element(fullCache.begin(), fullCache.end(), [] (const CACHE::value_type &left, const CACHE::value_type & right) -> bool
                                              {
                                                  return left.second.GetAccesID() < right.second.GetAccesID();
                                              });
        if(found != fullCache.end())
        {
            Remove(found);
        }
    }
}

void CacheDB::RemoveOldestFromFastCache()
{
    const auto & found = std::min_element(fastCache.begin(), fastCache.end(), [] (const FASTCACHE::value_type &left, const FASTCACHE::value_type & right) -> bool
                                          {
                                              return left.second->GetAccesID() < right.second->GetAccesID();
                                          });
    if(found != fastCache.end())
    {
        RemoveFromFastCache(found);
    }
}
    
    
ServerCacheEntry * CacheDB::Get(const CacheItemKey &key)
{
    ServerCacheEntry * entry = FindInFastCache(key);
    
    if(nullptr == entry)
    {
        entry = FindInFullCache(key);
        if(nullptr != entry)
        {
            entry->Load();
            InsertInFastCache(key, entry);
        }
    }

    InvalidateAccessToken(entry);
    
    return entry;
}
    
    
ServerCacheEntry * CacheDB::FindInFastCache(const CacheItemKey &key) const
{
    auto found = fastCache.find(key);
    if(found != fastCache.cend())
    {
        return found->second;
    }

    return nullptr;
}
    
ServerCacheEntry * CacheDB::FindInFullCache(const CacheItemKey &key)
{
    auto found = fullCache.find(key);
    if(found != fullCache.cend())
    {
        return &found->second;
    }
    
    return nullptr;
}

const ServerCacheEntry * CacheDB::FindInFullCache(const CacheItemKey &key) const
{
    auto found = fullCache.find(key);
    if(found != fullCache.cend())
    {
        return &found->second;
    }
    
    return nullptr;
}


void CacheDB::Insert(const CacheItemKey &key, const CachedFiles &files)
{
    auto savedPath = CreateFolderPath(key);
    auto dbFiles = files.Copy(savedPath);
    
    ServerCacheEntry entry(dbFiles);
    Insert(key, entry);
}
    
void CacheDB::Insert(const CacheItemKey &key, const ServerCacheEntry &entry)
{
    auto found = fullCache.find(key);
    if(found != fullCache.end())
    {
        IncreaseUsedSize(found->second.GetFiles());
    }

    fullCache[key] = entry;
    ServerCacheEntry *entryForFastCache = &fullCache[key];
    
    entryForFastCache->InvalidateAccesToken(nextItemID++);
    InsertInFastCache(key, entryForFastCache);
    
    auto newFilesSize = entryForFastCache->GetFiles().GetFilesSize();
    if(usedSize + newFilesSize > storageSize)
    {
        DVASSERT(storageSize > newFilesSize);
        ReduceFullCacheBySize((usedSize > newFilesSize) ? (usedSize - newFilesSize) : 0);
    }
    
    auto savedPath = CreateFolderPath(key);
    entry.GetFiles().Save(savedPath);
    usedSize += newFilesSize;
    
    dbStateChanged = true;
}

    
void CacheDB::InsertInFastCache(const CacheItemKey &key, ServerCacheEntry * entry)
{
    if(fastCache.count(key) != 0)
    {
        return;
    }
    
    if (itemsInMemory == fastCache.size() && itemsInMemory > 0)
    {
        RemoveOldestFromFastCache();
    }
    
    DVASSERT(entry->GetFiles().FilesAreLoaded() == true);

    fastCache[key] = entry;
}

    
void CacheDB::InvalidateAccessToken(const CacheItemKey &key)
{
    ServerCacheEntry * entry = FindInFastCache(key);
    if(nullptr == entry)
    {
        entry = FindInFullCache(key);
    }
    
    InvalidateAccessToken(entry);
}


void CacheDB::InvalidateAccessToken(ServerCacheEntry *entry)
{
    if(nullptr != entry)
    {
        entry->InvalidateAccesToken(nextItemID++);
        dbStateChanged = true;
    }
}

void CacheDB::Remove(const CacheItemKey &key)
{
    auto found = fullCache.find(key);
    if(found != fullCache.end())
    {
        Remove(found);
    }
    else
    {
        Logger::Error("[CacheDB::%s] Cannot find item in cache");
    }
}
    
void CacheDB::Remove(const CACHE::iterator &it)
{
    auto found = fastCache.find(it->first);
    if(found != fastCache.end())
    {
        RemoveFromFastCache(found);
    }
    
    RemoveFromFullCache(it);
    dbStateChanged = true;
}
    
void CacheDB::RemoveFromFullCache(const CACHE::iterator &it)
{
    DVASSERT(it != fullCache.end());
    
    IncreaseUsedSize(it->second.GetFiles());
    fullCache.erase(it);
}
    
    
void CacheDB::RemoveFromFastCache(const FASTCACHE::iterator &it)
{
    DVASSERT(it != fastCache.end());

    DVASSERT(it->second->GetFiles().FilesAreLoaded() == true);
    it->second->Unload();
    fastCache.erase(it);
}

    
void CacheDB::IncreaseUsedSize(const CachedFiles &files)
{
    auto fileSize = files.GetFilesSize();
    DVASSERT(fileSize <= usedSize);
    
    usedSize -= fileSize;
}

FilePath CacheDB::CreateFolderPath(const CacheItemKey &key) const
{
    return (cacheRootFolder + (key.ToString() + "/"));
}
    
void CacheDB::Update()
{
    if(dbStateChanged && (autoSaveTimeout != 0))
    {
        auto curTime = SystemTimer::Instance()->AbsoluteMS();
        if(curTime - lastSaveTime > autoSaveTimeout)
        {
            Save();
            lastSaveTime = curTime;
        }
    }
}

const uint64 CacheDB::GetAvailableSize() const
{
    DVASSERT(GetStorageSize() > GetUsedSize());
    return (GetStorageSize() - GetUsedSize());
}
    
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

