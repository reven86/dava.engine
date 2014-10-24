#include <Debug/DVAssert.h>

#include "IOLoop.h"
#include "Endpoint.h"
#include "TCPSocketBase.h"

namespace DAVA {

TCPSocketBase::TCPSocketBase (IOLoop* ioLoop) : loop (ioLoop), handle ()
{
    DVASSERT (ioLoop);

    uv_tcp_init (loop->Handle (), &handle);
}

bool TCPSocketBase::IsClosed () const
{
    return uv_is_closing (HandleAsHandle ()) ? true : false;
}

int TCPSocketBase::Bind (const Endpoint& endpoint)
{
    return uv_tcp_bind (Handle (), endpoint.CastToSockaddr (), 0);
}

int TCPSocketBase::Bind (const char* ipaddr, unsigned short port)
{
    DVASSERT (ipaddr);

    Endpoint endpoint;
    int result = uv_ip4_addr (ipaddr, port, endpoint.CastToSockaddrIn ());
    if (0 == result)
        result = Bind (endpoint);
    return result;
}

int TCPSocketBase::Bind (unsigned short port)
{
    return Bind (Endpoint (port));
}

void TCPSocketBase::InternalClose (uv_close_cb callback)
{
    if (!IsClosed ())
        uv_close (HandleAsHandle (), callback);
}

void TCPSocketBase::CleanUpBeforeNextUse ()
{
    //DVASSERT (handle.socket == INVALID_SOCKET);
    uv_tcp_init (loop->Handle (), &handle);
}

}	// namespace DAVA
