#include <Debug/DVAssert.h>

#include "IOLoop.h"
#include "Endpoint.h"
#include "UDPSocketBase.h"

namespace DAVA {

UDPSocketBase::UDPSocketBase (IOLoop* ioLoop) : loop (ioLoop), handle ()
{
    DVASSERT (ioLoop);

    uv_udp_init (loop->Handle (), &handle);
}

bool UDPSocketBase::IsClosed () const
{
    return uv_is_closing (HandleAsHandle ()) ? true : false;
}

int UDPSocketBase::Bind (const Endpoint& endpoint, bool reuseAddrOption)
{
    return uv_udp_bind (Handle (), endpoint.CastToSockaddr (), reuseAddrOption ? UV_UDP_REUSEADDR : 0);
}

int UDPSocketBase::Bind (const char* ipaddr, unsigned short port, bool reuseAddrOption)
{
    DVASSERT (ipaddr);

    Endpoint endpoint;
    int result = uv_ip4_addr (ipaddr, port, endpoint.CastToSockaddrIn ());
    if (result == 0)
        result = Bind (endpoint, reuseAddrOption);
    return result;
}

int UDPSocketBase::Bind (unsigned short port, bool reuseAddrOption)
{
    return Bind (Endpoint (port), reuseAddrOption);
}

int UDPSocketBase::JoinMulticastGroup (const char* multicastAddr, const char* interfaceAddr)
{
    DVASSERT (multicastAddr);
    return uv_udp_set_membership (Handle (), multicastAddr, interfaceAddr, UV_JOIN_GROUP);
}

int UDPSocketBase::LeaveMulticastGroup (const char* multicastAddr, const char* interfaceAddr)
{
    DVASSERT (multicastAddr);
    return uv_udp_set_membership (Handle (), multicastAddr, interfaceAddr, UV_LEAVE_GROUP);
}

void UDPSocketBase::InternalClose (uv_close_cb callback)
{
    if (!IsClosed ())
        uv_close (HandleAsHandle (), callback);
}

}	// namespace DAVA
