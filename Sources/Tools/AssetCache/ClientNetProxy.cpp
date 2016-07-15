#include "AssetCache/ClientNetProxy.h"
#include "AssetCache/AssetCacheConstants.h"
#include "AssetCache/CachedItemValue.h"
#include "AssetCache/CachePacket.h"
#include "FileSystem/KeyedArchive.h"
#include "Debug/DVAssert.h"
#include "FileSystem/DynamicMemoryFile.h"

namespace DAVA
{
namespace AssetCache
{
ClientNetProxy::ClientNetProxy()
    : addressResolver(Net::NetCore::Instance()->Loop())
{
    DVASSERT(nullptr != Net::NetCore::Instance());
}

ClientNetProxy::~ClientNetProxy()
{
    Disconnect();
}

bool ClientNetProxy::Connect(const String& ip, uint16 port)
{
    Logger::Debug("Connecting to %s:%d", ip.c_str(), port);
    DVASSERT(nullptr == netClient);
    DVASSERT(nullptr == openedChannel);

    return addressResolver.AsyncResolve(ip.c_str(), port, MakeFunction(this, &ClientNetProxy::OnAddressResolved));
}

void ClientNetProxy::Disconnect()
{
    Logger::Debug("Disconnect");
    openedChannel = nullptr;

    addressResolver.Cancel();
    netClient.reset();
}

void ClientNetProxy::OnAddressResolved(const Net::Endpoint& endpoint, int32 status)
{
    DVASSERT(!netClient);
    DVASSERT(nullptr == openedChannel);

    if (0 == status)
    {
        netClient.reset(new Connection(Net::CLIENT_ROLE, endpoint, this));
    }
    else
    {
        Logger::Error("[ClientNetProxy::OnAddressResolved] address cannot resolved with error %d", status);
    }
}

bool ClientNetProxy::RequestServerStatus()
{
    if (openedChannel)
    {
        Logger::Debug("Requesting cache server status");
        StatusRequestPacket packet;
        return packet.SendTo(openedChannel);
    }

    return false;
}

bool ClientNetProxy::RequestAddData(const CacheItemKey& key, const CachedItemValue& value)
{
    if (openedChannel)
    {
        Logger::Debug("Requesting to add data");
        AddRequestPacket packet(key, value);
        return packet.SendTo(openedChannel);
    }

    return false;
}

bool ClientNetProxy::RequestData(const CacheItemKey& key)
{
    Logger::Debug("Requesting data");
    if (openedChannel)
    {
        GetRequestPacket packet(key);
        return packet.SendTo(openedChannel);
    }

    return false;
}

bool ClientNetProxy::RequestWarmingUp(const CacheItemKey& key)
{
    Logger::Debug("Requesting warmup");
    if (openedChannel)
    {
        WarmupRequestPacket packet(key);
        return packet.SendTo(openedChannel);
    }

    return false;
}

bool ClientNetProxy::RequestRemoveData(const CacheItemKey& key)
{
    Logger::Debug("Requesting to remove data entry");
    if (openedChannel)
    {
        RemoveRequestPacket packet(key);
        return packet.SendTo(openedChannel);
    }

    return false;
}

bool ClientNetProxy::RequestClearCache()
{
    Logger::Debug("Requesting to clear cache");
    if (openedChannel)
    {
        ClearRequestPacket packet;
        return packet.SendTo(openedChannel);
    }

    return false;
}

void ClientNetProxy::OnChannelOpen(DAVA::Net::IChannel* channel)
{
    Logger::Debug("Connection established");
    DVASSERT(openedChannel == nullptr);
    openedChannel = channel;
    StateChanged();
}

void ClientNetProxy::OnChannelClosed(DAVA::Net::IChannel* channel, const char8*)
{
    Logger::Debug("Connection closed");
    DVASSERT(openedChannel == channel);
    openedChannel = nullptr;
    StateChanged();
}

void ClientNetProxy::StateChanged()
{
    for (auto& listener : listeners)
    {
        listener->OnClientProxyStateChanged();
    }
}

void ClientNetProxy::OnPacketReceived(DAVA::Net::IChannel* channel, const void* packetData, size_t length)
{
    if (listeners.empty())
    { // do not need to process data in case of nullptr listener
        return;
    }

    IncorrectPacketType incorrectType = IncorrectPacketType::UNDEFINED_DATA;

    DVASSERT(openedChannel == channel);
    if (length > 0)
    {
        std::unique_ptr<CachePacket> packet;
        CachePacket::CreateResult createResult = CachePacket::Create(static_cast<const uint8*>(packetData), static_cast<uint32>(length), packet);

        if (createResult == CachePacket::CREATED)
        {
            DVASSERT(packet);

            switch (packet->type)
            {
            case PACKET_ADD_RESPONSE:
            {
                AddResponsePacket* p = static_cast<AddResponsePacket*>(packet.get());
                Logger::Debug("Response is received: data %s added to cache", p->added ? "is" : "is not");
                for (ClientNetProxyListener* listener : listeners)
                    listener->OnAddedToCache(p->key, p->added);
                return;
            }
            case PACKET_GET_RESPONSE:
            {
                GetResponsePacket* p = static_cast<GetResponsePacket*>(packet.get());
                Logger::Debug("Response is received: data %s received from cache", p->value.IsEmpty() ? "is not" : "is");
                for (ClientNetProxyListener* listener : listeners)
                    listener->OnReceivedFromCache(p->key, std::forward<CachedItemValue>(p->value));
                return;
            }
            case PACKET_STATUS_RESPONSE:
            {
                StatusResponsePacket* p = static_cast<StatusResponsePacket*>(packet.get());
                Logger::Debug("Response is received: server status is OK");
                for (ClientNetProxyListener* listener : listeners)
                    listener->OnServerStatusReceived();
                return;
            }
            case PACKET_REMOVE_RESPONSE:
            {
                RemoveResponsePacket* p = static_cast<RemoveResponsePacket*>(packet.get());
                Logger::Debug("Response is received: data %s removed from cache", p->removed ? "is" : "is not");
                for (ClientNetProxyListener* listener : listeners)
                    listener->OnRemovedFromCache(p->key, p->removed);
                return;
            }
            case PACKET_CLEAR_RESPONSE:
            {
                ClearResponsePacket* p = static_cast<ClearResponsePacket*>(packet.get());
                Logger::Debug("Response is received: cache %s cleared", p->cleared ? "is" : "is not");
                for (ClientNetProxyListener* listener : listeners)
                    listener->OnCacheCleared(p->cleared);
                return;
            }
            default:
            {
                Logger::Error("[AssetCache::ClientNetProxy::%s] Unexpected packet type: %d", __FUNCTION__, packet->type);
                incorrectType = IncorrectPacketType::UNEXPECTED_PACKET;
                break;
            }
            }
        }
        else
        {
            incorrectType = (createResult == CachePacket::ERR_UNSUPPORTED_VERSION) ? IncorrectPacketType::UNSUPPORTED_VERSION : IncorrectPacketType::UNDEFINED_DATA;
        }
    }
    else
    {
        Logger::Error("[AssetCache::Client::%s] Empty packet is received", __FUNCTION__);
        incorrectType = IncorrectPacketType::UNDEFINED_DATA;
    }

    for (ClientNetProxyListener* listener : listeners)
        listener->OnIncorrectPacketReceived(incorrectType);
}

void ClientNetProxy::OnPacketSent(Net::IChannel* channel, const void* buffer, size_t length)
{
    CachePacket::PacketSent(static_cast<const uint8*>(buffer), length);
}

void ClientNetProxy::AddListener(ClientNetProxyListener* listener)
{
    DVASSERT(listener != nullptr);
    listeners.insert(listener);
}

void ClientNetProxy::RemoveListener(ClientNetProxyListener* listener)
{
    DVASSERT(listener != nullptr);
    listeners.erase(listener);
}

} // end of namespace AssetCache
} // end of namespace DAVA
