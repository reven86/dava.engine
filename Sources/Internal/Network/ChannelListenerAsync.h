#pragma once

#include "Network/IChannel.h"
#include "Network/NetEventsDispatcher.h"

namespace DAVA
{
namespace Net
{
class ChannelListenerAsync : public IChannelListener
{
public:
    explicit ChannelListenerAsync(IChannelListener* listener, NetEventsDispatcher* netEventsDispatcher);

    void OnChannelOpen(IChannel* channel) override;
    void OnChannelClosed(IChannel* channel, const char8* message) override;
    void OnPacketReceived(IChannel* channel, const void* buffer, size_t length) override;
    void OnPacketSent(IChannel* channel, const void* buffer, size_t length) override;
    void OnPacketDelivered(IChannel* channel, uint32 packetId) override;

private:
    NetEventsDispatcher* netEventsDispatcher = nullptr;
    std::shared_ptr<IChannelListener> listenerShared;
    std::weak_ptr<IChannelListener> listenerWeak;
};
}
}
