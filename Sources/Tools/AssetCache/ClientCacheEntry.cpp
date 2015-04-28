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
    : type(ENTRY_UNKNOWN)
{
    Memset(hash, 0, MD5::DIGEST_SIZE);
}
    
ClientCacheEntry::ClientCacheEntry(ClientCacheEntry::eEntryType _type, const FilePath &_pathname)
    : type(_type)
    , pathname(_pathname)
{
    if(ENTRY_FILE == type)
    {
        MD5::ForFile(pathname, hash);
    }
    else if(ENTRY_FOLDER == type)
    {
        MD5::ForDirectory(pathname, hash, false);
    }
}
    
    
void ClientCacheEntry::Serialize(KeyedArchive * archieve) const
{
    DVASSERT(nullptr != archieve);
    
    archieve->SetUInt32("type", type);
    archieve->SetByteArray("hash", hash, MD5::DIGEST_SIZE);
    archieve->SetString("path", pathname.GetAbsolutePathname());
    archieve->SetString("tool", toolDescription);

    auto count = params.size();
    archieve->SetUInt32("paramsCount", count);
    
    int32 index = 0;
    for(auto & param : params)
    {
        archieve->SetString(Format("param_%d", index++), param);
    }
}
    
void ClientCacheEntry::Deserialize(KeyedArchive * archieve)
{
    DVASSERT(nullptr != archieve);
    
    type = static_cast<eEntryType>(archieve->GetUInt32("type", ENTRY_UNKNOWN));

    auto hashArray = archieve->GetByteArray("hash");
    if(hashArray)
    {
        Memcpy(hash, hashArray, MD5::DIGEST_SIZE);
    }
    else
    {
        Memset(hash, 0, MD5::DIGEST_SIZE);
    }

    pathname = FilePath(archieve->GetString("path"));
    toolDescription = archieve->GetString("tool");

    params.clear();
    
    auto count = archieve->GetUInt32("paramsCount");
    for(uint32 i = 0; i < count; ++i)
    {
        params.push_back(archieve->GetString(Format("param_%d", i)));
    }
}
    
bool ClientCacheEntry::operator == (const ClientCacheEntry &cce) const
{
    return (    (type == cce.type)
            &&  (Memcmp(hash, cce.hash, MD5::DIGEST_SIZE) == 0)
            &&  (pathname == cce.pathname)
            &&  (toolDescription == cce.toolDescription)
            &&  (params == cce.params)
            );
}
    
bool operator < (const ClientCacheEntry& left, const ClientCacheEntry& right)
{
    if(left.type != right.type)
        return left.type < right.type;
    
    if(left.pathname != right.pathname)
        return left.pathname < right.pathname;

    if(left.toolDescription != right.toolDescription)
        return left.toolDescription < right.toolDescription;

    if(left.params != right.params)
        return left.params < right.params;
    
    return Memcmp(left.hash, right.hash, MD5::DIGEST_SIZE) < 0;
}
    
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

