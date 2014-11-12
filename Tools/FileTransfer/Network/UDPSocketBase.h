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

#ifndef __DAVAENGINE_UDPSOCKETBASE_H__
#define __DAVAENGINE_UDPSOCKETBASE_H__

#include <libuv/uv.h>

#include <Base/BaseTypes.h>
#include <Base/Noncopyable.h>

namespace DAVA
{

class IOLoop;
class Endpoint;

/*
 Class UDPSocketBase - base class for UDP sockets
*/
class UDPSocketBase : private Noncopyable
{
public:
    IOLoop* Loop() { return loop; }

    uv_udp_t* Handle();
    const uv_udp_t* Handle() const;

    uv_handle_t* HandleAsHandle();
    const uv_handle_t* HandleAsHandle() const;

    bool IsClosed() const;

    std::size_t SendQueueSize() const;
    std::size_t SendRequestCount() const;

    int32 Bind(const Endpoint& endpoint, bool reuseAddrOption = false);
    int32 Bind(const char8* ipaddr, int16 port, bool reuseAddrOption = false);
    int32 Bind(int16 port, bool reuseAddrOption = false);

    int32 JoinMulticastGroup(const char8* multicastAddr, const char8* interfaceAddr = NULL);
    int32 LeaveMulticastGroup(const char8* multicastAddr, const char8* interfaceAddr = NULL);

protected:
    void InternalClose(uv_close_cb callback);
    void CleanUpBeforeNextUse();

    // Protected constructor and destructor to prevent creation and deletion through this type
    UDPSocketBase(IOLoop* ioLoop);
    ~UDPSocketBase() {}

protected:
    IOLoop*  loop;
    uv_udp_t handle;
};

//////////////////////////////////////////////////////////////////////////
inline uv_udp_t* UDPSocketBase::Handle() { return &handle; }
inline const uv_udp_t* UDPSocketBase::Handle() const { return &handle; }

inline uv_handle_t* UDPSocketBase::HandleAsHandle() { return reinterpret_cast<uv_handle_t*>(&handle); }
inline const uv_handle_t* UDPSocketBase::HandleAsHandle() const { return reinterpret_cast<const uv_handle_t*>(&handle); }

}	// namespace DAVA

#endif  // __DAVAENGINE_UDPSOCKETBASE_H__
