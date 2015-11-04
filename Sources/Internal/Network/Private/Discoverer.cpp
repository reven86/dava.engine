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


#include <Functional/Function.h>
#include <Debug/DVAssert.h>

#include <Network/Private/Discoverer.h>

namespace DAVA
{
namespace Net
{

Discoverer::Discoverer(IOLoop* ioLoop, const Endpoint& endp, Function<void (size_t, const void*, const Endpoint&)> dataReadyCallback)
    : loop(ioLoop)
    , socket(ioLoop)
    , timer(ioLoop)
    , endpoint(endp)
    , isTerminating(false)
    , runningObjects(0)
    , dataCallback(dataReadyCallback)
{
    DVVERIFY(true == endpoint.Address().ToString(endpAsString.data(), endpAsString.size()));
    DVASSERT(true == endpoint.Address().IsMulticast());
    DVASSERT(loop != nullptr && dataCallback != nullptr);
}

Discoverer::~Discoverer()
{

}

void Discoverer::Start()
{
    DVASSERT(false == isTerminating);
    loop->Post(MakeFunction(this, &Discoverer::DoStart));
}

void Discoverer::Stop(Function<void (IController*)> callback)
{
    DVASSERT(false == isTerminating);
    DVASSERT(callback != nullptr);
    isTerminating = true;
    stopCallback = callback;
    loop->Post(MakeFunction(this, &Discoverer::DoStop));
}

void Discoverer::Restart()
{
    loop->Post(MakeFunction(this, &Discoverer::DoStop));
}

void Discoverer::DoStart()
{
    int32 error = socket.Bind(Endpoint(endpoint.Port()), true);
    if (0 == error)
    {
        error = socket.JoinMulticastGroup(endpAsString.data(), NULL);
        if (0 == error)
            error = socket.StartReceive(CreateBuffer(inbuf, sizeof(inbuf)), MakeFunction(this, &Discoverer::SocketHandleReceive));
    }
    if (error != 0 && false == isTerminating)
    {
        DoStop();
    }
}

void Discoverer::DoStop()
{
    if (true == socket.IsOpen() && false == socket.IsClosing())
    {
        runningObjects += 1;
        socket.Close(MakeFunction(this, &Discoverer::SocketHandleClose));
    }
    if (true == timer.IsOpen() && false == timer.IsClosing())
    {
        runningObjects += 1;
        timer.Close(MakeFunction(this, &Discoverer::TimerHandleClose));
    }
}

void Discoverer::DoObjectClose()
{
    runningObjects -= 1;
    if (0 == runningObjects)
    {
        if (true == isTerminating)
        {
            loop->Post(MakeFunction(this, &Discoverer::DoBye));
        }
        else
        {
            timer.Wait(RESTART_DELAY_PERIOD, MakeFunction(this, &Discoverer::TimerHandleDelay));
        }
    }
}

void Discoverer::DoBye()
{
    isTerminating = false;
    stopCallback(this);
}
    
void Discoverer::TimerHandleClose(DeadlineTimer* timer)
{
    DoObjectClose();
}

void Discoverer::TimerHandleDelay(DeadlineTimer* timer)
{
    DoStart();
}
    
void Discoverer::SocketHandleClose(UDPSocket* socket)
{
    DoObjectClose();
}

void Discoverer::SocketHandleReceive(UDPSocket* socket, int32 error, size_t nread, const Endpoint& endpoint, bool partial)
{
    if (true == isTerminating) return;

    if (0 == error)
    {
        if (nread > 0 && false == partial)
        {
            dataCallback(nread, inbuf, endpoint);
        }
    }
    else
    {
        DoStop();
    }
}

}   // namespace Net
}   // namespace DAVA
