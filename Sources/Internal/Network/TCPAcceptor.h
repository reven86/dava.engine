#ifndef __DAVAENGINE_TCPACCEPTOR_H__
#define __DAVAENGINE_TCPACCEPTOR_H__

#include <Debug/DVAssert.h>
#include <Base/Function.h>

#include "TCPAcceptorTemplate.h"

namespace DAVA {

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
