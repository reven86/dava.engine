/*==================================================================================
    Copyright(c) 2008, binaryzebra
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

#include "IOLoop.h"
#include "Endpoint.h"
#include "UDPSocketBase.h"

namespace DAVA
{

UDPSocketBase::UDPSocketBase(IOLoop* ioLoop) : loop(ioLoop)
                                             , handle()
{
    DVASSERT(ioLoop != NULL);
    uv_udp_init(loop->Handle(), &handle);
}

bool UDPSocketBase::IsClosed() const
{
    return uv_is_closing(HandleAsHandle()) ? true : false;
}

std::size_t UDPSocketBase::SendQueueSize() const
{
    return handle.send_queue_size;
}

std::size_t UDPSocketBase::SendRequestCount() const
{
    return handle.send_queue_count;
}

int32 UDPSocketBase::Bind(const Endpoint& endpoint, bool reuseAddrOption)
{
    return uv_udp_bind(Handle(), endpoint.CastToSockaddr(), reuseAddrOption ? UV_UDP_REUSEADDR : 0);
}

int32 UDPSocketBase::Bind(const char8* ipaddr, int16 port, bool reuseAddrOption)
{
    DVASSERT(ipaddr != NULL);

    Endpoint endpoint;
    int result = uv_ip4_addr(ipaddr, port, endpoint.CastToSockaddrIn());
    if(0 == result)
    {
        result = Bind(endpoint, reuseAddrOption);
    }
    return result;
}

int32 UDPSocketBase::Bind(int16 port, bool reuseAddrOption)
{
    return Bind(Endpoint(port), reuseAddrOption);
}

int32 UDPSocketBase::JoinMulticastGroup(const char8* multicastAddr, const char8* interfaceAddr)
{
    DVASSERT(multicastAddr != NULL);
    return uv_udp_set_membership(Handle(), multicastAddr, interfaceAddr, UV_JOIN_GROUP);
}

int32 UDPSocketBase::LeaveMulticastGroup(const char8* multicastAddr, const char8* interfaceAddr)
{
    DVASSERT(multicastAddr != NULL);
    return uv_udp_set_membership(Handle(), multicastAddr, interfaceAddr, UV_LEAVE_GROUP);
}

void UDPSocketBase::InternalClose(uv_close_cb callback)
{
    if(!IsClosed())
    {
        uv_close(HandleAsHandle(), callback);
    }
}

void UDPSocketBase::CleanUpBeforeNextUse()
{
    uv_udp_init(loop->Handle(), &handle);
}

}	// namespace DAVA
