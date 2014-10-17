#ifndef __DAVAENGINE_TCPSOCKET_H__
#define __DAVAENGINE_TCPSOCKET_H__

#include <Debug/DVAssert.h>
#include <Base/Function.h>

#include "TCPSocketTemplate.h"

namespace DAVA {

class TCPSocket : public TCPSocketTemplate<TCPSocket>
{
public:
    typedef TCPSocketTemplate<TCPSocket> BaseClassType;
    typedef TCPSocket                    ThisClassType;

    typedef DAVA::Function<void (ThisClassType* socket, int error)>                                  ConnectHandlerType;
    typedef DAVA::Function<void (ThisClassType* socket, int error, std::size_t nread, void* buffer)> ReadHandlerType;
    typedef DAVA::Function<void (ThisClassType* socket, int error, const void* buffer)>              WriteHandlerType;

    struct WriteRequest
    {
        DerivedClassType* pthis;
        uv_write_t        request;
        uv_buf_t          buffer;
        WriteHandlerType  writeHandler;
    };

public:
    explicit TCPSocket (IOLoop* ioLoop, bool oneShotReadFlag, bool autoDeleteOnCloseFlag) : BaseClassType (ioLoop, oneShotReadFlag), autoDeleteOnClose (autoDeleteOnClose)
    {
        DVASSERT (ioLoop);
    }

    ~TCPSocket () {}

    template <typename Handler>
    void AsyncConnect (const Endpoint& endpoint, Handler handler)
    {
        DVASSERT (!(handler == 0));

        connectHandler = handler;
        InternalAsyncConnect (endpoint);
    }

    template <typename Handler>
    void AsyncRead (void* buffer, std::size_t size, Handler handler)
    {
        DVASSERT (buffer && size && !(handler == 0));

        readHandler = handler;
        InternalAsyncRead (buffer, size);
    }

    template <typename Handler>
    void AsyncWrite (const void* buffer, std::size_t size, Handler handler)
    {
        DVASSERT (buffer && size && !(handler == 0));

        writeRequest.writeHandler = handler;
        InternalAsyncWrite (&writeRequest, buffer, size);
    }

    void HandleClose ()
    {
        if (autoDeleteOnClose)
            delete this;
    }

    void HandleRead (int error, size_t nread, const uv_buf_t* buffer)
    {
        readHandler (this, error, nread, buffer->base);
    }

    template<typename RequestType>
    void HandleWrite (RequestType* request, int error)
    {
        request->writeHandler (this, error, request->buffer.base);
    }

private:
    bool               autoDeleteOnClose;   // TODO: do I really need this flag?
    ConnectHandlerType connectHandler;
    ReadHandlerType    readHandler;
    WriteRequest       writeRequest;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPSOCKET_H__
