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


#ifndef __DAVAENGINE_ASSET_CACHE_CLIENT_CACHE_ENTRY_H__
#define __DAVAENGINE_ASSET_CACHE_CLIENT_CACHE_ENTRY_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

#include "AssetCache/CacheItemKey.h"
#include "AssetCache/CacheItemParams.h"
#include "AssetCache/CachedFiles.h"


namespace DAVA
{
class KeyedArchive;

namespace AssetCache
{

class ClientCacheEntry
{
public:
    
    ClientCacheEntry();
    virtual ~ClientCacheEntry() = default;

    void Serialize(KeyedArchive * archieve, bool serializeData) const;
    void Deserialize(KeyedArchive * archieve);

    bool operator == (const ClientCacheEntry &right) const;

    
    void InvalidatePrimaryKey();
    void InvalidatePrimaryKey(const uint8 *digest);
    void InvalidateSecondaryKey();
    
    const CacheItemKey & GetKey() const;
    const CacheItemParams & GetParams() const;
    const CachedFiles & GetFiles() const;
    
    void AddParam(const String &param);
    void AddFile(const FilePath &pathname);
    
private:
public: //to test functionality
    CacheItemKey key;
    CacheItemParams params;
    CachedFiles files;
};

inline const CacheItemKey & ClientCacheEntry::GetKey() const
{
    return key;
}

inline const CacheItemParams & ClientCacheEntry::GetParams() const
{
    return params;
}

inline const CachedFiles & ClientCacheEntry::GetFiles() const
{
    return files;
}

    
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_CLIENT_CACHE_ENTRY_H__

