#pragma once

#include "Network/IChannel.h"

namespace DAVA
{
namespace Net
{
class NetCallbacksHolder;

class ChannelListenerAsync : public IChannelListener
{
public:
    explicit ChannelListenerAsync(IChannelListener* listener, NetCallbacksHolder* callbacksHolder);

    void OnChannelOpen(IChannel* channel) override;
    void OnChannelClosed(IChannel* channel, const char8* message) override;
    void OnPacketReceived(IChannel* channel, const void* buffer, size_t length) override;
    void OnPacketSent(IChannel* channel, const void* buffer, size_t length) override;
    void OnPacketDelivered(IChannel* channel, uint32 packetId) override;

private:
    NetCallbacksHolder* callbacksHolder;
    std::shared_ptr<IChannelListener> listenerShared;
    std::weak_ptr<IChannelListener> listenerWeak;
};
}
}
