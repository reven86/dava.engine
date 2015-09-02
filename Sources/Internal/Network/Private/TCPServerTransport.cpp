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

#include <Network/Base/IOLoop.h>

#include <Network/Private/TCPClientTransport.h>
#include <Network/Private/TCPServerTransport.h>

namespace DAVA
{
namespace Net
{

TCPServerTransport::TCPServerTransport(IOLoop* aLoop, const Endpoint& aEndpoint, uint32 readTimeout_)
    : loop(aLoop)
    , endpoint(aEndpoint)
    , acceptor(aLoop)
    , listener(NULL)
    , isTerminating(false)
    , readTimeout(readTimeout_)
{
    DVASSERT(loop != NULL);
}

TCPServerTransport::~TCPServerTransport()
{
    DVASSERT(NULL == listener && false == isTerminating && true == spawnedClients.empty());
}

int32 TCPServerTransport::Start(IServerListener* aListener)
{
    DVASSERT(NULL == listener && false == isTerminating && aListener != NULL);
    listener = aListener;
    return DoStart();
}

void TCPServerTransport::Stop()
{
    DVASSERT(listener != NULL && false == isTerminating);
    isTerminating = true;
    DoStop();
}

void TCPServerTransport::Reset()
{
    DVASSERT(listener != NULL && false == isTerminating);
    DoStop();
}

void TCPServerTransport::ReclaimClient(IClientTransport* client)
{
    DVASSERT(spawnedClients.find(client) != spawnedClients.end());
    if (spawnedClients.find(client) != spawnedClients.end())
    {
        spawnedClients.erase(client);
        SafeDelete(client);
    }
}

int32 TCPServerTransport::DoStart()
{
    int32 error = acceptor.Bind(endpoint);
    if (0 == error)
    {
        error = acceptor.StartListen(MakeFunction(this, &TCPServerTransport::AcceptorHandleConnect));
    }
    if (error != 0)
    {
        DoStop();
    }
    return error;
}

void TCPServerTransport::DoStop()
{
    if (acceptor.IsOpen() && !acceptor.IsClosing())
    {
        acceptor.Close(MakeFunction(this, &TCPServerTransport::AcceptorHandleClose));
    }
}

void TCPServerTransport::AcceptorHandleClose(TCPAcceptor* acceptor)
{
    if (isTerminating)
    {
        IServerListener* p = listener;
        listener = NULL;
        isTerminating = false;
        p->OnTransportTerminated(this);     // This can be the last executed line of object instance
    }
    else
    {
        DoStart();
    }
}

void TCPServerTransport::AcceptorHandleConnect(TCPAcceptor* acceptor, int32 error)
{
    if (true == isTerminating) return;
    
    if (0 == error)
    {
        TCPClientTransport* client = new TCPClientTransport(loop, readTimeout);
        error = acceptor->Accept(&client->Socket());
        if (0 == error)
        {
            spawnedClients.insert(client);
            listener->OnTransportSpawned(this, client);
        }
        else
        {
            SafeDelete(client);
        }
    }
    else
    {
        DoStop();
    }
}

}   // namespace Net
}   // namespace DAVA
