#pragma once

#include "Tools/AssetCache/CacheItemKey.h"
#include "Tools/AssetCache/CachedItemValue.h"
#include "Tools/AssetCache/AssetCacheConstants.h"

#include <FileSystem/DynamicMemoryFile.h>

#include <memory>

namespace DAVA
{
namespace Net
{
struct IChannel;
}

namespace AssetCache
{

#pragma pack(push, 1) // exact fit - no padding
struct CachePacketHeader
{
    CachePacketHeader(){};
    CachePacketHeader(uint16 header, uint8 _version, ePacketID type)
        : headerID(header)
        , version(_version)
        , packetType(type)
    {
    }

    uint16 headerID = 0;
    uint8 version = 0;
    ePacketID packetType = PACKET_UNKNOWN;
};
#pragma pack(pop) //back to whatever the previous packing mode was

class CachePacket
{
public:
    virtual ~CachePacket(){}; // this is base class for asset cache network packets

    enum CreateResult
    {
        CREATED,
        ERR_UNSUPPORTED_VERSION,
        ERR_INCORRECT_DATA
    };
    static CreateResult Create(const uint8* buffer, uint32 length, std::unique_ptr<CachePacket>& packet);

    bool SendTo(Net::IChannel* channel);
    static void PacketSent(const uint8* buffer, size_t length);

protected:
    CachePacket(ePacketID type, bool createBuffer);

    static std::unique_ptr<CachePacket> CreateByType(ePacketID type);

    void WriteHeader(File* file) const;
    virtual bool Load(File* file) = 0;

public:
    ePacketID type = PACKET_UNKNOWN;
    ScopedPtr<DynamicMemoryFile> serializationBuffer;

private:
    static Map<const uint8*, ScopedPtr<DynamicMemoryFile>> sendingPackets;
};

class AddRequestPacket : public CachePacket
{
public:
    AddRequestPacket()
        : CachePacket(PACKET_ADD_REQUEST, false)
    {
    }
    AddRequestPacket(const CacheItemKey& key, const CachedItemValue& value);

protected:
    bool Load(File* file) override;

public:
    CacheItemKey key;
    CachedItemValue value;
};

class AddResponsePacket : public CachePacket
{
public:
    AddResponsePacket()
        : CachePacket(PACKET_ADD_RESPONSE, false)
    {
    }
    AddResponsePacket(const CacheItemKey& key, bool added);

protected:
    bool Load(File* file) override;

public:
    CacheItemKey key;
    bool added = false;
};

class GetRequestPacket : public CachePacket
{
public:
    GetRequestPacket()
        : CachePacket(PACKET_GET_REQUEST, false)
    {
    }
    GetRequestPacket(const CacheItemKey& key);

protected:
    bool Load(File* file) override;

public:
    CacheItemKey key;
};

class GetResponsePacket : public CachePacket
{
public:
    GetResponsePacket()
        : CachePacket(PACKET_GET_RESPONSE, false)
    {
    }
    GetResponsePacket(const CacheItemKey& key, const CachedItemValue& value);

protected:
    bool Load(File* file) override;

public:
    CacheItemKey key;
    CachedItemValue value;
};

class WarmupRequestPacket : public CachePacket
{
public:
    WarmupRequestPacket()
        : CachePacket(PACKET_WARMING_UP_REQUEST, false)
    {
    }
    WarmupRequestPacket(const CacheItemKey& key);

protected:
    bool Load(File* file) override;

public:
    CacheItemKey key;
};

struct StatusRequestPacket : public CachePacket
{
    StatusRequestPacket()
        : CachePacket(PACKET_STATUS_REQUEST, true)
    {
        WriteHeader(serializationBuffer);
    }

protected:
    bool Load(File* file) override
    {
        return true;
    }
};

struct StatusResponsePacket : public CachePacket
{
    StatusResponsePacket()
        : CachePacket(PACKET_STATUS_RESPONSE, true)
    {
        WriteHeader(serializationBuffer);
    }

protected:
    bool Load(File* file) override
    {
        return true;
    }
};

struct RemoveRequestPacket : public CachePacket
{
    RemoveRequestPacket()
        : CachePacket(PACKET_REMOVE_REQUEST, false)
    {
    }
    RemoveRequestPacket(const CacheItemKey& key);

protected:
    bool Load(File* file) override;

public:
    CacheItemKey key;
};

class RemoveResponsePacket : public CachePacket
{
public:
    RemoveResponsePacket()
        : CachePacket(PACKET_REMOVE_RESPONSE, false)
    {
    }
    RemoveResponsePacket(const CacheItemKey& key, bool removed);

protected:
    bool Load(File* file) override;

public:
    CacheItemKey key;
    bool removed = false;
};

struct ClearRequestPacket : public CachePacket
{
    ClearRequestPacket()
        : CachePacket(PACKET_CLEAR_REQUEST, true)
    {
        WriteHeader(serializationBuffer);
    }

protected:
    bool Load(File* file) override
    {
        return true;
    }
};

struct ClearResponsePacket : public CachePacket
{
    ClearResponsePacket()
        : CachePacket(PACKET_CLEAR_RESPONSE, false)
    {
    }
    ClearResponsePacket(bool cleared);

protected:
    bool Load(File* file) override;

public:
    bool cleared = false;
};

} // end of namespace AssetCache
} // end of namespace DAVA
