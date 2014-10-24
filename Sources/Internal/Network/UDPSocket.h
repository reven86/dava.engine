#ifndef __DAVAENGINE_UDPSOCKET_H__
#define __DAVAENGINE_UDPSOCKET_H__

#include <Debug/DVAssert.h>
#include <Base/Function.h>

#include "UDPSocketTemplate.h"

namespace DAVA {

/*
 Class UDPSocket - fully functional UDP socket implementation which can be used in most cases.
 Can receiev from and send data t socket.
 User can provide functional object which is called on completion.
 Functional objects prototypes:
    ReceiveHandlerType - called on read operation completion
        void f (UDPSocket* socket, int error, std::size_t nread, void* buffer, const Endpoint& endpoint, bool partial);
    SendHandlerType - called on write operation completion
        void f (UDPSocket* socket, int error, const void* buffer);
 User is responsible for error processing.
*/

class UDPSocket : public UDPSocketTemplate<UDPSocket, false>
{
public:
    typedef UDPSocketTemplate<UDPSocket, false> BaseClassType;
    typedef UDPSocket                           ThisClassType;

    typedef DAVA::Function<void (ThisClassType* socket)>                                                                                     CloseHandlerType;
    typedef DAVA::Function<void (ThisClassType* socket, int error, std::size_t nread, void* buffer, const Endpoint& endpoint, bool partial)> ReceiveHandlerType;
    typedef DAVA::Function<void (ThisClassType* socket, int error, const void* buffer)>                                                      SendHandlerType;

    struct SendRequest : public BaseClassType::SendRequestBase
    {
        SendRequest (SendHandlerType handler) : BaseClassType::SendRequestBase (), sendHandler (handler) {}
        SendHandlerType sendHandler;
    };

public:
    explicit UDPSocket (IOLoop* ioLoop, bool autoDeleteOnCloseFlag = false);

    ~UDPSocket () {}

    template <typename Handler>
    void SetCloseHandler (Handler handler)
    {
        closeHandler = handler;
    }

    template <typename Handler>
    int AsyncReceive (void* buffer, std::size_t size, Handler handler)
    {
        DVASSERT (buffer && size && !(handler == 0));

        receiveHandler = handler;
        return BaseClassType::InternalAsyncReceive (buffer, size);
    }

    template <typename Handler>
    int AsyncSend (const Endpoint& endpoint, const void* buffer, std::size_t size, Handler handler)
    {
        DVASSERT (buffer && size && !(handler == 0));

        SendRequest* request = new SendRequest (handler);
        return BaseClassType::InternalAsyncSend (request, buffer, size, endpoint);
    }

    void HandleClose ();

    void HandleReceive (int error, std::size_t nread, const uv_buf_t* buffer, const Endpoint& endpoint, bool partial);

    void HandleSend (SendRequest* request, int error);

private:
    bool               autoDeleteOnClose;   // TODO: do I really need this flag?
    CloseHandlerType   closeHandler;
    ReceiveHandlerType receiveHandler;
};

}   // namespace DAVA

#endif  // __DAVAENGINE_UDPSOCKET_H__
