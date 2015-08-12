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

#include "AssetCache/CachePacket.h"

namespace DAVA {
namespace AssetCache {

const String PACKET_HEADER = "AssetCachePacket";
const uint32 PACKET_VERSION = 1;

bool CachePacket::Serialize()
{
    DVASSERT(type != PACKET_UNKNOWN);

    buffer.reset(DynamicMemoryFile::Create(File::CREATE | File::WRITE));
    File* file = buffer;

    if (file->WriteString(PACKET_HEADER) == false)
        return false;

    if (file->Write(&PACKET_VERSION) != sizeof(PACKET_VERSION))
        return false;

    if (file->Write(&type) != sizeof(type))
        return false;

    if (file->Write(key.data(), key.size()) != key.size())
        return false;

    if (type == PACKET_ADD_FILES_REQUEST || type == PACKET_GET_FILES_RESPONSE)
    {
        if (files.Serialize(buffer) == false)
            return false;
    }

    if (type == PACKET_ADD_FILES_RESPONSE)
    {
        if (file->Write(&added) != sizeof(added))
            return false;
    }

    return true;
}

bool CachePacket::Deserialize()
{
    DVASSERT(buffer)

    File* file = buffer;

    String header;
    if (file->ReadString(header) == false)
        return false;

    if (header != PACKET_HEADER)
    {
        Logger::Error("[%s] Wrong packet header: %s", __FUNCTION__, header);
        return false;
    }

    uint32 version = 0;
    if (file->Read(&version) != sizeof(version))
        return false;

    if (version != PACKET_VERSION)
    {
        Logger::Error("[%s] Wrong packet version: %d", __FUNCTION__, version);
        return false;
    }

    if (file->Read(&type) != sizeof(type))
        return false;

    if (type == PACKET_UNKNOWN || type >= PACKET_COUNT)
    {
        Logger::Error("[%s] Unknown packet type: %d", __FUNCTION__, type);
        return false;
    }

    if (file->Read(key.data(), key.size()) != key.size())
        return false;

    if (type == PACKET_ADD_FILES_REQUEST || type == PACKET_GET_FILES_RESPONSE)
    {
        if (files.Deserialize(file) == false)
            return false;
    }

    if (type == PACKET_ADD_FILES_RESPONSE)
    {
        if (file->Read(&added) != sizeof(added))
            return false;
    }

    return true;
}

}}
