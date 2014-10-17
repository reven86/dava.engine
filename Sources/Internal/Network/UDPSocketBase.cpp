#include <cstring>

#include <Debug/DVAssert.h>

#include "IOLoop.h"
#include "Endpoint.h"
#include "UDPSocketBase.h"

namespace DAVA {

UDPSocketBase::UDPSocketBase (IOLoop* ioLoop) : loop (ioLoop)
{
    DVASSERT (ioLoop);

    memset (&handle, 0, sizeof (handle));
    uv_udp_init (loop->Handle (), &handle);
}

bool UDPSocketBase::IsClosed () const
{
    return uv_is_closing (HandleAsHandle ()) ? true : false;
}

int UDPSocketBase::Bind (const Endpoint& endpoint)
{
    return uv_udp_bind (Handle (), endpoint.CastToSockaddr (), 0);
}

int UDPSocketBase::Bind (const char* ipaddr, unsigned short port)
{
    DVASSERT (ipaddr);

    Endpoint endpoint;
    int result = uv_ip4_addr (ipaddr, port, endpoint.CastToSockaddrIn ());
    if (result == 0)
        result = Bind (endpoint);
    return result;
}

int UDPSocketBase::Bind (unsigned short port)
{
    return Bind ("0.0.0.0", port);
}

}	// namespace DAVA
