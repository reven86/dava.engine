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

#ifndef __DAVAENGINE_CHANNELMANAGER_H__
#define __DAVAENGINE_CHANNELMANAGER_H__

#include <utility>

#include <Base/Noncopyable.h>
#include <Base/Function.h>

#include <Network/Base/Endpoint.h>

#include "NetworkCommon.h"
#include "ITransport.h"
#include "IChannel.h"

namespace DAVA
{
namespace Net
{

/*
 Class ChannelManager implements concept of data channels hiding details about how data is transfered.
 This class binds data channels to transports and manages dataflow between channels and transports.

 ChannelManager do the following things:
    - supports any type of transport which can be created by TransportFactory
    - supports any number of transports and channels
    - channel is bound to one transport but transport may work with any number of channels
    - data recieved through channel are passed to clients through IChannelListener interface
    - client can be bound to several channels but channel may work only with one client
    - clients can send data through IChannelSender interface

 If client is bound to only one channel then client is allowed to specify DEFAULT_CHANNEL_ID
 instead of real channel ID: ChannelManager will determine real channel ID.

 As all work is done asynchronously owner of ChannelManager instance can be notified that
 ChannelManager has completed its activity through callback passed as parameter to Stop method.
 In that callback it is safe to delete ChannelManager instance.
*/
class IOLoop;
struct IChannelListener;
class ChannelManager : public ITransportListener
                     , public IChannelSender
                     , private Noncopyable
{
private:
    struct ChannelBind
    {
        ITransport*       transport;
        uint32            channelId;
        IChannelListener* listener;

        friend bool operator < (const ChannelBind& left, const ChannelBind& right)
        {
            // Yes, your eyes don't deceive you: we compare pointers
            return left.transport == right.transport ? left.channelId < right.channelId
                                                     : left.transport < right.transport;
        }
    };
    typedef Vector<ChannelBind> BindListType;
    typedef Vector<ITransport*> TransportListType;
    typedef std::pair<size_t, size_t> RangeType;

public:
    typedef Function<void ()> StopHandlerType; 

public:
    ChannelManager(IOLoop* ioLoop);
    virtual ~ChannelManager();

    bool Start();
    void Stop(StopHandlerType handler = StopHandlerType());

    bool CreateTransport(eTransportType type, eTransportRole role, const Endpoint& endpoint, const uint32* channels, size_t channelCount);
    bool InstallListener(uint32 channelId, IChannelListener* listener);

    // IChannelSender
    virtual bool Send(IChannelListener* source, uint32 channelId, const uint8* buffer, size_t length, uint32* packetId);

    // ITransportListener
    virtual void OnTransportActivated(ITransport* transport);
    virtual void OnTransportDeactivated(ITransport* transport, eDeactivationReason reason, int32 error);
    virtual void OnTransportTerminated(ITransport* transport);
    virtual void OnTransportReceive(ITransport* transport, uint32 channelId, const uint8* buffer, size_t length);
    virtual void OnTransportSendComplete(ITransport* transport, uint32 channelId, const uint8* buffer, size_t length);
    virtual void OnTransportPacketDelivered(ITransport* transport, uint32 channelId, uint32 packetId);

private:
    ITransport* CreateTransport(eTransportType type, eTransportRole role, const Endpoint& endpoint);

    RangeType GetTransportChannels(ITransport* transport) const;
    ChannelBind* GetChannelBind(ITransport* transport, uint32 channelId);
    ChannelBind* GetChannelBind(uint32 channelId);
    ChannelBind* GetChannelBind(IChannelListener* listener);

private:
    IOLoop*           loop;
    bool              isRunning;
    int32             nonConfirmed;             // Number of transports left with not confirmed termination
    StopHandlerType   stopHandler;              // Handler to call when all transports have been terminated
    TransportListType transportList;            // List of transports
    BindListType      bindOrderByTransport;     // Bindings ordered by transport and channel ID
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_CHANNELMANAGER_H__
