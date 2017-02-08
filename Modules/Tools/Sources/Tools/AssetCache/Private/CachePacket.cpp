#include "Tools/AssetCache/CachePacket.h"

#include <FileSystem/StaticMemoryFile.h>
#include <Network/IChannel.h>
#include <Logger/Logger.h>

namespace DAVA
{
namespace AssetCache
{
const uint16 PACKET_HEADER = 0xACCA;
const uint8 PACKET_VERSION = 2;

Map<const uint8*, ScopedPtr<DynamicMemoryFile>> CachePacket::sendingPackets;

bool CachePacket::SendTo(Net::IChannel* channel)
{
    DVASSERT(channel);

    auto insertRes = sendingPackets.insert(std::make_pair(serializationBuffer->GetData(), serializationBuffer));
    DVASSERT(true == insertRes.second && "packet is already inserted");

    uint32 packetId = 0;
    channel->Send(serializationBuffer->GetData(), static_cast<size_t>(serializationBuffer->GetSize()), 0, &packetId);
    return (packetId != 0);
}

void CachePacket::PacketSent(const uint8* buffer, size_t length)
{
    DVASSERT(sendingPackets.empty() == false);

    auto found = sendingPackets.find(buffer);
    DVASSERT(found != sendingPackets.end() && "packet is not found in sending list");
    DVASSERT(found->second->GetSize() == length);
    sendingPackets.erase(found);
}

CachePacket::CreateResult CachePacket::Create(const uint8* rawdata, uint32 length, std::unique_ptr<CachePacket>& packet)
{
    ScopedPtr<File> buffer(StaticMemoryFile::Create(const_cast<uint8*>(rawdata), length, File::OPEN | File::READ));

    CachePacketHeader header;
    if (buffer->Read(&header) != sizeof(header))
    {
        Logger::Error("[CachePacket::%s] Cannot read header: %s", __FUNCTION__);
        return ERR_INCORRECT_DATA;
    }

    if (header.headerID != PACKET_HEADER)
    {
        Logger::Error("[CachePacket::%s] Unsupported header id: %d, expected is %d", __FUNCTION__, header.headerID, PACKET_HEADER);
        return ERR_UNSUPPORTED_VERSION;
    }
    if (header.version != PACKET_VERSION)
    {
        Logger::Error("[CachePacket::%s] Unsupported header version: %d, expected is %d", __FUNCTION__, header.version, PACKET_VERSION);
        return ERR_UNSUPPORTED_VERSION;
    }

    packet = CachePacket::CreateByType(header.packetType);
    if (!packet)
    {
        return ERR_INCORRECT_DATA;
    }

    bool loaded = packet->Load(buffer);
    if (!loaded)
    {
        Logger::Error("[CachePacket::%s] Cannot load packet (type: %d)", __FUNCTION__, header.packetType);
        packet.reset();
        return ERR_INCORRECT_DATA;
    }

    return CREATED;
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
    case PACKET_STATUS_REQUEST:
        return std::unique_ptr<CachePacket>(new StatusRequestPacket());
    case PACKET_STATUS_RESPONSE:
        return std::unique_ptr<CachePacket>(new StatusResponsePacket());
    case PACKET_REMOVE_REQUEST:
        return std::unique_ptr<CachePacket>(new RemoveRequestPacket());
    case PACKET_REMOVE_RESPONSE:
        return std::unique_ptr<CachePacket>(new RemoveResponsePacket());
    case PACKET_CLEAR_REQUEST:
        return std::unique_ptr<CachePacket>(new ClearRequestPacket());
    case PACKET_CLEAR_RESPONSE:
        return std::unique_ptr<CachePacket>(new ClearResponsePacket());
    default:
    {
        Logger::Error("[CachePacket::%s] Wrong packet type: %d", __FUNCTION__, type);
        break;
    }
    }

    return nullptr;
}

CachePacket::CachePacket(ePacketID type_, bool createBuffer)
    : type(type_)
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

AddRequestPacket::AddRequestPacket(const CacheItemKey& key_, const CachedItemValue& value_)
    : CachePacket(PACKET_ADD_REQUEST, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(key_.data(), static_cast<uint32>(key_.size()));
    value_.Serialize(serializationBuffer);
}

bool AddRequestPacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return ((file->Read(key.data(), keySize) == keySize) && value.Deserialize(file));
}

AddResponsePacket::AddResponsePacket(const CacheItemKey& key_, bool added_)
    : CachePacket(PACKET_ADD_RESPONSE, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(key_.data(), static_cast<uint32>(key_.size()));
    serializationBuffer->Write(&added_, sizeof(added_));
}

bool AddResponsePacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return ((file->Read(key.data(), keySize) == keySize) && (file->Read(&added) == sizeof(added)));
}

GetRequestPacket::GetRequestPacket(const CacheItemKey& key_)
    : CachePacket(PACKET_GET_REQUEST, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(key_.data(), static_cast<uint32>(key_.size()));
}

bool GetRequestPacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return (file->Read(key.data(), keySize) == keySize);
}

GetResponsePacket::GetResponsePacket(const CacheItemKey& key_, const CachedItemValue& value_)
    : CachePacket(PACKET_GET_RESPONSE, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(key_.data(), static_cast<uint32>(key_.size()));
    value_.Serialize(serializationBuffer);
}

bool GetResponsePacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return ((file->Read(key.data(), keySize) == keySize) && value.Deserialize(file));
}

WarmupRequestPacket::WarmupRequestPacket(const CacheItemKey& key_)
    : CachePacket(PACKET_WARMING_UP_REQUEST, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(key_.data(), static_cast<uint32>(key_.size()));
}

bool WarmupRequestPacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return (file->Read(key.data(), keySize) == keySize);
}

RemoveRequestPacket::RemoveRequestPacket(const CacheItemKey& key_)
    : CachePacket(PACKET_REMOVE_REQUEST, true)
{
    WriteHeader(serializationBuffer);
    serializationBuffer->Write(key_.data(), static_cast<uint32>(key_.size()));
}

bool RemoveRequestPacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return (file->Read(key.data(), keySize) == keySize);
}

RemoveResponsePacket::RemoveResponsePacket(const CacheItemKey& key_, bool removed_)
    : CachePacket(PACKET_REMOVE_RESPONSE, true)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(key_.data(), static_cast<uint32>(key_.size()));
    serializationBuffer->Write(&removed_, sizeof(removed_));
}

bool RemoveResponsePacket::Load(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return ((file->Read(key.data(), keySize) == keySize) && (file->Read(&removed) == sizeof(removed)));
}

ClearResponsePacket::ClearResponsePacket(bool cleared_)
    : CachePacket(PACKET_CLEAR_RESPONSE, true)
{
    WriteHeader(serializationBuffer);
    serializationBuffer->Write(&cleared_, sizeof(cleared_));
}

bool ClearResponsePacket::Load(File* file)
{
    return ((file->Read(&cleared) == sizeof(cleared)));
}

} //AssetCache
} //DAVA
