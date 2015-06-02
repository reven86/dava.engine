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

#include <Network/Base/Buffer.h>
#include <Network/Base/Endpoint.h>
#include <Network/NetworkCommon.h>

namespace DAVA
{
namespace Net
{

struct IClientTransport;
struct IServerListener;

struct IServerTransport
{
    virtual ~IServerTransport() {}

    virtual int32 Start(IServerListener* listener) = 0;
    virtual void Stop() = 0;
    virtual void Reset() = 0;
    virtual void ReclaimClient(IClientTransport* client) = 0;
};

struct IServerListener
{
    virtual void OnTransportSpawned(IServerTransport* parent, IClientTransport* child) = 0;
    virtual void OnTransportTerminated(IServerTransport* tr) = 0;
};

//////////////////////////////////////////////////////////////////////////
struct IClientListener;

struct IClientTransport
{
    virtual ~IClientTransport() {}

    virtual int32 Start(IClientListener* listener) = 0;
    virtual void Stop() = 0;
    virtual void Reset() = 0;
    virtual int32 Send(const Buffer* buffers, size_t bufferCount) = 0;
};

struct IClientListener
{
    virtual void OnTransportTerminated(IClientTransport* tr) = 0;
    virtual void OnTransportConnected(IClientTransport* tr, const Endpoint& endp) = 0;
    virtual void OnTransportDisconnected(IClientTransport* tr, int32 error) = 0;
    virtual void OnTransportDataReceived(IClientTransport* tr, const void* buffer, size_t length) = 0;
    virtual void OnTransportSendComplete(IClientTransport* tr) = 0;
    virtual void OnTransportReadTimeout(IClientTransport* tr) = 0;
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_ITRANSPORT_H__
