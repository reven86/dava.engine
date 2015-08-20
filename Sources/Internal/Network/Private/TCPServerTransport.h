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


#ifndef __DAVAENGINE_TCPSERVERTRANSPORT_H__
#define __DAVAENGINE_TCPSERVERTRANSPORT_H__

#include <Network/Base/Endpoint.h>
#include <Network/Base/TCPAcceptor.h>

#include <Network/Private/ITransport.h>

namespace DAVA
{
namespace Net
{

class IOLoop;
class TCPServerTransport : public IServerTransport
{
public:
    TCPServerTransport(IOLoop* aLoop, const Endpoint& aEndpoint, uint32 readTimeout);
    virtual ~TCPServerTransport();

    // IServerTransport
    virtual int32 Start(IServerListener* listener);
    virtual void Stop();
    virtual void Reset();
    virtual void ReclaimClient(IClientTransport* client);

private:
    int32 DoStart();
    void DoStop();

    void AcceptorHandleClose(TCPAcceptor* acceptor);
    void AcceptorHandleConnect(TCPAcceptor* acceptor, int32 error);

private:
    IOLoop* loop;
    Endpoint endpoint;
    TCPAcceptor acceptor;
    IServerListener* listener;  // Who receive notifications; also indicator that Start has been called
    bool isTerminating;         // Stop has been invoked
    uint32 readTimeout = 0;

    Set<IClientTransport*> spawnedClients;
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_TCPSERVERTRANSPORT_H__
