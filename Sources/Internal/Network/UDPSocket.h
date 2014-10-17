#ifndef __DAVAENGINE_UDPSOCKET_H__
#define __DAVAENGINE_UDPSOCKET_H__

#include <Debug/DVAssert.h>
#include <Base/Function.h>

#include "UDPSocketTemplate.h"

namespace DAVA {

class UDPSocket : public UDPSocketTemplate<UDPSocket>
{
public:
    typedef UDPSocketTemplate<UDPSocket> BaseClassType;
    typedef UDPSocket                    ThisClassType;

    //typedef DAVA::Function<void (ThisClassType* socket, int error)>                                                                                ConnectHandlerType;
    typedef DAVA::Function<void (ThisClassType* socket, int error, std::size_t nread, void* buffer, const Endpoint& endpoint, unsigned int flags)> ReceiveHandlerType;
    typedef DAVA::Function<void (ThisClassType* socket, int error, const void* buffer)>                                                            SendHandlerType;

    struct SendRequest
    {
        DerivedClassType* pthis;
        uv_udp_send_t     request;
        uv_buf_t          buffer;
        SendHandlerType   sendHandler;
    };

public:
    explicit UDPSocket (IOLoop* ioLoop, bool oneShotReadFlag, bool autoDeleteOnCloseFlag) : BaseClassType (ioLoop, oneShotReadFlag), autoDeleteOnClose (autoDeleteOnClose)
    {
        DVASSERT (ioLoop);
    }

    ~UDPSocket () {}

    template <typename Handler>
    void AsyncReceive (void* buffer, std::size_t size, Handler handler)
    {
        DVASSERT (buffer && size && !(handler == 0));

        receiveHandler = handler;
        InternalAsyncReceive (buffer, size);
    }

    template <typename Handler>
    void AsyncSend (const Endpoint& endpoint, const void* buffer, std::size_t size, Handler handler)
    {
        DVASSERT (buffer && size && !(handler == 0));

        sendRequest.sendHandler = handler;
        InternalAsyncSend (&sendRequest, buffer, size, endpoint);
    }

    void HandleReceive (int error, std::size_t nread, const uv_buf_t* buffer, const Endpoint& endpoint, unsigned int flags)
    {
        receiveHandler (this, error, nread, buffer->base, endpoint, flags);
    }

    template<typename SendRequestType>
    void HandleSend (SendRequestType* request, int error)
    {
        request->sendHandler (this, error, request->buffer.base);
    }

private:
    bool               autoDeleteOnClose;   // TODO: do I really need this flag?
    //ConnectHandlerType connectHandler;
    ReceiveHandlerType receiveHandler;
    SendRequest        sendRequest;
};

}   // namespace DAVA

#endif  // __DAVAENGINE_UDPSOCKET_H__
