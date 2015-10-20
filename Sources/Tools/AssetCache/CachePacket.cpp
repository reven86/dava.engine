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
#include "FileSystem/StaticMemoryFile.h"
#include "Network/IChannel.h"

namespace DAVA
{
namespace AssetCache
{
const uint16 PACKET_HEADER = 0xACCA;
const uint8 PACKET_VERSION = 1;

List<ScopedPtr<DynamicMemoryFile>> CachePacket::sendingPackets;

bool CachePacket::SendTo(Net::IChannel* channel)
{
    DVASSERT(channel);

    sendingPackets.push_back(serializationBuffer);

    uint32 packetId = 0;
    channel->Send(serializationBuffer->GetData(), serializationBuffer->GetSize(), 0, &packetId);
    return (packetId != 0);
}

void CachePacket::PacketSent(const uint8* buffer, size_t length)
{
    DVASSERT(sendingPackets.empty() == false);

    if (sendingPackets.empty() == false)
    {
        ScopedPtr<DynamicMemoryFile> packet = sendingPackets.front();
        DVASSERT((packet->GetData() == buffer) && (packet->GetSize() == length));

        sendingPackets.pop_front();
    }
}

std::unique_ptr<CachePacket> CachePacket::Create(const uint8* rawdata, uint32 length)
{
    ScopedPtr<File> buffer(StaticMemoryFile::Create(const_cast<uint8*>(rawdata), length, File::OPEN | File::READ));

    CachePacketHeader header;
    if (buffer->Read(&header) != sizeof(header))
    {
        Logger::Error("[CachePacket::%s] Cannot read header: %s", __FUNCTION__);
        return nullptr;
    }

    if (header.headerID != PACKET_HEADER || header.version != PACKET_VERSION || header.packetType == PACKET_UNKNOWN)
    {
        Logger::Error("[CachePacket::%s] Wrong header: id(%d), version(%d), packet type(%d)", __FUNCTION__, header.headerID, header.version, header.packetType);
        return nullptr;
    }

    std::unique_ptr<CachePacket> packet = CachePacket::CreateByType(header.packetType);
    if (packet != nullptr)
    {
        bool loaded = packet->Load(buffer);
        if (!loaded)
        {
            Logger::Error("[CachePacket::%s] Cannot load packet(type: %d)", __FUNCTION__, header.packetType);
            packet.reset();
        }
    }

    return packet;
}

std::unique_ptr<CachePacket> CachePacket::CreateByType(ePacketID type)
{
    switch (type)
    {
    case PACKET_ADD_REQUEST:
        return std::unique_ptr<CachePacket>(new AddRequestPacket());
    case PACKET_ADD_RESPONSE:
        return std::unique_ptr<CachePacket>(new AddResponsePacket());
    case PACKET_GET_REQUEST:
        return std::unique_ptr<CachePacket>(new GetRequestPacket());
    case PACKET_GET_RESPONSE:
        return std::unique_ptr<CachePacket>(new GetResponsePacket());
    case PACKET_WARMING_UP_REQUEST:
        return std::unique_ptr<CachePacket>(new WarmupRequestPacket());
    default:
    {
        Logger::Error("[CachePacket::%s] Wrong packet type: %d", __FUNCTION__, type);
        break;
    }
    }

    return nullptr;
}

CachePacket::CachePacket(ePacketID _type, bool createBuffer)
    : type(_type)
    , serializationBuffer(nullptr)
{
    if (createBuffer)
    {
        serializationBuffer.reset(DynamicMemoryFile::Create(File::CREATE | File::WRITE));
    }
}

void CachePacket::WriteHeader(File* file) const
{
    CachePacketHeader header(PACKET_HEADER, PACKET_VERSION, type);
    file->Write(&header);
}

AddRequestPacket::AddRequestPacket(const CacheItemKey& _key, const CachedItemValue& _value)
    : CachePacket(PACKET_ADD_REQUEST, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(_key.data(), static_cast<uint32>(_key.size()));
    _value.Serialize(serializationBuffer);
}

bool AddRequestPacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return ((file->Read(key.data(), keySize) == keySize) && value.Deserialize(file));
}

AddResponsePacket::AddResponsePacket(const CacheItemKey& _key, bool _added)
    : CachePacket(PACKET_ADD_RESPONSE, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(_key.data(), static_cast<uint32>(_key.size()));
    serializationBuffer->Write(&_added, sizeof(_added));
}

bool AddResponsePacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return ((file->Read(key.data(), keySize) == keySize) && (file->Read(&added) == sizeof(added)));
}

GetRequestPacket::GetRequestPacket(const CacheItemKey& _key)
    : CachePacket(PACKET_GET_REQUEST, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(_key.data(), static_cast<uint32>(_key.size()));
}

bool GetRequestPacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return (file->Read(key.data(), keySize) == keySize);
}

GetResponsePacket::GetResponsePacket(const CacheItemKey& _key, const CachedItemValue& _value)
    : CachePacket(PACKET_GET_RESPONSE, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(_key.data(), static_cast<uint32>(_key.size()));
    _value.Serialize(serializationBuffer);
}

bool GetResponsePacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return ((file->Read(key.data(), keySize) == keySize) && value.Deserialize(file));
}

WarmupRequestPacket::WarmupRequestPacket(const CacheItemKey& _key)
    : CachePacket(PACKET_WARMING_UP_REQUEST, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(_key.data(), static_cast<uint32>(_key.size()));
}

bool WarmupRequestPacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return (file->Read(key.data(), keySize) == keySize);
}

} //AssetCache
} //DAVA
