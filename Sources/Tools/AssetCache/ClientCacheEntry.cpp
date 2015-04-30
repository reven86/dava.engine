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



#include "AssetCache/ClientCacheEntry.h"


#include "FileSystem/KeyedArchive.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
    
namespace AssetCache
{
    
ClientCacheEntry::ClientCacheEntry()
{
}
    
void ClientCacheEntry::Serialize(KeyedArchive * archieve) const
{
    DVASSERT(nullptr != archieve);
    
    ScopedPtr<KeyedArchive> keyArchieve(new KeyedArchive());
    key.Serialize(keyArchieve);
    archieve->SetArchive("key", keyArchieve);

    ScopedPtr<KeyedArchive> paramsArchieve(new KeyedArchive());
    params.Serialize(paramsArchieve);
    archieve->SetArchive("params", paramsArchieve);

    ScopedPtr<KeyedArchive> filesArchieve(new KeyedArchive());
    files.Serialize(filesArchieve);
    archieve->SetArchive("files", filesArchieve);
}
    
void ClientCacheEntry::Deserialize(KeyedArchive * archieve)
{
    DVASSERT(nullptr != archieve);
    
    KeyedArchive *keyArchieve = archieve->GetArchive("key");
    DVASSERT(keyArchieve);
    key.Deserialize(keyArchieve);
    
    KeyedArchive *paramsArchieve = archieve->GetArchive("params");
    DVASSERT(paramsArchieve);
    params.Deserialize(paramsArchieve);

    KeyedArchive *filesArchieve = archieve->GetArchive("files");
    DVASSERT(filesArchieve);
    files.Deserialize(filesArchieve);
}
    
bool ClientCacheEntry::operator == (const ClientCacheEntry &right) const
{
    return (    (key == right.key)
            &&  (params == right.params)
            &&  (files == right.files)
            );
}
    
void ClientCacheEntry::InvalidatePrimaryKey()
{
    ScopedPtr<KeyedArchive> filesArchieve(new KeyedArchive());
    files.Serialize(filesArchieve);
    
    auto dataSize = filesArchieve->Serialize(nullptr, 0);
    auto data = new uint8[dataSize];
    
    DVVERIFY(dataSize == filesArchieve->Serialize(data, dataSize));
    MD5::ForData(data, dataSize, key.keyData.hash.primary);
    
    SafeDeleteArray(data);
}
    
void ClientCacheEntry::InvalidatePrimaryKey(const uint8 *digest)
{
    Memcpy(key.keyData.hash.primary, digest, CacheItemKey::HASH_SIZE);
}
    

void ClientCacheEntry::InvalidateSecondaryKey()
{
    ScopedPtr<KeyedArchive> archieve(new KeyedArchive());
    params.Serialize(archieve);

    auto fileNames = files.GetFileNames();
    auto count = fileNames.size();
    archieve->SetUInt32("count", count);
    
    int32 index = 0;
    for(auto & file : fileNames)
    {
        archieve->SetString(Format("file_%d", index++), file.GetStringValue());
    }
    
    auto dataSize = archieve->Serialize(nullptr, 0);
    auto data = new uint8[dataSize];
    
    DVVERIFY(dataSize == archieve->Serialize(data, dataSize));
    MD5::ForData(data, dataSize, key.keyData.hash.secondary);
    
    SafeDeleteArray(data);
}
    
void ClientCacheEntry::AddParam(const String &param)
{
    DVASSERT(std::find(params.params.begin(), params.params.end(), param) == params.params.end());
    params.params.push_back(param);
}
    
void ClientCacheEntry::AddFile(const FilePath &pathname)
{
    files.AddFile(pathname);
}
    
    
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

