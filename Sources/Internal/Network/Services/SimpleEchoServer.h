#ifndef __DAVAENGINE_SIMPLEECHOSERVER_H__
#define __DAVAENGINE_SIMPLEECHOSERVER_H__

#include <Network/NetService.h>

namespace DAVA
{
namespace Net
{
/*
 This is a simple echo service: each recieved packet is sent back
*/
class SimpleEchoServer : public NetService
{
public:
    virtual void OnPacketReceived(IChannel* channel, const void* buffer, size_t length)
    {
        uint8* p = new uint8[length];
        Memcpy(p, buffer, length);

        buffers.push_back(std::make_pair(p, length));
        Send(p, length);
    }

    virtual void PacketSent()
    {
        std::pair<const uint8*, size_t> item = buffers.front();
        buffers.pop_front();
        delete[] item.first;
    }

private:
    Deque<std::pair<const uint8*, size_t>> buffers;
};

} // namespace Net
} // namespace DAVA

#endif // __DAVAENGINE_SIMPLEECHOSERVER_H__
