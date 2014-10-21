#ifndef __DAVAENGINE_TCPACCEPTOR_H__
#define __DAVAENGINE_TCPACCEPTOR_H__

#include <Debug/DVAssert.h>
#include <Base/Function.h>

#include "TCPAcceptorTemplate.h"

namespace DAVA {

/*
 Class TCPAcceptor
 This class provides ability to call user-specified functional object on incoming TCP connection event
 Handler should have prototype:
    void ConnectHandler (TCPAcceptor* acceptor, int error)
    where
        acceptor - pointer to TCPAcceptor object which listens incoming TCP connections
        error    - non zero on error
 Handler can be free-standing C function, static member function, or functional object created by DAVA::Function and DAVA::Bind
*/
class TCPAcceptor : public TCPAcceptorTemplate<TCPAcceptor>
{
public:
    typedef TCPAcceptorTemplate<TCPAcceptor> BaseClassType;
    typedef TCPAcceptor                      ThisClassType;

	typedef DAVA::Function<void (ThisClassType* acceptor, int error)> ConnectHandlerType;

public:
    explicit TCPAcceptor (IOLoop* ioLoop) : BaseClassType (ioLoop) {}

    ~TCPAcceptor () {}

    template <typename Handler>
    int AsyncListen (Handler handler, int backlog = SOMAXCONN)
    {
        DVASSERT (!(handler == 0));

        connectHandler = handler;
        return InternalAsyncListen (backlog);
    }

    void HandleConnect (int error)
    {
        connectHandler (this, error);
    }

private:
    ConnectHandlerType connectHandler;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPACCEPTOR_H__
