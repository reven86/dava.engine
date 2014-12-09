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

#include <Base/Atomic.h>
#include <Debug/DVAssert.h>

#include <Network/Base/NetworkUtils.h>

#include "TCPTransport.h"
#include "ChannelManager.h"

namespace DAVA
{
namespace Net
{

ChannelManager::ChannelManager(IOLoop* ioLoop) : loop(ioLoop)
                                               , isRunning(false)
                                               , nonConfirmed(0)
                                               , stopHandler()
{

}

ChannelManager::~ChannelManager()
{
    DVASSERT(0 == nonConfirmed && false == isRunning);
    for (TransportListType::iterator i = transportList.begin(), e = transportList.end();i != e;++i)
        delete *i;
}

bool ChannelManager::Start()
{
    DVASSERT(false == transportList.empty() && false == isRunning);
    if (false == isRunning && false == transportList.empty())
    {
        isRunning    = true;
        nonConfirmed = static_cast<int32>(transportList.size());
        for (TransportListType::iterator i = transportList.begin(), e = transportList.end();i != e;++i)
            (*i)->Activate();
        return true;
    }
    return false;
}

void ChannelManager::Stop(StopHandlerType handler)
{
    if (isRunning)
    {
        stopHandler = handler;
        for (TransportListType::iterator i = transportList.begin(), e = transportList.end();i != e;++i)
            (*i)->Deactivate();
        isRunning = false;
    }
}

bool ChannelManager::CreateTransport(eTransportType type, eTransportRole role, const Endpoint& endpoint, const uint32* channels, size_t channelCount)
{
    DVASSERT(!isRunning && channels != NULL && channelCount > 0);
    if (isRunning || NULL == channels || 0 == channelCount)
        return false;

    // Simple check for:
    //  - valid channel ID
    //  - given channels do not overlap with channels for other transports
    for (size_t i = 0;i < channelCount;++i)
    {
        DVASSERT(channels[i] != DEFAULT_CHANNEL_ID && NULL == GetChannelBind(channels[i]));
        if (DEFAULT_CHANNEL_ID == channels[i] || GetChannelBind(channels[i]) != NULL)
            return false;
    }

    ITransport* transport = CreateTransport(type, role, endpoint);
    if (NULL == transport)
        return false;
    transportList.push_back(transport);

    // Add entry for each transport-channel pair
    bindOrderByTransport.reserve(bindOrderByTransport.size() + channelCount);
    for (size_t i = 0;i < channelCount;++i)
    {
        ChannelBind bind;
        bind.transport = transport;
        bind.channelId = channels[i];
        bind.listener  = NULL;
        bindOrderByTransport.push_back(bind);
    }
    //  and sort by transport and channel ID within channels belonging to each transport
    std::sort(bindOrderByTransport.begin(), bindOrderByTransport.end());
    return true;
}

bool ChannelManager::InstallListener(uint32 channelId, IChannelListener* listener)
{
    // Listeners can be installed before on ChannelManager that is not running
    DVASSERT(!isRunning && channelId != DEFAULT_CHANNEL_ID && listener != NULL);
    if (!isRunning && channelId != DEFAULT_CHANNEL_ID && listener != NULL)
    {
        ChannelBind* bind = GetChannelBind(channelId);
        DVASSERT(bind != NULL && NULL == bind->listener);
        // And listeners cannot replace already installed other listener
        if (bind != NULL && NULL == bind->listener)
        {
            bind->listener = listener;
            return true;
        }
    }
    return false;
}

bool ChannelManager::Send(IChannelListener* source, uint32 channelId, const uint8* buffer, size_t length, uint32* packetId)
{
    DVASSERT(isRunning && source != NULL && buffer != NULL && length > 0);

    // If default channel specified find any channel for source listener
    ChannelBind* bind = DEFAULT_CHANNEL_ID == channelId ? GetChannelBind(source)
                                                        : GetChannelBind(channelId);
    DVASSERT(bind != NULL && bind->listener == source);
    // Simple check that only registered listener can send to channel
    if (bind && bind->listener == source)
    {
        bind->transport->Send(bind->channelId, buffer, length, packetId);
        return true;
    }
    return false;
}

void ChannelManager::OnTransportActivated(ITransport* transport)
{
    RangeType range = GetTransportChannels(transport);
    DVASSERT(range.first < range.second);

    // Notify all channels bound to transport
    for (size_t i = range.first;i != range.second;++i)
    {
        if (bindOrderByTransport[i].listener)
            bindOrderByTransport[i].listener->OnChannelOpen(bindOrderByTransport[i].channelId, this);
    }
}

void ChannelManager::OnTransportDeactivated(ITransport* transport, eDeactivationReason reason, int32 error)
{
    RangeType range = GetTransportChannels(transport);
    DVASSERT(range.first < range.second);

    // Notify all channels bound to transport
    for (size_t i = range.first;i != range.second;++i)
    {
        if (bindOrderByTransport[i].listener)
            bindOrderByTransport[i].listener->OnChannelClosed(bindOrderByTransport[i].channelId, reason, error);
    }
}

void ChannelManager::OnTransportTerminated(ITransport* transport)
{
    // When nonConfirmed reaches zero we can believe that everything has been stopped
    // and we are ready to die through, e.g. delete
    AtomicDecrement(nonConfirmed);
    isRunning = nonConfirmed > 0;
    DVASSERT(nonConfirmed >= 0);
    if (0 == nonConfirmed && stopHandler != 0)
        stopHandler();  // Say bye to beloved owner
}

void ChannelManager::OnTransportReceive(ITransport* transport, uint32 channelId, const uint8* buffer, size_t length)
{
    ChannelBind* bind = GetChannelBind(transport, channelId);
    DVASSERT(bind != NULL);
    if (bind && bind->listener)
    {
        bind->listener->OnChannelReceive(channelId, buffer, length);
    }
}

void ChannelManager::OnTransportSendComplete(ITransport* transport, uint32 channelId, const uint8* buffer, size_t length)
{
    ChannelBind* bind = GetChannelBind(transport, channelId);
    DVASSERT(bind != NULL);
    if (bind && bind->listener)
    {
        bind->listener->OnChannelSendComplete(channelId, buffer, length);
    }
}

void ChannelManager::OnTransportPacketDelivered(ITransport* transport, uint32 channelId, uint32 packetId)
{
    ChannelBind* bind = GetChannelBind(transport, channelId);
    DVASSERT(bind != NULL);
    if (bind && bind->listener)
    {
        bind->listener->OnChannelDelivered(channelId, packetId);
    }
}

ITransport* ChannelManager::CreateTransport(eTransportType type, eTransportRole role, const Endpoint& endpoint)
{
    switch(type)
    {
    case TRANSPORT_TCP:
        return new TCPTransport(loop, this, role, endpoint);
    case TRANSPORT_RDP:
        DVASSERT(0 && "TRANSPORT_RDP not yet implemented");
        return NULL;
    case TRANSPORT_UDP:
        DVASSERT(0 && "TRANSPORT_UDP not yet implemented");
        return NULL;
    default:
        DVASSERT(0 && "Unknown transport type");
        return NULL;
    }
}

ChannelManager::RangeType ChannelManager::GetTransportChannels(ITransport* transport) const
{
    DVASSERT(transport != NULL);
    RangeType result;
    result.first  = 0;
    result.second = 0;
    size_t count  = bindOrderByTransport.size();
    // NOTE: bindOrderByTransport is sorted by transport and transport's channelID
    for(;result.first < count && bindOrderByTransport[result.first].transport != transport;++result.first);
    result.second = result.first + 1;
    for(;result.second < count && bindOrderByTransport[result.second].transport == transport;++result.second);
    return result;
}

ChannelManager::ChannelBind* ChannelManager::GetChannelBind(ITransport* transport, uint32 channelId)
{
    // TODO: linear search, need to improve
    DVASSERT(transport != NULL && channelId != 0);
    RangeType range = GetTransportChannels(transport);
    if (range.first < range.second)
    {
        for (size_t i = range.first;i < range.second;++i)
        {
            if (bindOrderByTransport[i].channelId == channelId)
                return &bindOrderByTransport[i];
        }
    }
    return NULL;
}

ChannelManager::ChannelBind* ChannelManager::GetChannelBind(uint32 channelId)
{
    // TODO: linear search, need to improve
    DVASSERT(channelId != 0);
    for (size_t i = 0, n = bindOrderByTransport.size();i != n;++i)
    {
        if (bindOrderByTransport[i].channelId == channelId)
            return &bindOrderByTransport[i];
    }
    return NULL;
}

ChannelManager::ChannelBind* ChannelManager::GetChannelBind(IChannelListener* listener)
{
    // TODO: linear search, need to improve
    DVASSERT(listener != NULL);
    for (size_t i = 0, n = bindOrderByTransport.size();i != n;++i)
    {
        if (bindOrderByTransport[i].listener == listener)
            return &bindOrderByTransport[i];
    }
    return NULL;
}

}   // namespace Net
}   // namespace DAVA
