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


#include <algorithm>

#include <Functional/Function.h>
#include <Debug/DVAssert.h>

#include <Network/Base/IOLoop.h>
#include <Network/Base/NetworkUtils.h>
#include <Network/ServiceRegistrar.h>
#include <Network/NetConfig.h>

#include <Network/Private/ProtoDriver.h>
#include <Network/Private/TCPServerTransport.h>
#include <Network/Private/TCPClientTransport.h>

#include <Network/Private/NetController.h>

namespace DAVA
{
namespace Net
{

NetController::NetController(IOLoop* aLoop, const ServiceRegistrar& aRegistrar, void* aServiceContext, uint32 readTimeout_)
    : loop(aLoop)
    , role(SERVER_ROLE)
    , registrar(aRegistrar)
    , serviceContext(aServiceContext)
    , runningObjects(0)
    , isTerminating(false)
    , readTimeout(readTimeout_)
{
    DVASSERT(loop != NULL);
}

NetController::~NetController()
{
    DVASSERT(0 == runningObjects);
    if (SERVER_ROLE == role)
    {
        for (Vector<IServerTransport*>::iterator i = servers.begin(), e = servers.end();i != e;++i)
            delete *i;
    }
    else
    {
        for (List<ClientEntry>::iterator i = clients.begin(), e = clients.end();i != e;++i)
        {
            ClientEntry& entry = *i;
            delete entry.driver;
            delete entry.client;
        }
    }
}

bool NetController::ApplyConfig(const NetConfig& config, size_t trIndex)
{
    DVASSERT(true == config.Validate() && trIndex < config.Transports().size());
    if (false == config.Validate() || trIndex >= config.Transports().size()) return false;

    const Vector<NetConfig::TransportConfig>& trConfig = config.Transports();

    role = config.Role();
    serviceIds = config.Services();
    if (SERVER_ROLE == role)
    {
        servers.reserve(trConfig.size());
        for (size_t i = 0, n = trConfig.size();i < n;++i)
        {
            IServerTransport* tr = CreateServerTransport(trConfig[i].type, trConfig[i].endpoint);
            DVASSERT(tr != NULL);
            if (tr != NULL)
                servers.push_back(tr);
        }
    }
    else // if (CLIENT_ROLE == role)
    {
        // For now create only one transport when operating as client
        IClientTransport* tr = CreateClientTransport(trConfig[trIndex].type, trConfig[trIndex].endpoint);
        DVASSERT(tr != NULL);
        if (tr != NULL)
        {
            ProtoDriver* driver = new ProtoDriver(loop, role, registrar, serviceContext);
            driver->SetTransport(tr, &*serviceIds.begin(), serviceIds.size());
            clients.push_back(ClientEntry(tr, driver));
        }
    }
    return true;
}

void NetController::Start()
{
    SERVER_ROLE == role ? loop->Post(MakeFunction(this, &NetController::DoStartServers))
                        : loop->Post(MakeFunction(this, &NetController::DoStartClients));
}

void NetController::Stop(Function<void (IController*)> handler)
{
    DVASSERT(false == isTerminating && handler != nullptr);
    isTerminating = true;
    stopHandler = handler;
    SERVER_ROLE == role ? loop->Post(MakeFunction(this, &NetController::DoStopServers))
                        : loop->Post(MakeFunction(this, &NetController::DoStopClients));
}

void NetController::Restart()
{
    DVASSERT(false == isTerminating);
    loop->Post(MakeFunction(this, &NetController::DoRestart));
}

void NetController::DoStartServers()
{
    runningObjects = servers.size();
    for (size_t i = 0, n = servers.size();i < n;++i)
    {
        servers[i]->Start(this);
    }
}

void NetController::DoStartClients()
{
    // For now there is always one transport in client role
    runningObjects = 1;
    clients.front().client->Start(this);
}

void NetController::DoRestart()
{
    for (List<ClientEntry>::iterator i = clients.begin(), e = clients.end();i != e;++i)
    {
        ClientEntry& entry = *i;
        entry.client->Reset();
    }
    for (size_t i = 0, n = servers.size();i < n;++i)
        servers[i]->Reset();
}

void NetController::DoStopServers()
{
    if (true == clients.empty())
    {
        for (size_t i = 0, n = servers.size();i < n;++i)
        {
            servers[i]->Stop();
        }
    }
    else
        DoStopClients();
}

void NetController::DoStopClients()
{
    for (List<ClientEntry>::iterator i = clients.begin(), e = clients.end();i != e;++i)
    {
        ClientEntry& entry = *i;
        entry.client->Stop();
    }
}

void NetController::OnTransportSpawned(IServerTransport* parent, IClientTransport* child)
{
    DVASSERT(std::find(servers.begin(), servers.end(), parent) != servers.end());

    ProtoDriver* driver = new ProtoDriver(loop, role, registrar, serviceContext);
    driver->SetTransport(child, &*serviceIds.begin(), serviceIds.size());
    clients.push_back(ClientEntry(child, driver, parent));

    child->Start(this);
}

void NetController::OnTransportTerminated(IServerTransport* tr)
{
    DVASSERT(std::find(servers.begin(), servers.end(), tr) != servers.end());

    DVASSERT(runningObjects > 0);
    runningObjects -= 1;
    if (0 == runningObjects)
        stopHandler(this);
}

void NetController::OnTransportTerminated(IClientTransport* tr)
{
    List<ClientEntry>::iterator i = std::find(clients.begin(), clients.end(), tr);
    DVASSERT(i != clients.end());
    ClientEntry& entry = *i;
    entry.driver->ReleaseServices();

    if (SERVER_ROLE == role)
    {
        entry.parent->ReclaimClient(entry.client);
        delete entry.driver;
        clients.erase(i);

        if (true == isTerminating && true == clients.empty())
        {
            DoStopServers();
        }
    }
    else
    {
        DVASSERT(runningObjects > 0);
        runningObjects -= 1;
        if (0 == runningObjects)
            stopHandler(this);
    }
}

void NetController::OnTransportConnected(IClientTransport* tr, const Endpoint& endp)
{
    DVASSERT(GetClientEntry(tr) != NULL);
    GetClientEntry(tr)->driver->OnConnected(endp);
}

void NetController::OnTransportDisconnected(IClientTransport* tr, int32 error)
{
    DVASSERT(GetClientEntry(tr) != NULL);
    GetClientEntry(tr)->driver->OnDisconnected(error ? ErrorToString(error) : "");
}

void NetController::OnTransportDataReceived(IClientTransport* tr, const void* buffer, size_t length)
{
    ClientEntry* entry = GetClientEntry(tr);
    DVASSERT(entry != NULL);

    if (false == entry->driver->OnDataReceived(buffer, length))
    {
        entry->client->Reset();
    }
}

void NetController::OnTransportSendComplete(IClientTransport* tr)
{
    DVASSERT(GetClientEntry(tr) != NULL);
    GetClientEntry(tr)->driver->OnSendComplete();
}

void NetController::OnTransportReadTimeout(IClientTransport* tr)
{
    DVASSERT(GetClientEntry(tr) != NULL);
    
    ClientEntry* entry = GetClientEntry(tr);
    if (false == entry->driver->OnTimeout())
    {
        entry->client->Reset();
    }
}

NetController::ClientEntry* NetController::GetClientEntry(IClientTransport* client)
{
    List<ClientEntry>::iterator i = std::find(clients.begin(), clients.end(), client);
    return i != clients.end() ? &*i
                              : NULL;
}

IServerTransport* NetController::CreateServerTransport(eTransportType type, const Endpoint& endpoint)
{
    switch(type)
    {
    case TRANSPORT_TCP:
        return new TCPServerTransport(loop, endpoint, readTimeout);
    default:
        DVASSERT(0 && "Unknown transport type");
        return NULL;
    }
}

IClientTransport* NetController::CreateClientTransport(eTransportType type, const Endpoint& endpoint)
{
    switch(type)
    {
    case TRANSPORT_TCP:
        return new TCPClientTransport(loop, endpoint, readTimeout);
    default:
        DVASSERT(0 && "Unknown transport type");
        return NULL;
    }
}

}   // namespace Net
}   // namespace DAVA
