#include <Debug/DVAssert.h>

#include "NetService.h"

namespace DAVA
{
namespace Net
{
IChannelListener::~IChannelListener() = default;
IChannel::~IChannel() = default;

void NetService::OnChannelOpen(IChannel* aChannel)
{
    DVASSERT(NULL == channel);
    channel = aChannel;
    ChannelOpen();
}

void NetService::OnChannelClosed(IChannel* aChannel, const char8* message)
{
    // OnChannelClosed can be called without corresponding OnChannelOpen, e.g. when remote service is unavailable
    DVASSERT(NULL == channel || channel == aChannel);
    channel = NULL;
    ChannelClosed(message);
}

void NetService::OnPacketReceived(IChannel* aChannel, const void* buffer, size_t length)
{
    DVASSERT(channel == aChannel);
    PacketReceived(buffer, length);
}

void NetService::OnPacketSent(IChannel* aChannel, const void* buffer, size_t length)
{
    // If channel is NULL then OnChannelClosed has been called already
    DVASSERT(NULL == channel || channel == aChannel);
    PacketSent();
}

void NetService::OnPacketDelivered(IChannel* aChannel, uint32 packetId)
{
    DVASSERT(channel == aChannel);
    PacketDelivered();
}

bool NetService::Send(const void* data, size_t length, uint32* packetId)
{
    DVASSERT(data != NULL && length > 0 && true == IsChannelOpen());
    return IsChannelOpen() ? channel->Send(data, length, 0, packetId)
                             :
                             false;
}

} // namespace Net
} // namespace DAVA
