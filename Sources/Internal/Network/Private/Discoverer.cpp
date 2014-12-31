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

#include "Discoverer.h"

namespace DAVA
{
namespace Net
{

Discoverer::Discoverer(IOLoop* ioLoop, const Endpoint& endp, Function<void (size_t, const void*)> dataReadyCallback)
    : loop(ioLoop)
    , socket(ioLoop)
    , endpoint(endp)
    , isTerminating(false)
    , dataCallback(dataReadyCallback)
{
    DVASSERT(loop != NULL && dataCallback != 0 && true == endpoint.Address().IsMulticast());
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
    DVASSERT(callback != 0 && false == isTerminating);
    isTerminating = true;
    stopCallback = callback;
    loop->Post(MakeFunction(this, &Discoverer::DoStop));
}

void Discoverer::DoStart()
{
    char8 addr[30];
    DVVERIFY(true == endpoint.Address().ToString(addr, COUNT_OF(addr)));

    int32 error = socket.Bind(Endpoint(endpoint.Port()), true);
    if (0 == error)
    {
        error = socket.JoinMulticastGroup(addr, NULL);
        if (0 == error)
            error = socket.StartReceive(CreateBuffer(inbuf, sizeof(inbuf)), MakeFunction(this, &Discoverer::SocketHandleReceive));
    }
    DVASSERT(0 == error);
}

void Discoverer::DoStop()
{
    socket.Close(MakeFunction(this, &Discoverer::SocketHandleClose));
}

void Discoverer::SocketHandleClose(UDPSocket* socket)
{
    if (true == isTerminating)
    {
        isTerminating = false;
        stopCallback(this);
    }
    else
        DoStart();
}

void Discoverer::SocketHandleReceive(UDPSocket* socket, int32 error, size_t nread, const Endpoint& endpoint, bool partial)
{
    if (true == isTerminating) return;

    if (0 == error)
    {
        if (nread > 0 && false == partial)
        {
            dataCallback(nread, inbuf);
        }
    }
    else
    {
        DoStop();
    }
}

}   // namespace Net
}   // namespace DAVA
