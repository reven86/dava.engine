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


#ifndef __DAVAENGINE_ICHANNEL_H__
#define __DAVAENGINE_ICHANNEL_H__

#include "NetworkCommon.h"

namespace DAVA
{
namespace Net
{

/*
 Interface should be implemented by objects which want to know what is going on in channel.
*/
struct IChannel;
struct IChannelListener
{
    // There should be a virtual destructor defined as objects may be deleted through this interface
    virtual ~IChannelListener() {}

    // Channel is open (underlying transport has connection) and can receive and send data through IChannel interface
    virtual void OnChannelOpen(IChannel* channel) = 0;
    // Channel is closed (underlying transport has disconnected) with reason
    virtual void OnChannelClosed(IChannel* channel, const char8* message) = 0;
    // Some data arrived into channel
    virtual void OnPacketReceived(IChannel* channel, const void* buffer, size_t length) = 0;
    // Buffer has been sent and can be reused or freed
    virtual void OnPacketSent(IChannel* channel, const void* buffer, size_t length) = 0;
    // Data packet with given ID has been delivered to other side
    virtual void OnPacketDelivered(IChannel* channel, uint32 packetId) = 0;
};

/*
 This interface is passed to IChannelListener methods to allow clients to send data to channels.
*/
class Endpoint;
struct IChannel
{
    virtual bool Send(const void* data, size_t length, uint32 flags, uint32* packetId) = 0;
    virtual const Endpoint& RemoteEndpoint() const = 0;
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_ICHANNEL_H__
