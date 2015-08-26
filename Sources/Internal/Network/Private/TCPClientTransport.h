/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_TCPCLIENTTRANSPORT_H__
#define __DAVAENGINE_TCPCLIENTTRANSPORT_H__

#include <Network/Base/Endpoint.h>
#include <Network/Base/TCPSocket.h>
#include <Network/Base/DeadlineTimer.h>

#include <Network/Private/ITransport.h>

namespace DAVA
{
namespace Net
{

class IOLoop;
class TCPClientTransport : public IClientTransport
{
    static const uint32 RESTART_DELAY_PERIOD = 3000;

public:
    // Constructor for accepted connection
    TCPClientTransport(IOLoop* aLoop, uint32 readTimeout);
    // Constructor for connection initiator
    TCPClientTransport(IOLoop* aLoop, const Endpoint& aEndpoint, uint32 readTimeout);
    virtual ~TCPClientTransport();

    TCPSocket& Socket();

    // IClientTransport
    virtual int32 Start(IClientListener* aListener);
    virtual void Stop();
    virtual void Reset();
    virtual int32 Send(const Buffer* buffers, size_t bufferCount);

private:
    void DoStart();
    int32 DoConnected();
    void CleanUp(int32 error);
    void RunningObjectStopped();
    void DoBye();

    void TimerHandleClose(DeadlineTimer* timer);
    void TimerHandleTimeout(DeadlineTimer* timer);
    void TimerHandleDelay(DeadlineTimer* timer);

    void SocketHandleClose(TCPSocket* socket);
    void SocketHandleConnect(TCPSocket* socket, int32 error);
    void SocketHandleRead(TCPSocket* socket, int32 error, size_t nread);
    void SocketHandleWrite(TCPSocket* socket, int32 error, const Buffer* buffers, size_t bufferCount);

private:
    IOLoop* loop;
    Endpoint endpoint;
    Endpoint remoteEndpoint;
    size_t runningObjects;
    TCPSocket socket;
    DeadlineTimer timer;
    IClientListener* listener;  // Who receive notifications; also indicator that Start has been called
    uint32 readTimeout;
    bool isInitiator;       // true: establishes connection; false: created from accepted connection
    bool isTerminating;     // Stop has been invoked
    bool isConnected;       // Connections has been established

    static const size_t INBUF_SIZE = 10 * 1024;
    uint8 inbuf[INBUF_SIZE];

    static const size_t SENDBUF_COUNT = 2;
    Buffer sendBuffers[SENDBUF_COUNT];
    size_t sendBufferCount;
};

//////////////////////////////////////////////////////////////////////////
inline TCPSocket& TCPClientTransport::Socket()
{
    return socket;
}

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_TCPCLIENTTRANSPORT_H__
