#ifndef __DAVAENGINE_UDPSOCKETBASE_H__
#define __DAVAENGINE_UDPSOCKETBASE_H__

#include <libuv/uv.h>

#include "Detail/Noncopyable.h"

namespace DAVA {

class IOLoop;
class Endpoint;

/*
 Class UDPSocketBase - base class for UDP sockets
*/
class UDPSocketBase : private DAVA::Noncopyable
{
public:
    explicit UDPSocketBase (IOLoop* ioLoop);

    IOLoop* Loop () { return loop; }

          uv_udp_t* Handle ()       { return &handle; }
    const uv_udp_t* Handle () const { return &handle; }

          uv_handle_t* HandleAsHandle ()       { return reinterpret_cast<uv_handle_t*> (&handle); }
    const uv_handle_t* HandleAsHandle () const { return reinterpret_cast<const uv_handle_t*> (&handle); }

    bool IsClosed () const;

    int Bind (const Endpoint& endpoint, bool reuseAddrOption = false);

    int Bind (const char* ipaddr, unsigned short port, bool reuseAddrOption = false);

    int Bind (unsigned short port, bool reuseAddrOption = false);

    int JoinMulticastGroup (const char* multicastAddr, const char* interfaceAddr = NULL);

    int LeaveMulticastGroup (const char* multicastAddr, const char* interfaceAddr = NULL);

protected:
    void InternalClose (uv_close_cb callback)
    {
        if (!IsClosed ())
            uv_close (HandleAsHandle (), callback);
    }

    // Protected destructor to prevent deletion through this type
    ~UDPSocketBase () {}

protected:
    IOLoop*  loop;
    uv_udp_t handle;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_UDPSOCKETBASE_H__
