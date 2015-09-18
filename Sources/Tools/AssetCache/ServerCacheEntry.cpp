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



#include "AssetCache/ServerCacheEntry.h"


#include "FileSystem/KeyedArchive.h"

#include "Platform/SystemTimer.H"
#include "Debug/DVAssert.h"

namespace DAVA
{
    
namespace AssetCache
{
    
ServerCacheEntry::ServerCacheEntry()
{
}

ServerCacheEntry::ServerCacheEntry(const CachedItemValue &_value)
    : value(_value)
{
}

ServerCacheEntry::ServerCacheEntry(ServerCacheEntry &&right)
    : value(std::move(right.value))
    , accessID(right.accessID)
{
}

ServerCacheEntry & ServerCacheEntry::operator=(ServerCacheEntry &&right)
{
    if(this != &right)
    {
        value = std::move(right.value);
        accessID = right.accessID;
    }
    
    return (*this);
}

    

bool ServerCacheEntry::operator == (const ServerCacheEntry &right) const
{
    return (accessID == right.accessID) && (value == right.value);
}

void ServerCacheEntry::Serialize(KeyedArchive * archieve) const
{
    DVASSERT(nullptr != archieve);
    
    archieve->SetUInt64("accessID", accessID);
    
    ScopedPtr<KeyedArchive> valueArchieve(new KeyedArchive());
    value.Serialize(valueArchieve, false);
    archieve->SetArchive("value", valueArchieve);
}

void ServerCacheEntry::Deserialize(KeyedArchive * archieve)
{
    DVASSERT(nullptr != archieve);
    
    accessID = archieve->GetUInt64("accessID");
    
    KeyedArchive *valueArchieve = archieve->GetArchive("value");
    DVASSERT(valueArchieve);
    value.Deserialize(valueArchieve);
}

bool ServerCacheEntry::Fetch(const FilePath& folder)
{
    return value.Fetch(folder);
}
    
void ServerCacheEntry::Free()
{
    value.Free();
}
    
    
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

