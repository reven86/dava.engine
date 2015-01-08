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

#include <Base/FunctionTraits.h>
#include <Debug/DVAssert.h>

#include <Network/Private/TCPClientTransport.h>

namespace DAVA
{
namespace Net
{

TCPClientTransport::TCPClientTransport(IOLoop* aLoop)
    : endpoint()
    , runningObjects(0)
    , socket(aLoop)
    , timer(aLoop)
    , listener(NULL)
    , readTimeout(3600 * 1000)
    , isInitiator(false)
    , isTerminating(false)
    , isConnected(false)
    , sendBufferCount(0)
{
    DVASSERT(aLoop != NULL);
}

TCPClientTransport::TCPClientTransport(IOLoop* aLoop, const Endpoint& aEndpoint)
    : endpoint(aEndpoint)
    , runningObjects(0)
    , socket(aLoop)
    , timer(aLoop)
    , listener(NULL)
    , readTimeout(3600 * 1000)
    , isInitiator(true)
    , isTerminating(false)
    , isConnected(false)
    , sendBufferCount(0)
{
    DVASSERT(aLoop != NULL);
}

TCPClientTransport::~TCPClientTransport()
{
    DVASSERT(NULL == listener && false == isTerminating && false == isConnected && 0 == runningObjects);
}

int32 TCPClientTransport::Start(IClientListener* aListener)
{
    DVASSERT(false == isTerminating && aListener != NULL && NULL == listener);
    listener = aListener;
    DoStart();
    return 0;
}

void TCPClientTransport::Stop()
{
    DVASSERT(listener != NULL && false == isTerminating);
    isTerminating = true;
    CleanUp(0);
}

void TCPClientTransport::Reset()
{
    DVASSERT(listener != NULL && false == isTerminating);
    CleanUp(0);
}

int32 TCPClientTransport::Send(const Buffer* buffers, size_t bufferCount)
{
    DVASSERT(buffers != NULL && 0 < bufferCount && bufferCount <= SENDBUF_COUNT);
    DVASSERT(true == isConnected && 0 == sendBufferCount);

    for (size_t i = 0;i < bufferCount;++i)
    {
        DVASSERT(buffers[i].base != NULL && buffers[i].len > 0);
        sendBuffers[i] = buffers[i];
    }
    sendBufferCount = bufferCount;

    int32 error = socket.Write(sendBuffers, sendBufferCount, MakeFunction(this, &TCPClientTransport::SocketHandleWrite));
    return error;
}

void TCPClientTransport::DoStart()
{
    DVASSERT(0 == runningObjects);
    runningObjects = 2; // Socket and timer

    // Try to establish connection if connection is initiated by this
    // Otherwise connection should be already accepted
    int32 error = true == isInitiator ? socket.Connect(endpoint, MakeFunction(this, &TCPClientTransport::SocketHandleConnect))
                                      : DoConnected();
    DVASSERT(0 == error);
    if (error != 0)
        CleanUp(error);
}

int32 TCPClientTransport::DoConnected()
{
    int32 error = socket.RemoteEndpoint(remoteEndpoint);
    if (0 == error)
        error = socket.StartRead(CreateBuffer(inbuf, sizeof(inbuf)), MakeFunction(this, &TCPClientTransport::SocketHandleRead));
    if (0 == error)
    {
        timer.Wait(readTimeout, MakeFunction(this, &TCPClientTransport::TimerHandleTimeout));
        isConnected = true;
        listener->OnTransportConnected(this, remoteEndpoint);
    }
    DVASSERT(0 == error);
    return error;
}

void TCPClientTransport::CleanUp(int32 error)
{
    if (true == isConnected)
    {
        isConnected = false;
        sendBufferCount = 0;
        listener->OnTransportDisconnected(this, error);
    }
    socket.Close(MakeFunction(this, &TCPClientTransport::SocketHandleClose));
    timer.Close(MakeFunction(this, &TCPClientTransport::TimerHandleClose));
}

void TCPClientTransport::RunningObjectStopped()
{
    DVASSERT(runningObjects > 0);
    runningObjects -= 1;
    if (runningObjects > 0) return;

    if (true == isTerminating || false == isInitiator)
    {
        IClientListener* p = listener;
        listener = NULL;
        isTerminating = false;
        p->OnTransportTerminated(this); // This can be the last executed line of object instance
    }
    else if (true == isInitiator)
    {
        DoStart();
    }
}

void TCPClientTransport::TimerHandleClose(DeadlineTimer* timer)
{
    RunningObjectStopped();
}

void TCPClientTransport::TimerHandleTimeout(DeadlineTimer* timer)
{
    if (true == isTerminating) return;
}

void TCPClientTransport::SocketHandleClose(TCPSocket* socket)
{
    RunningObjectStopped();
}

void TCPClientTransport::SocketHandleConnect(TCPSocket* socket, int32 error)
{
    if (true == isTerminating) return;

    if (0 == error)
        error = DoConnected();
    if (error != 0)
        CleanUp(error);
}

void TCPClientTransport::SocketHandleRead(TCPSocket* socket, int32 error, size_t nread)
{
    DVASSERT(false == isTerminating && true == isConnected);
    if (0 == error)
    {
        listener->OnTransportDataReceived(this, inbuf, nread);
        timer.Wait(readTimeout, MakeFunction(this, &TCPClientTransport::TimerHandleTimeout));
    }
    else
    {
        CleanUp(error);
    }
}

void TCPClientTransport::SocketHandleWrite(TCPSocket* socket, int32 error, const Buffer* buffers, size_t bufferCount)
{
    if (false == isConnected) return;

    if (0 == error)
    {
        sendBufferCount = 0;
        listener->OnTransportSendComplete(this);
    }
    else
    {
        CleanUp(error);
    }
}

}   // namespace Net
}   // namespace DAVA
