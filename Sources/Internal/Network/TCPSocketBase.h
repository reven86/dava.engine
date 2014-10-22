#ifndef __DAVAENGINE_TCPSOCKETBASE_H__
#define __DAVAENGINE_TCPSOCKETBASE_H__

#include <libuv/uv.h>

#include "Detail/Noncopyable.h"

namespace DAVA {

class IOLoop;
class Endpoint;

/*
 Class TCPSocketBase - base class for TCP sockets
*/
class TCPSocketBase : private DAVA::Noncopyable
{
public:
    explicit TCPSocketBase (IOLoop* ioLoop);

    IOLoop* Loop () { return loop; }

          uv_tcp_t* Handle ()       { return &handle; }
    const uv_tcp_t* Handle () const { return &handle; }

          uv_stream_t* HandleAsStream ()       { return reinterpret_cast<uv_stream_t*> (&handle); }
    const uv_stream_t* HandleAsStream () const { return reinterpret_cast<const uv_stream_t*> (&handle); }

          uv_handle_t* HandleAsHandle ()       { return reinterpret_cast<uv_handle_t*> (&handle); }
    const uv_handle_t* HandleAsHandle () const { return reinterpret_cast<const uv_handle_t*> (&handle); }

    bool IsClosed () const;

    int Bind (const Endpoint& endpoint);

    int Bind (const char* ipaddr, unsigned short port);

    int Bind (unsigned short port);

protected:
    void InternalClose (uv_close_cb callback);

    // Protected destructor to prevent deletion through this type
    ~TCPSocketBase () {}

protected:
    IOLoop*  loop;
    uv_tcp_t handle;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPSOCKETBASE_H__
