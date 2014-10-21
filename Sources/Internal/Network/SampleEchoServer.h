#ifndef __DAVAENGINE_SAMPLEECHOSERVER_H__
#define __DAVAENGINE_SAMPLEECHOSERVER_H__

#include "TCPAcceptor.h"
#include "UDPSocket.h"

namespace DAVA {

class IOLoop;
class TCPSocket;

class SampleEchoServer
{
public:
    SampleEchoServer (IOLoop* ioLoop);
    ~SampleEchoServer ();

    void Start (unsigned short port);
    void Stop ();

private:
    void HandleConnect (TCPAcceptor* acceptor, int error);
    void HandleRead (TCPSocket* socket, int error, std::size_t nread, void* buffer);
    void HandleWrite (TCPSocket* socket, int error, const void* buffer);

    void HandleReceive (UDPSocket* socket, int error, std::size_t nread, void* buffer, const Endpoint& endpoint, unsigned int flags);
    void HandleSend (UDPSocket* socket, int error, const void* buffer);

    char* DuplicateInput (char* input, std::size_t nread);

private:
    TCPAcceptor acceptor;
    UDPSocket   udpSocket;
    char        tcpInbuf[100];
    char        udpInbuf[100];
};

}   // namespace DAVA

#endif  // __DAVAENGINE_SAMPLEECHOSERVER_H__
