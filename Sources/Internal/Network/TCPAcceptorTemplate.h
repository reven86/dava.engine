#ifndef __DAVAENGINE_TCPACCEPTORTEMPLATE_H__
#define __DAVAENGINE_TCPACCEPTORTEMPLATE_H__

#include <Debug/DVAssert.h>

#include "Endpoint.h"
#include "TCPSocketBase.h"

namespace DAVA {

class IOLoop;

/*
 Template class TCPAcceptorTemplate accepts incoming TCP connections.
 Template parameter T specifies type that inherits TCPAcceptorTemplate (CRTP idiom)
 Type specified by T should implement methods:
    void HandleConnect (int error)
        This method is called on recieving new incoming TCP connection.
        Parameter error is non zero on error
    void HandleClose () - optional
        This method is called after underlying socket has been closed by libuv
*/
template <typename T>
class TCPAcceptorTemplate : public TCPSocketBase
{
public:
    typedef TCPAcceptorTemplate<T> ThisClassType;
    typedef T                      DerivedClassType;

public:
    explicit TCPAcceptorTemplate (IOLoop* ioLoop) : TCPSocketBase (ioLoop)
    {
        handle.data = static_cast<DerivedClassType*> (this);
    }

    ~TCPAcceptorTemplate () {}

    void Close ()
    {
        InternalClose (&HandleCloseThunk);
    }

    int Accept (TCPSocketBase* socket)
    {
        DVASSERT (socket);
        return uv_accept (HandleAsStream (), socket->HandleAsStream ());
    }

protected:
    int InternalAsyncListen (int backlog)
    {
        DVASSERT (backlog > 0);
        return uv_listen (HandleAsStream (), backlog, &HandleConnectThunk);
    }

private:
    void HandleClose () {}

    void HandleConnect (int /*error*/) {}

    static void HandleCloseThunk (uv_handle_t* handle)
    {
        DerivedClassType* pthis = static_cast<DerivedClassType*> (handle->data);
        pthis->HandleClose ();
    }

    static void HandleConnectThunk (uv_stream_t* handle, int error)
    {
        DerivedClassType* pthis = static_cast<DerivedClassType*> (handle->data);
        pthis->HandleConnect (error);
    }
};

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPACCEPTORTEMPLATE_H__
