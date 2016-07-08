#ifndef __DAVAENGINE_DISCOVERER_H__
#define __DAVAENGINE_DISCOVERER_H__

#include <Network/Base/UDPSocket.h>
#include <Network/Base/DeadlineTimer.h>

#include "Network/Base/TCPSocket.h"

#include <Network/IController.h>

namespace DAVA
{
namespace Net
{
class IOLoop;

class Discoverer : public IController
{
    static const uint32 RESTART_DELAY_PERIOD = 3000;

public:
    Discoverer(IOLoop* ioLoop, const Endpoint& endp, Function<void(size_t, const void*, const Endpoint&)> dataReadyCallback);
    virtual ~Discoverer();

    // IController
    void Start() override;
    void Stop(Function<void(IController*)> callback) override;
    void Restart() override;

    bool TryDiscoverDevice(const Endpoint& endpoint);

private:
    void DoStart();
    void DoStop();
    void DoObjectClose();
    void DoBye();
    void DiscoverDevice();

    void SocketHandleReceive(UDPSocket* socket, int32 error, size_t nread, const Endpoint& endpoint, bool partial);

    void TcpSocketHandleRead(TCPSocket* socket, int32 error, size_t nread);

private:
    IOLoop* loop;
    UDPSocket socket;
    DeadlineTimer timer;
    Endpoint endpoint;
    Array<char8, 30> endpAsString;
    bool isTerminating;
    size_t runningObjects;
    Function<void(IController*)> stopCallback;
    Function<void(size_t, const void*, const Endpoint&)> dataCallback;
    uint8 inbuf[4 * 1024];

    Endpoint tcpEndpoint; // IP address of remote announcer
    TCPSocket tcpSocket; // TCP socket for direct connection to remote announcer
    char tcpInbuf[4 * 1024];
};

} // namespace Net
} // namespace DAVA

#endif // __DAVAENGINE_DISCOVERER_H__
