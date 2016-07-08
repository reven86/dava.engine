#ifndef __DAVAENGINE_ANNOUNCER_H__
#define __DAVAENGINE_ANNOUNCER_H__

#include <Network/Base/DeadlineTimer.h>
#include <Network/Base/UDPSocket.h>

#include "Network/Base/TCPAcceptor.h"
#include "Network/Base/TCPSocket.h"

#include <Network/IController.h>

namespace DAVA
{
namespace Net
{
class IOLoop;

class Announcer : public IController
{
    static const uint32 RESTART_DELAY_PERIOD = 3000;

public:
    Announcer(IOLoop* ioLoop, const Endpoint& endp, uint32 sendPeriod, Function<size_t(size_t, void*)> needDataCallback, const Endpoint& tcpEndp);
    virtual ~Announcer();

    // IController
    void Start() override;
    void Stop(Function<void(IController*)> callback) override;
    void Restart() override;

private:
    void DoStart();
    void DoStop();
    void DoObjectClose();
    void DoBye();

    void TimerHandleTimer(DeadlineTimer* timer);

    void SocketHandleSend(UDPSocket* socket, int32 error, const Buffer* buffers, size_t bufferCount);

    void AcceptorHandleConnect(TCPAcceptor* acceptor, int32 error);

private:
    IOLoop* loop;
    UDPSocket socket;
    DeadlineTimer timer;
    Endpoint endpoint;
    Array<char8, 30> endpAsString;
    uint32 announcePeriod;
    bool isTerminating;
    size_t runningObjects;
    Function<void(IController*)> stopCallback;
    Function<size_t(size_t, void*)> dataCallback;
    uint8 buffer[4 * 1024];

    Endpoint tcpEndpoint; // Listening port for direct connection
    TCPAcceptor acceptor; // TCP socket for direct connection from remote discoverer
    uint8 tcpBuffer[4 * 1024];
};

} // namespace Net
} // namespace DAVA

#endif // __DAVAENGINE_ANNOUNCER_H__
