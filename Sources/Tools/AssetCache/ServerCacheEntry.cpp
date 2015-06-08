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
    : accessID(0)
{
}

ServerCacheEntry::ServerCacheEntry(const CachedFiles &_files)
    : files(_files)
    , accessID(0)
{
    
}

    
bool ServerCacheEntry::operator == (const ServerCacheEntry &right) const
{
    return (accessID == right.accessID) && (files == right.files);
}

void ServerCacheEntry::Serialize(KeyedArchive * archieve) const
{
    DVASSERT(nullptr != archieve);
    
    archieve->SetUInt64("accessID", accessID);
    
    ScopedPtr<KeyedArchive> filesArchieve(new KeyedArchive());
    files.Serialize(filesArchieve, false);
    archieve->SetArchive("files", filesArchieve);
}

void ServerCacheEntry::Deserialize(KeyedArchive * archieve)
{
    DVASSERT(nullptr != archieve);
    
    accessID = archieve->GetUInt64("accessID");
    
    KeyedArchive *filesArchieve = archieve->GetArchive("files");
    DVASSERT(filesArchieve);
    files.Deserialize(filesArchieve);
}
    
    
    
void ServerCacheEntry::InvalidateAccesToken(uint64 newID)
{
    accessID = newID;
}

    
const CachedFiles & ServerCacheEntry::GetFiles() const
{
    return files;
}


    
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

