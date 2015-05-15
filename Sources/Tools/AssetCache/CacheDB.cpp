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
#include "AssetCache/ServerCacheEntry.h"

#include "FileSystem/File.h"
#include "FileSystem/KeyedArchive.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
    
namespace AssetCache
{
    
const String CacheDB::DB_FILE_NAME = "cache.dat";

    
CacheDB::CacheDB(const FilePath &folderPath, uint64 size)
    : path(folderPath)
    , storageSize(size)
{
    path.MakeDirectoryPathname();
    path = path + DB_FILE_NAME;
}

CacheDB::~CacheDB()
{
}
 
    
void CacheDB::Save() const
{
    ScopedPtr<File> file(File::Create(path, File::CREATE | File::WRITE));
    if(static_cast<File*>(file) == nullptr)
    {
        Logger::Error("[CacheDB::%s] Cannot create file %s", __FUNCTION__, path.GetStringValue().c_str());
        return;
    }

    ScopedPtr<KeyedArchive> header(new KeyedArchive());
    header->SetString("signature", "cache");
    header->SetInt32("version", 1);
    header->SetUInt64("itemsCount", cache.size());

    header->Save(file);
    
}

void CacheDB::Load()
{
    ScopedPtr<File> file(File::Create(path, File::OPEN | File::READ));
    if(static_cast<File*>(file) == nullptr)
    {
        Logger::Error("[CacheDB::%s] Cannot create file %s", __FUNCTION__, path.GetStringValue().c_str());
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
    
}

    
void CacheDB::Serialize(KeyedArchive * archieve) const
{
    DVASSERT(nullptr != archieve);
}
    
void CacheDB::Deserialize(KeyedArchive * archieve)
{
    DVASSERT(nullptr != archieve);
}
    
bool CacheDB::operator == (const CacheDB &right) const
{
    return true;
}
    
bool CacheDB::operator < (const CacheDB& right) const
{
    return false;
}
    
bool CacheDB::Contains(const CacheItemKey &key) const
{
    auto found = cache.find(key);
    
    if(found != cache.end())
    {
        //TODO UpdateTouchTime
        return true;
    }
    
    return false;
}
    
ServerCacheEntry CacheDB::Get(const CacheItemKey &key)
{
    auto found = cache.find(key);
    if(found != cache.end())
    {
        //TODO UpdateTouchTime
        return found->second;
    }
    
    return ServerCacheEntry();
}

void CacheDB::Insert(const CacheItemKey &key, const ServerCacheEntry &entry)
{
    cache[key] = entry;
    //TODO InitializeTouchTime
}

void CacheDB::Remove(const CacheItemKey &key)
{
    auto found = cache.find(key);
    if(found != cache.end())
    {
        cache.erase(found);
    }
}

    
const FilePath & CacheDB::GetPath() const
{
    return path;
}

const uint64 CacheDB::GetStorageSize() const
{
    return storageSize;
}

const uint64 CacheDB::GetAvailableSize() const
{
    return storageSize;
}

const uint64 CacheDB::GetUsedSize() const
{
    return 0;
}
    
    
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

