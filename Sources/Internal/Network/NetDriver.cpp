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

#include <Debug/DVAssert.h>

#include <Network/TCPTransport.h>
#include <Network/ServiceRegistrar.h>
#include <Network/NetConfig.h>
#include <Network/NetDriver.h>

namespace DAVA
{
namespace Net
{

NetDriver::NetDriver(IOLoop* aLoop, const ServiceRegistrar& aRegistrar)
    : loop(aLoop)
    , role(SERVER_ROLE)
    , runningTransports(0)
    , activeTransport(0)
    , registrar(aRegistrar)
{

}

NetDriver::~NetDriver()
{
    DVASSERT(0 == runningTransports);
}

bool NetDriver::ApplyConfig(const NetConfig& config)
{
    const Vector<NetConfig::TransportConfig>& trConfig = config.Transports();
    const Vector<uint32>& serviceIds = config.Services();

    role = config.Role();
    transports.reserve(trConfig.size());
    services.reserve(serviceIds.size());

    for (size_t i = 0, n = serviceIds.size();i < n;++i)
    {
        uint32 id = serviceIds[i];
        IChannelListener* obj = registrar.Create(id);
        services.push_back(ServiceEntry(id, obj));
    }

    for (size_t i = 0, n = trConfig.size();i < n;++i)
    {
        ITransport* tr = CreateTransport(trConfig[i].type, role, trConfig[i].endpoint);
        transports.push_back(TransportEntry(tr));
        transports.back().channels.reserve(services.size());
    }

    if (SERVER_ROLE == role)
    {
        // Reference all transport when operating as server
        for (Vector<TransportEntry>::iterator tr = transports.begin(), etr = transports.end();tr != etr;++tr)
        {
            for (Vector<uint32>::const_iterator i = serviceIds.begin(), e = serviceIds.end();i != e;++i)
            {
                (*tr).channels.push_back(Channel(*i, this, (*tr).transport, GetServiceEntry(*i)));
            }
        }
    }
    else // if (CLIENT_ROLE == role)
    {
        // Reference only one transport when operating as client
        for (Vector<uint32>::const_iterator i = serviceIds.begin(), e = serviceIds.end();i != e;++i)
        {
            transports[activeTransport].channels.push_back(Channel(*i, this, transports[activeTransport].transport, GetServiceEntry(*i)));
        }
    }
    return true;
}

void NetDriver::Start()
{
    if (SERVER_ROLE == role)
    {
        runningTransports = transports.size();
        for (Vector<TransportEntry>::iterator i = transports.begin(), e = transports.end();i != e;++i)
        {
            TransportEntry& entry = *i;
            entry.transport->Activate();
        }
    }
    else
    {
        runningTransports = 1;
        transports[activeTransport].transport->Activate();
    }
}

void NetDriver::Stop(Function<void (INetDriver*)> handler)
{
    stopHandler = handler;
    if (SERVER_ROLE == role)
    {
        for (Vector<TransportEntry>::iterator i = transports.begin(), e = transports.end();i != e;++i)
        {
            (*i).transport->Deactivate();
        }
    }
    else
    {
        transports[activeTransport].transport->Deactivate();
    }
}

void NetDriver::OnTransportActivated(ITransport* transport, const Endpoint& endp)
{
    TransportEntry* entry = GetTransportEntry(transport);
    DVASSERT(entry);

    for (Vector<Channel>::iterator i = entry->channels.begin(), e = entry->channels.end();i != e;++i)
    {
        Channel& ch = *i;
        DVASSERT(NULL == ch.service);

        if (ch.serviceEntry->obj && 0 == ch.serviceEntry->nref)
        {
            ch.serviceEntry->nref += 1;

            ch.remoteEndpoint = endp;
            ch.service = ch.serviceEntry->obj;
            ch.service->OnChannelOpen(&ch);
        }
    }
}

void NetDriver::OnTransportDeactivated(ITransport* transport, eDeactivationReason reason, int32 error)
{
    TransportEntry* entry = GetTransportEntry(transport);
    DVASSERT(entry);

    for (Vector<Channel>::iterator i = entry->channels.begin(), e = entry->channels.end();i != e;++i)
    {
        Channel& ch = *i;
        if (ch.service != NULL)
        {
            ch.serviceEntry->nref -= 1;

            IChannelListener* service = ch.service;
            ch.service = NULL;
            service->OnChannelClosed(&ch);
        }
    }
}

void NetDriver::OnTransportTerminated(ITransport* transport)
{
    runningTransports -= 1;
    DVASSERT(runningTransports >= 0);
    if (0 == runningTransports && stopHandler != 0)
    {
        // delete services
        for (Vector<ServiceEntry>::iterator i = services.begin(), e = services.end();i != e;++i)
        {
            ServiceEntry& entry = *i;
            DVASSERT(0 == entry.nref);

            registrar.Delete(entry.serviceId, entry.obj);
        }
        services.clear();

        // delete transports
        for (Vector<TransportEntry>::iterator i = transports.begin(), e = transports.end();i != e;++i)
        {
            TransportEntry& entry = *i;
            delete entry.transport;
        }
        transports.clear();

        stopHandler(this);  // Say bye to beloved owner
    }
}

void NetDriver::OnTransportReceive(ITransport* transport, uint32 channelId, const uint8* buffer, size_t length)
{
    TransportEntry* entry = GetTransportEntry(transport);
    DVASSERT(entry);

    Channel* ch = GetTransportChannel(entry, channelId);
    DVASSERT(ch && ch->service);

    if (ch && ch->service)
    {
        ch->service->OnPacketReceived(ch, buffer, length);
    }
}

void NetDriver::OnTransportSendComplete(ITransport* transport, uint32 channelId, const uint8* buffer, size_t length)
{
    TransportEntry* entry = GetTransportEntry(transport);
    DVASSERT(entry);

    Channel* ch = GetTransportChannel(entry, channelId);
    DVASSERT(ch && ch->service);

    if (ch && ch->service)
    {
        ch->service->OnPacketSent(ch, buffer, length);
    }
}

void NetDriver::OnTransportPacketDelivered(ITransport* transport, uint32 channelId, uint32 packetId)
{
    TransportEntry* entry = GetTransportEntry(transport);
    DVASSERT(entry);

    Channel* ch = GetTransportChannel(entry, channelId);
    DVASSERT(ch && ch->service);

    if (ch && ch->service)
    {
        ch->service->OnPacketDelivered(ch, packetId);
    }
}

bool NetDriver::Send(Channel* channel, const void* data, size_t length, uint32 flags, uint32* packetId)
{
    DVASSERT(data != NULL && length > 0 && channel->service != NULL);
    if (channel->service != NULL)
    {
        channel->transport->Send(channel->channelId, static_cast<const uint8*>(data), length, packetId);
        return true;
    }
    return false;
}

NetDriver::ServiceEntry* NetDriver::GetServiceEntry(uint32 serviceId)
{
    Vector<ServiceEntry>::iterator i = std::find(services.begin(), services.end(), serviceId);
    return i != services.end() ? &*i
                               : NULL;
}

NetDriver::TransportEntry* NetDriver::GetTransportEntry(ITransport* tr)
{
    Vector<TransportEntry>::iterator i = std::find(transports.begin(), transports.end(), tr);
    return i != transports.end() ? &*i
                                 : NULL;
}

NetDriver::Channel* NetDriver::GetTransportChannel(TransportEntry* entry, uint32 channelId)
{
    Vector<Channel>::iterator i = std::find(entry->channels.begin(), entry->channels.end(), channelId);
    return i != entry->channels.end() ? &*i
                                      : NULL;
}

ITransport* NetDriver::CreateTransport(eTransportType type, eTransportRole role, const Endpoint& endpoint)
{
    switch(type)
    {
    case TRANSPORT_TCP:
        return new TCPTransport(loop, this, role, endpoint);
    default:
        DVASSERT(0 && "Unknown transport type");
        return NULL;
    }
}

}   // namespace Net
}   // namespace DAVA
