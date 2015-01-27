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

#include <Debug/DVAssert.h>

#include "NetService.h"

namespace DAVA
{
namespace Net
{

void NetService::OnChannelOpen(IChannel* aChannel)
{
    DVASSERT(NULL == channel);
    channel = aChannel;
    ChannelOpen();
}

void NetService::OnChannelClosed(IChannel* aChannel, const char8* message)
{
    // OnChannelClosed can be called without corresponding OnChannelOpen, e.g. when remote service is unavailable
    DVASSERT(NULL == channel || channel == aChannel);
    channel = NULL;
    ChannelClosed(message);
}

void NetService::OnPacketReceived(IChannel* aChannel, const void* buffer, size_t length)
{
    DVASSERT(channel == aChannel);
    PacketReceived(buffer, length);
}

void NetService::OnPacketSent(IChannel* aChannel, const void* buffer, size_t length)
{
    // If channel is NULL then OnChannelClosed has been called already
    DVASSERT(NULL == channel || channel == aChannel);
    PacketSent();
}

void NetService::OnPacketDelivered(IChannel* aChannel, uint32 packetId)
{
    DVASSERT(channel == aChannel);
    PacketDelivered();
}

bool NetService::Send(const void* data, size_t length, uint32* packetId)
{
    DVASSERT(data != NULL && length > 0 && true == IsChannelOpen());
    return IsChannelOpen() ? channel->Send(data, length, 0, packetId)
                           : false;
}

}   // namespace Net
}   // namespace DAVA
