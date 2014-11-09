#ifndef FileTransfer_SimpleSender_h
#define FileTransfer_SimpleSender_h

#include <Network/UDPSocketEx.h>
#include <Network/DeadlineTimer.h>

namespace DAVA
{

class IOLoop;
    
class SimpleSender
{
public:
    SimpleSender(IOLoop* loop);
    ~SimpleSender();
    
    void Start(const char8* ipaddr, uint16 port);
    void Stop();
    
private:
    void Cleanup();
    void StartWait(uint32 timeout);
    void Send(const char8* buf, std::size_t length = std::size_t(-1));
    
    void HandleCloseSocket(UDPSocketEx* socket);
    void HandleSend(UDPSocketEx* socket, int32 error, const Buffer* buffers, std::size_t bufferCount);

    void HandleCloseTimer(DeadlineTimer* socket);
    void HandleTimer(DeadlineTimer* timer);
    
private:
    UDPSocketEx   socket;
    DeadlineTimer timer;
    Endpoint      endpoint;
    bool          interrupted;
    char8         outbuf[128];
    int           n;
};
    
}

#endif
