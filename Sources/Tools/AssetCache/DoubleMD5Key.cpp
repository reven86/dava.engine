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

#include "AssetCache/DoubleMD5Key.h"
#include "FileSystem/KeyedArchive.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace AssetCache
{
        
String KeyToString(const DoubleMD5Key &key)
{
    static const DAVA::uint32 bufferSize = HASH_SIZE * 2;
    Array<DAVA::char8, bufferSize + 1> buffer; // +1 is for MD5::HashToChar for \0
    
    MD5::HashToChar(key.data(), key.size(), buffer.data(), buffer.size());
    return String(buffer.data(), bufferSize);
}
  
void StringToKey(const String & string, DoubleMD5Key &key)
{
    DVASSERT(string.length() == HASH_SIZE * 2);
    
    MD5::CharToHash(string.data(), string.size(), key.data(), key.size());
}

void SerializeKey(const DoubleMD5Key & key, KeyedArchive *archieve)
{
    archieve->SetByteArray("keyData", key.data(), key.size());
}

void DeserializeKey(DoubleMD5Key & key, const KeyedArchive *archieve)
{
    auto size = archieve->GetByteArraySize("keyData");
    DVASSERT(size == HASH_SIZE);

    Memcpy(key.data(), archieve->GetByteArray("keyData"), size);
}

    
    
} // end of namespace AssetCache
} // end of namespace DAVA



