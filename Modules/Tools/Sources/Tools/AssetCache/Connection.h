#pragma once

#include "Tools/NetworkHelpers/ChannelListenerDispatched.h"

#include <Network/Base/Endpoint.h>
#include <Network/NetworkCommon.h>
#include <Network/NetCore.h>
#include <Network/IChannel.h>

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

class Connection final : public Net::IChannelListener, public std::enable_shared_from_this<Connection>
{
public:
    static std::shared_ptr<Connection> MakeConnection(
    Dispatcher<Function<void()>>* dispatcher,
    Net::eNetworkRole role,
    const Net::Endpoint& endpoint,
    Net::IChannelListener* listener,
    Net::eTransportType transport = Net::TRANSPORT_TCP,
    uint32 timeoutMs = 5 * 1000);

    ~Connection();

    void Disconnect();
    void DisconnectBlocked();

    const Net::Endpoint& GetEndpoint() const;

    // IChannelListener
    void OnChannelOpen(Net::IChannel* channel) override;
    void OnChannelClosed(Net::IChannel* channel, const char8* message) override;
    void OnPacketReceived(Net::IChannel* channel, const void* buffer, size_t length) override;
    void OnPacketSent(Net::IChannel* channel, const void* buffer, size_t length) override;
    void OnPacketDelivered(Net::IChannel* channel, uint32 packetId) override;

private:
    Connection(Dispatcher<Function<void()>>* dispatcher, Net::eNetworkRole role, const Net::Endpoint& endpoint, Net::IChannelListener* listener, Net::eTransportType transport, uint32 timeoutMs);
    bool Connect(Net::eNetworkRole _role, Net::eTransportType transport, uint32 timeoutMs);

    static Net::IChannelListener* Create(uint32 serviceId, void* context);
    static void Delete(Net::IChannelListener* obj, void* context);

private:
    Dispatcher<Function<void()>>* dispatcher = nullptr;

    Net::Endpoint endpoint;
    Net::NetCore::TrackId controllerId = Net::NetCore::INVALID_TRACK_ID;

    Net::IChannelListener* listener = nullptr;
    std::shared_ptr<Net::ChannelListenerDispatched> channelListenerDispatched;
};

inline const Net::Endpoint& Connection::GetEndpoint() const
{
    return endpoint;
}

} // end of namespace AssetCache
} // end of namespace DAVA
