#ifndef __DAVAENGINE_ASSET_CACHE_CLIENT_H__
#define __DAVAENGINE_ASSET_CACHE_CLIENT_H__

#include "Base/BaseTypes.h"

#include "Network/IChannel.h"

#include "AssetCache/Connection.h"
#include "AssetCache/CacheItemKey.h"
#include "Network/Base/AddressResolver.h"

namespace DAVA
{
namespace AssetCache
{
class CachedItemValue;

class ClientNetProxyListener
{
public:
    virtual ~ClientNetProxyListener() = default;

    virtual void OnAssetClientStateChanged(){};
    virtual void OnAddedToCache(const CacheItemKey& key, bool added){};
    virtual void OnReceivedFromCache(const CacheItemKey& key, const CachedItemValue& value){};
};

class ClientNetProxy : public DAVA::Net::IChannelListener
{
public:
    ClientNetProxy();
    ~ClientNetProxy();

    void AddListener(ClientNetProxyListener*);
    void RemoveListener(ClientNetProxyListener*);

    bool Connect(const String& ip, uint16 port);
    void Disconnect();

    bool ChannelIsOpened() const;

    bool AddToCache(const CacheItemKey& key, const CachedItemValue& value);
    bool RequestFromCache(const CacheItemKey& key);
    bool WarmingUp(const CacheItemKey& key);

    Connection* GetConnection() const;

    //Net::IChannelListener
    // Channel is open (underlying transport has connection) and can receive and send data through IChannel interface
    void OnChannelOpen(Net::IChannel* channel) override;
    // Channel is closed (underlying transport has disconnected) with reason
    void OnChannelClosed(Net::IChannel* channel, const char8* message) override;
    // Some data arrived into channel
    void OnPacketReceived(Net::IChannel* channel, const void* buffer, size_t length) override;
    // Buffer has been sent and can be reused or freed
    void OnPacketSent(Net::IChannel* channel, const void* buffer, size_t length) override;
    // Data packet with given ID has been delivered to other side
    void OnPacketDelivered(Net::IChannel* channel, uint32 packetId) override{};

    void OnAddressResolved(const Net::Endpoint& endpoint, int32 status);

    void StateChanged();

private:
    Net::AddressResolver addressResolver;
    std::unique_ptr<Connection> netClient;
    DAVA::Net::IChannel* openedChannel = nullptr;

    Set<ClientNetProxyListener*> listeners;
};

inline bool ClientNetProxy::ChannelIsOpened() const
{
    return (openedChannel != nullptr);
}

inline Connection* ClientNetProxy::GetConnection() const
{
    return netClient.get();
}

}; // end of namespace AssetCache
}; // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_CLIENT_H__
