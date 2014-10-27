#ifndef __DAVAENGINE_TCPACCEPTOR_H__
#define __DAVAENGINE_TCPACCEPTOR_H__

#include <Debug/DVAssert.h>
#include <Base/Function.h>

#include "TCPAcceptorTemplate.h"

namespace DAVA {

/*
 Class TCPAcceptor - fully functional implementation which can be used in most cases.
 This class provides ability to call user-specified functional object on incoming TCP connection event
 Functional objects prototypes:
    void ConnectHandlerType (TCPAcceptor* acceptor, int error)
*/
class TCPAcceptor : public TCPAcceptorTemplate<TCPAcceptor>
{
public:
    typedef TCPAcceptorTemplate<TCPAcceptor> BaseClassType;
    typedef TCPAcceptor                      ThisClassType;

    typedef DAVA::Function<void (ThisClassType* acceptor)>            CloseHandlerType;
	typedef DAVA::Function<void (ThisClassType* acceptor, int error)> ConnectHandlerType;

public:
    explicit TCPAcceptor (IOLoop* ioLoop, bool autoDeleteOnCloseFlag = false);

    ~TCPAcceptor () {}

    template <typename Handler>
    void SetCloseHandler (Handler handler)
    {
        closeHandler = handler;
    }

    template <typename Handler>
    int AsyncListen (Handler handler, int backlog = SOMAXCONN)
    {
        DVASSERT (!(handler == 0));

        connectHandler = handler;
        return InternalAsyncListen (backlog);
    }

    void HandleClose ();

    void HandleConnect (int error);

private:
    bool               autoDeleteOnClose;   // TODO: do I really need this flag?
    CloseHandlerType   closeHandler;
    ConnectHandlerType connectHandler;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPACCEPTOR_H__
