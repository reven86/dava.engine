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


#ifndef __DAVAENGINE_NETSERVICE_H__
#define __DAVAENGINE_NETSERVICE_H__

#include "Base/BaseTypes.h"
#include "Network/IChannel.h"

namespace DAVA
{
namespace Net
{

class NetService : public IChannelListener
{
public:
    NetService();
    virtual ~NetService() {}

    // IChannelListener
    void OnChannelOpen(IChannel* aChannel) override;
    void OnChannelClosed(IChannel* aChannel, const char8* message) override;
    void OnPacketReceived(IChannel* aChannel, const void* buffer, size_t length) override;
    void OnPacketSent(IChannel* aChannel, const void* buffer, size_t length) override;
    void OnPacketDelivered(IChannel* aChannel, uint32 packetId) override;

    virtual void ChannelOpen() {}
    virtual void ChannelClosed(const char8* message) {}
    virtual void PacketReceived(const void* packet, size_t length) {}
    virtual void PacketSent() {}
    virtual void PacketDelivered() {}

protected:
    bool IsChannelOpen() const;
    bool Send(const void* data, size_t length, uint32* packetId = NULL);
    template<typename T>
    bool Send(const T* value, uint32* packetId = NULL);

protected:
    IChannel* channel;
};

//////////////////////////////////////////////////////////////////////////
inline NetService::NetService() : channel(NULL)
{}

inline bool NetService::IsChannelOpen() const
{
    return channel != NULL;
}

template<typename T>
inline bool NetService::Send(const T* value, uint32* packetId)
{
    return Send(value, sizeof(T), packetId);
}

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_NETSERVICE_H__
