#ifndef __DAVAENGINE_ASSETCACHE_CONNECTION_H__
#define __DAVAENGINE_ASSETCACHE_CONNECTION_H__

#include "Network/Base/Endpoint.h"
#include "Network/NetworkCommon.h"
#include "Network/NetCore.h"
#include "Network/IChannel.h"

namespace DAVA
{
namespace Net
{
struct IChannel;
}

class KeyedArchive;
namespace AssetCache
{
bool SendArchieve(Net::IChannel* channel, KeyedArchive* archieve);

class Connection final : public Net::IChannelListener
{
public:
    Connection(Net::eNetworkRole role, const Net::Endpoint& endpoint, Net::IChannelListener* listener, Net::eTransportType transport = Net::TRANSPORT_TCP);
    ~Connection();

    const Net::Endpoint& GetEndpoint() const;

    // IChannelListener
    void OnChannelOpen(Net::IChannel* channel) override;
    void OnChannelClosed(Net::IChannel* channel, const char8* message) override;
    void OnPacketReceived(Net::IChannel* channel, const void* buffer, size_t length) override;
    void OnPacketSent(Net::IChannel* channel, const void* buffer, size_t length) override;
    void OnPacketDelivered(Net::IChannel* channel, uint32 packetId) override;

private:
    bool Connect(Net::eNetworkRole _role, Net::eTransportType transport);
    void DisconnectBlocked();

    static Net::IChannelListener* Create(uint32 serviceId, void* context);
    static void Delete(Net::IChannelListener* obj, void* context);

private:
    Net::Endpoint endpoint;
    Net::NetCore::TrackId controllerId = Net::NetCore::INVALID_TRACK_ID;

    Net::IChannelListener* listener = nullptr;
};

inline const Net::Endpoint& Connection::GetEndpoint() const
{
    return endpoint;
}

} // end of namespace AssetCache
} // end of namespace DAVA

#endif // __DAVAENGINE_ASSETCACHE_CONNECTION_H__
