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

#ifndef __DAVAENGINE_ITRANSPORT_H__
#define __DAVAENGINE_ITRANSPORT_H__

#include "NetworkCommon.h"

namespace DAVA
{
namespace Net
{

/*
 Interface should be implemented by transports :)
*/
struct ITransport
{
    // Check whether transport is alive, i.e. can send/receive data
    virtual bool IsActive() const = 0;
    // Start transport
    virtual void Activate() = 0;
    // Shutdown transport
    virtual void Deactivate() = 0;
    // Send some data with given channel ID
    virtual void Send(uint32 channelId, const uint8* buffer, size_t length) = 0;
};

/*
 Interface should be implemented by objects which want to receive notification from transports.
 Main candidate is ChannelManager class.
*/
struct ITransportListener
{
    // Transport has been successfully started and can transfer data
    virtual void OnTransportActivated(ITransport* transport) = 0;
    // Transport has ended session by some reason
    virtual void OnTransportDeactivated(ITransport* transport, eDeactivationReason reason, int32 error) = 0;
    // Transport has been fully terminated and can be safely deleted
    virtual void OnTransportTerminated(ITransport* transport) = 0;
    // Transport has some data arrived from other side
    virtual void OnTransportReceive(ITransport* transport, uint32 channelId, const uint8* buffer, size_t length) = 0;
    // Buffer has been sent and client has a chance to free it or do something useful else
    virtual void OnTransportSendComplete(ITransport* transport, uint32 channelId, const uint8* buffer, size_t length) = 0;
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_ITRANSPORT_H__
