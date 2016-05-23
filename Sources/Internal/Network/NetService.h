#ifndef __DAVAENGINE_NETSERVICE_H__
#define __DAVAENGINE_NETSERVICE_H__

#include "Base/BaseTypes.h"
#include "Network/IChannel.h"

namespace DAVA
{
namespace Net
{
class NetService : public IChannelListener
{
public:
    NetService();
    virtual ~NetService()
    {
    }

    // IChannelListener
    void OnChannelOpen(IChannel* aChannel) override;
    void OnChannelClosed(IChannel* aChannel, const char8* message) override;
    void OnPacketReceived(IChannel* aChannel, const void* buffer, size_t length) override;
    void OnPacketSent(IChannel* aChannel, const void* buffer, size_t length) override;
    void OnPacketDelivered(IChannel* aChannel, uint32 packetId) override;

    virtual void ChannelOpen()
    {
    }
    virtual void ChannelClosed(const char8* message)
    {
    }
    virtual void PacketReceived(const void* packet, size_t length)
    {
    }
    virtual void PacketSent()
    {
    }
    virtual void PacketDelivered()
    {
    }

    bool IsChannelOpen() const;

protected:
    bool Send(const void* data, size_t length, uint32* packetId = NULL);
    template <typename T>
    bool Send(const T* value, uint32* packetId = NULL);

protected:
    IChannel* channel;
};

//////////////////////////////////////////////////////////////////////////
inline NetService::NetService()
    : channel(NULL)
{
}

inline bool NetService::IsChannelOpen() const
{
    return channel != NULL;
}

template <typename T>
inline bool NetService::Send(const T* value, uint32* packetId)
{
    return Send(value, sizeof(T), packetId);
}

} // namespace Net
} // namespace DAVA

#endif // __DAVAENGINE_NETSERVICE_H__
