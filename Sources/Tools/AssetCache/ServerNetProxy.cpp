#include "AssetCache/ServerNetProxy.h"
#include "AssetCache/AssetCacheConstants.h"
#include "AssetCache/CachedItemValue.h"
#include "AssetCache/CachePacket.h"
#include "Debug/DVAssert.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/DynamicMemoryFile.h"

namespace DAVA
{
namespace AssetCache
{
ServerNetProxy::~ServerNetProxy()
{
}

void ServerNetProxy::Listen(uint16 port)
{
    listenPort = port;
    DVASSERT(!netServer);

    netServer.reset(new Connection(Net::SERVER_ROLE, Net::Endpoint(listenPort), this));
}

void ServerNetProxy::Disconnect()
{
    netServer.reset();
}

void ServerNetProxy::OnPacketReceived(Net::IChannel* channel, const void* packetData, size_t length)
{
    if (nullptr == delegate)
    { // do not need to process data in case of nullptr delegate
        return;
    }

    if (length > 0)
    {
        std::unique_ptr<CachePacket> packet =
        CachePacket::Create(static_cast<const uint8*>(packetData), static_cast<uint32>(length));

        if (packet)
        {
            switch (packet->type)
            {
            case PACKET_ADD_REQUEST:
            {
                AddRequestPacket* p = static_cast<AddRequestPacket*>(packet.get());
                delegate->OnAddToCache(channel, p->key, std::forward<CachedItemValue>(p->value));
                break;
            }
            case PACKET_GET_REQUEST:
            {
                GetRequestPacket* p = static_cast<GetRequestPacket*>(packet.get());
                delegate->OnRequestedFromCache(channel, p->key);
                break;
            }
            case PACKET_WARMING_UP_REQUEST:
            {
                WarmupRequestPacket* p = static_cast<WarmupRequestPacket*>(packet.get());
                delegate->OnWarmingUp(channel, p->key);
                break;
            }
            default:
            {
                Logger::Error("[AssetCache::ServerNetProxy::%s] Unexpected packet type: (%d). Closing channel", __FUNCTION__, packet->type);
                DVASSERT(false);
                break;
            }
            }
        }
        else
        {
            DVASSERT(false && "Invalid packet received");
        }
    }
    else
    {
        Logger::Error("[AssetCache::ServerNetProxy::%s] Empty packet is received.", __FUNCTION__);
    }
}

void ServerNetProxy::OnPacketSent(Net::IChannel* channel, const void* buffer, size_t length)
{
    CachePacket::PacketSent(static_cast<const uint8*>(buffer), length);
}

void ServerNetProxy::OnChannelClosed(Net::IChannel* channel, const char8* message)
{
    if (delegate)
    {
        delegate->OnChannelClosed(channel, message);
    }
}

bool ServerNetProxy::AddedToCache(Net::IChannel* channel, const CacheItemKey& key, bool added)
{
    if (channel)
    {
        AddResponsePacket packet(key, added);
        return packet.SendTo(channel);
    }

    return false;
}

bool ServerNetProxy::Send(Net::IChannel* channel, const CacheItemKey& key, const CachedItemValue& value)
{
    if (channel)
    {
        GetResponsePacket packet(key, value);
        return packet.SendTo(channel);
    }

    return false;
}

}; // end of namespace AssetCache
}; // end of namespace DAVA
