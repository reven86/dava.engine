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

#ifndef __DAVAENGINE_NETDRIVER_H__
#define __DAVAENGINE_NETDRIVER_H__

#include <Base/BaseTypes.h>

#include <Network/Base/Endpoint.h>

#include <Network/ITransport.h>
#include <Network/IChannel.h>
#include <Network/INetDriver.h>
#include <Network/NetworkCommon.h>

namespace DAVA
{
namespace Net
{

class IOLoop;
class ServiceRegistrar;
class NetConfig;
class NetDriver : public INetDriver
                , public ITransportListener
{
private:
    struct ServiceEntry
    {
        ServiceEntry(uint32 id, IChannelListener* anObj);

        uint32 serviceId;
        IChannelListener* obj;
        size_t nref;
    };

    struct Channel : public IChannel
    {
        Channel(uint32 id, NetDriver* aDriver, ITransport* tr, ServiceEntry* entry);

        virtual bool Send(const void* data, size_t length, uint32 flags, uint32* packetId);
        virtual const Endpoint& RemoteEndpoint() const;

        uint32 channelId;
        Endpoint remoteEndpoint;
        NetDriver* driver;
        ITransport* transport;
        IChannelListener* service;
        ServiceEntry* serviceEntry;
    };

    struct TransportEntry
    {
        TransportEntry(ITransport* obj);

        ITransport* transport;
        Vector<Channel> channels;
    };

    friend bool operator == (const ServiceEntry& entry, uint32 serviceId);
    friend bool operator == (const Channel& ch, uint32 channelId);
    friend bool operator < (const Channel& left, const Channel& right);
    friend bool operator == (const TransportEntry& entry, const ITransport* obj);

public:
    NetDriver(IOLoop* aLoop, const ServiceRegistrar& aRegistrar);
    virtual ~NetDriver();

    bool ApplyConfig(const NetConfig& config);

    // INetDriver
    virtual void Start();
    virtual void Stop(Function<void (INetDriver*)> handler);

    // ITransportListener
    virtual void OnTransportActivated(ITransport* transport, const Endpoint& endp);
    virtual void OnTransportDeactivated(ITransport* transport, eDeactivationReason reason, int32 error);
    virtual void OnTransportTerminated(ITransport* transport);
    virtual void OnTransportReceive(ITransport* transport, uint32 channelId, const uint8* buffer, size_t length);
    virtual void OnTransportSendComplete(ITransport* transport, uint32 channelId, const uint8* buffer, size_t length);
    virtual void OnTransportPacketDelivered(ITransport* transport, uint32 channelId, uint32 packetId);

private:
    bool Send(Channel* channel, const void* data, size_t length, uint32 flags, uint32* packetId);

    ServiceEntry* GetServiceEntry(uint32 serviceId);
    TransportEntry* GetTransportEntry(ITransport* tr);
    Channel* GetTransportChannel(TransportEntry* entry, uint32 channelId);

    ITransport* CreateTransport(eTransportType type, eTransportRole role, const Endpoint& endpoint);

private:
    IOLoop* loop;
    eTransportRole role;
    size_t runningTransports;
    size_t activeTransport;
    Function<void (INetDriver*)> stopHandler;

    const ServiceRegistrar& registrar;
    Vector<TransportEntry> transports;
    Vector<ServiceEntry> services;
};

//////////////////////////////////////////////////////////////////////////
inline NetDriver::Channel::Channel(uint32 id, NetDriver* aDriver, ITransport* tr, ServiceEntry* entry)
    : channelId(id)
    , driver(aDriver)
    , transport(tr)
    , service(NULL)
    , serviceEntry(entry)
{}

inline bool NetDriver::Channel::Send(const void* data, size_t length, uint32 flags, uint32* packetId)
{
    return driver->Send(this, data, length, flags, packetId);
}

inline const Endpoint& NetDriver::Channel::RemoteEndpoint() const
{
    return remoteEndpoint;
}

inline NetDriver::ServiceEntry::ServiceEntry(uint32 id, IChannelListener* anObj)
    : serviceId(id)
    , obj(anObj)
    , nref(0)
{}

inline NetDriver::TransportEntry::TransportEntry(ITransport* obj)
    : transport(obj)
    , channels()
{}

inline bool operator == (const NetDriver::ServiceEntry& entry, uint32 serviceId)
{
    return entry.serviceId == serviceId;
}

inline bool operator == (const NetDriver::Channel& ch, uint32 channelId)
{
    return ch.channelId == channelId;
}

inline bool operator < (const NetDriver::Channel& left, const NetDriver::Channel& right)
{
    return left.channelId < right.channelId;
}

inline bool operator == (const NetDriver::TransportEntry& entry, const ITransport* obj)
{
    return entry.transport == obj;
}

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_NETDRIVER_H__
