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

struct IChannelListener;

/*
 Interface is implemented by ChannelManager to allow clients to send data to channels.
 Implementor should be able to determine actual channel ID by source (applied when 
 specified default channel ID - DEFAULT_CHANNEL_ID)
*/
struct IChannelSender
{
    virtual bool Send(IChannelListener* source, uint32 channelId, const uint8* buffer, size_t length) = 0;
};

/*
 Interface should be implemented by objects which want to know what is going on in channel.
*/
struct IChannelListener
{
    virtual void OnChannelOpen(uint32 channelId, IChannelSender* sender) = 0;
    virtual void OnChannelClosed(uint32 channelId, eDeactivationReason reason, int32 error) = 0;
    virtual void OnChannelReceive(uint32 channelId, const uint8* buffer, size_t length) = 0;
    virtual void OnChannelSendComplete(uint32 channelId, const uint8* buffer, size_t length) = 0;
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_ICHANNEL_H__
