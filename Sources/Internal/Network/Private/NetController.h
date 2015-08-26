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


#ifndef __DAVAENGINE_NETCONTROLLER_H__
#define __DAVAENGINE_NETCONTROLLER_H__

#include "Base/BaseTypes.h"

#include "Network/NetworkCommon.h"
#include "Network/IController.h"
#include "Network/Private/ITransport.h"

namespace DAVA
{
namespace Net
{

class IOLoop;
class ServiceRegistrar;
class NetConfig;
class ProtoDriver;

class NetController : public IController
                    , public IServerListener
                    , public IClientListener
{
private:
    struct ClientEntry
    {
        ClientEntry(IClientTransport* aClient, ProtoDriver* aDriver, IServerTransport* aParent = NULL);

        IClientTransport* client;
        IServerTransport* parent;
        ProtoDriver* driver;
    };

    friend bool operator == (const ClientEntry& entry, const IClientTransport* obj);

public:
    NetController(IOLoop* aLoop, const ServiceRegistrar& aRegistrar, void* aServiceContext, uint32 readTimeout = DEFAULT_READ_TIMEOUT);
    virtual ~NetController();

    bool ApplyConfig(const NetConfig& config, size_t trIndex = 0);

    // IController
    virtual void Start();
    virtual void Stop(Function<void (IController*)> handler);
    virtual void Restart();

    // IServerListener
    virtual void OnTransportSpawned(IServerTransport* parent, IClientTransport* child);
    virtual void OnTransportTerminated(IServerTransport* tr);

    // IClientListener
    virtual void OnTransportTerminated(IClientTransport* tr);
    virtual void OnTransportConnected(IClientTransport* tr, const Endpoint& endp);
    virtual void OnTransportDisconnected(IClientTransport* tr, int32 error);
    virtual void OnTransportDataReceived(IClientTransport* tr, const void* buffer, size_t length);
    virtual void OnTransportSendComplete(IClientTransport* tr);
    virtual void OnTransportReadTimeout(IClientTransport* tr);

private:
    void DoStartServers();
    void DoStartClients();
    void DoRestart();
    void DoStopServers();
    void DoStopClients();

    ClientEntry* GetClientEntry(IClientTransport* client);

    IServerTransport* CreateServerTransport(eTransportType type, const Endpoint& endpoint);
    IClientTransport* CreateClientTransport(eTransportType type, const Endpoint& endpoint);

private:
    IOLoop* loop;
    eNetworkRole role;
    const ServiceRegistrar& registrar;
    void* serviceContext;
    size_t runningObjects;
    Function<void (IController*)> stopHandler;
    bool isTerminating;
    uint32 readTimeout = 0;

    Vector<uint32> serviceIds;
    Vector<IServerTransport*> servers;
    List<ClientEntry> clients;
};

//////////////////////////////////////////////////////////////////////////
inline NetController::ClientEntry::ClientEntry(IClientTransport* aClient, ProtoDriver* aDriver, IServerTransport* aParent)
    : client(aClient)
    , parent(aParent)
    , driver(aDriver)
{}

inline bool operator == (const NetController::ClientEntry& entry, const IClientTransport* obj)
{
    return entry.client == obj;
}

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_NETCONTROLLER_H__
