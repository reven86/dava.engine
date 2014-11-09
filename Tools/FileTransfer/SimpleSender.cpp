#include <cstdio>

#include <Base/FunctionTraits.h>
#include <Network/IOLoop.h>

#include "SimpleSender.h"

namespace DAVA
{

SimpleSender::SimpleSender(IOLoop* loop) : socket(loop)
                                         , timer(loop)
                                         , endpoint()
                                         , interrupted(false)
                                         , n(0)
{
    socket.SetSendHandler(MakeFunction(this, &SimpleSender::HandleSend));
    socket.SetCloseHandler(MakeFunction(this, &SimpleSender::HandleCloseSocket));
    timer.SetCloseHandler(MakeFunction(this, &SimpleSender::HandleCloseTimer));
}
    
SimpleSender::~SimpleSender() {}
    
void SimpleSender::Start(const char8* ipaddr, uint16 port)
{
    endpoint = Endpoint(ipaddr, port);
    
    StartWait(0);
}

void SimpleSender::Stop()
{
    interrupted = true;
}

void SimpleSender::Cleanup()
{
    socket.Close();
    timer.Close();
}

void SimpleSender::StartWait(uint32 timeout)
{
    timer.AsyncStartWait(timeout, MakeFunction(this, &SimpleSender::HandleTimer));
}

void SimpleSender::Send(const char8* buf, std::size_t length)
{
    if (length == std::size_t(-1))
        length = strlen(buf);
    Memcpy(outbuf, buf, length);
    outbuf[length] = '\0';
    
    Buffer buffer = CreateBuffer(outbuf, length);
    socket.AsyncSend(endpoint, &buffer, 1);
}

void SimpleSender::HandleCloseSocket(UDPSocketEx* socket)
{
    printf("Sender socket closed\n");
}
    
void SimpleSender::HandleSend(UDPSocketEx* socket, int32 error, const Buffer* buffers, std::size_t bufferCount)
{
    //static int n = 0;
    //n += 1;
    
    uint32 to = n % 2 ? 250 : 3000;
    //printf("%d. Ping sent, next timeout %u\n", n, to);
    printf("%d. Sent - %s\n", n, outbuf);
    if (n < 10)
    {
        StartWait(to);
    }
    else
        Cleanup();
}
    
void SimpleSender::HandleCloseTimer(DeadlineTimer* socket)
{
    printf("Sender timer closed\n");
}

void SimpleSender::HandleTimer(DeadlineTimer* timer)
{
    char8 s[30];
    snprintf(s, COUNT_OF(s), "%d. PING", ++n);
    Send(s);
    //Send("PING");
}

}
