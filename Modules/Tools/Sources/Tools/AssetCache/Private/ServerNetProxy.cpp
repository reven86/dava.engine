#include "Tools/AssetCache/ServerNetProxy.h"
#include "Tools/AssetCache/AssetCacheConstants.h"
#include "Tools/AssetCache/CachedItemValue.h"
#include "Tools/AssetCache/CachePacket.h"

#include <Debug/DVAssert.h>
#include <Logger/Logger.h>
#include <FileSystem/KeyedArchive.h>
#include <FileSystem/DynamicMemoryFile.h>

namespace DAVA
{
namespace AssetCache
{
// timeout of waiting for response from client.
const uint32 CLIENT_PING_TIMEOUT_MS = 1 * 1000;

ServerNetProxy::ServerNetProxy(Dispatcher<Function<void()>>* dispatcher)
    : dispatcher(dispatcher)
{
}

ServerNetProxy::~ServerNetProxy()
{
}

void ServerNetProxy::Listen(uint16 port)
{
    listenPort = port;
    DVASSERT(!netServer);

    netServer = Connection::MakeConnection(dispatcher, Net::SERVER_ROLE, Net::Endpoint(listenPort), this, Net::TRANSPORT_TCP, CLIENT_PING_TIMEOUT_MS);
}

void ServerNetProxy::Disconnect()
{
    netServer.reset();
}

void ServerNetProxy::OnPacketReceived(Net::IChannel* channel, const void* packetData, size_t length)
{
    if (nullptr == listener)
    { // do not need to process data in case of nullptr delegate
        return;
    }

    if (length > 0)
    {
        std::unique_ptr<CachePacket> packet;
        CachePacket::CreateResult createResult = CachePacket::Create(static_cast<const uint8*>(packetData), static_cast<uint32>(length), packet);

        if (createResult == CachePacket::CREATED)
        {
            DVASSERT(packet);

            switch (packet->type)
            {
            case PACKET_ADD_REQUEST:
            {
                AddRequestPacket* p = static_cast<AddRequestPacket*>(packet.get());
                listener->OnAddToCache(channel, p->key, std::forward<CachedItemValue>(p->value));
                return;
            }
            case PACKET_GET_REQUEST:
            {
                GetRequestPacket* p = static_cast<GetRequestPacket*>(packet.get());
                listener->OnRequestedFromCache(channel, p->key);
                return;
            }
            case PACKET_REMOVE_REQUEST:
            {
                RemoveRequestPacket* p = static_cast<RemoveRequestPacket*>(packet.get());
                listener->OnRemoveFromCache(channel, p->key);
                return;
            }
            case PACKET_CLEAR_REQUEST:
            {
                listener->OnClearCache(channel);
                return;
            }
            case PACKET_WARMING_UP_REQUEST:
            {
                WarmupRequestPacket* p = static_cast<WarmupRequestPacket*>(packet.get());
                listener->OnWarmingUp(channel, p->key);
                return;
            }
            case PACKET_STATUS_REQUEST:
            {
                listener->OnStatusRequested(channel);
                return;
            }
            default:
            {
                Logger::Error("[AssetCache::ServerNetProxy::%s] Unexpected packet type: %d", __FUNCTION__, packet->type);
                break;
            }
            }
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
    if (listener)
    {
        listener->OnChannelClosed(channel, message);
    }
}

bool ServerNetProxy::SendAddedToCache(Net::IChannel* channel, const CacheItemKey& key, bool added)
{
    if (channel)
    {
        AddResponsePacket packet(key, added);
        return packet.SendTo(channel);
    }

    return false;
}

bool ServerNetProxy::SendRemovedFromCache(Net::IChannel* channel, const CacheItemKey& key, bool removed)
{
    if (channel)
    {
        RemoveResponsePacket packet(key, removed);
        return packet.SendTo(channel);
    }

    return false;
}

bool ServerNetProxy::SendCleared(Net::IChannel* channel, bool cleared)
{
    if (channel)
    {
        ClearResponsePacket packet(cleared);
        return packet.SendTo(channel);
    }

    return false;
}

bool ServerNetProxy::SendData(Net::IChannel* channel, const CacheItemKey& key, const CachedItemValue& value)
{
    if (channel)
    {
        GetResponsePacket packet(key, value);
        return packet.SendTo(channel);
    }

    return false;
}

bool ServerNetProxy::SendStatus(Net::IChannel* channel)
{
    if (channel)
    {
        StatusResponsePacket packet;
        return packet.SendTo(channel);
    }

    return false;
}

}; // end of namespace AssetCache
}; // end of namespace DAVA
