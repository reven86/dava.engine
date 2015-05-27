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



#include "AssetCache/CacheItemKey.h"
#include "FileSystem/KeyedArchive.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
    
namespace AssetCache
{
    
CacheItemKey::CacheItemKey()
{
    Memset(keyData.internalData, 0, INTERNAL_DATA_SIZE);
}

    
void CacheItemKey::Serialize(KeyedArchive * archieve) const
{
    DVASSERT(nullptr != archieve);
    
    archieve->SetByteArray("keyData", keyData.internalData, INTERNAL_DATA_SIZE);
}
    
void CacheItemKey::Deserialize(KeyedArchive * archieve)
{
    DVASSERT(nullptr != archieve);
    
    auto hashArray = archieve->GetByteArray("keyData");
    if(hashArray)
    {
        Memcpy(keyData.internalData, hashArray, INTERNAL_DATA_SIZE);
    }
}
    
bool CacheItemKey::operator == (const CacheItemKey &right) const
{
    return (Memcmp(keyData.internalData, right.keyData.internalData, INTERNAL_DATA_SIZE) == 0);
}
    
bool CacheItemKey::operator < (const CacheItemKey& right) const
{
    return (Memcmp(keyData.internalData, right.keyData.internalData, INTERNAL_DATA_SIZE) < 0);
}

bool CacheItemKey::operator() (const CacheItemKey &left, const CacheItemKey &right) const
{
    return (Memcmp(keyData.internalData, right.keyData.internalData, INTERNAL_DATA_SIZE) < 0);
}
    
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

