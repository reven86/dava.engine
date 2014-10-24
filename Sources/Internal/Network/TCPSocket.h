#ifndef __DAVAENGINE_TCPSOCKET_H__
#define __DAVAENGINE_TCPSOCKET_H__

#include <Debug/DVAssert.h>
#include <Base/Function.h>

#include "TCPSocketTemplate.h"

namespace DAVA {

/*
 Class TCPSocket - fully functional TCP socket implementation which can be used in most cases.
 Can connect to remote socket, read and write data.
 User can provide functional object which is called on completion.
 Functional objects prototypes:
    ConnectHandlerType - called on connect operation completion
        void f (TCPSocket* socket, int error);
    ReadHandlerType - called on read operation completion
        void f (TCPSocket* socket, int error, std::size_t nread, void* buffer);
    WriteHandlerType - called on write operation completion
        void f (TCPSocket* socket, int error, const void* buffer);
 User is responsible for error processing.
*/
class TCPSocket : public TCPSocketTemplate<TCPSocket, false>
{
public:
    typedef TCPSocketTemplate<TCPSocket, false> BaseClassType;
    typedef TCPSocket                           ThisClassType;

    typedef DAVA::Function<void (ThisClassType* socket)>                                             CloseHandlerType;
    typedef DAVA::Function<void (ThisClassType* socket, int error)>                                  ConnectHandlerType;
    typedef DAVA::Function<void (ThisClassType* socket, int error, std::size_t nread, void* buffer)> ReadHandlerType;
    typedef DAVA::Function<void (ThisClassType* socket, int error, const void* buffer)>              WriteHandlerType;

    struct WriteRequest : public BaseClassType::WriteRequestBase
    {
        WriteRequest (WriteHandlerType handler) : BaseClassType::WriteRequestBase (), writeHandler (handler) {}
        WriteHandlerType writeHandler;
    };

public:
    explicit TCPSocket (IOLoop* ioLoop, bool autoDeleteOnCloseFlag = false);

    ~TCPSocket () {}

    template <typename Handler>
    void SetCloseHandler (Handler handler)
    {
        closeHandler = handler;
    }

    template <typename Handler>
    int AsyncConnect (const Endpoint& endpoint, Handler handler)
    {
        DVASSERT (!(handler == 0));

        connectHandler = handler;
        return BaseClassType::InternalAsyncConnect (endpoint);
    }

    template <typename Handler>
    int AsyncRead (void* buffer, std::size_t size, Handler handler)
    {
        DVASSERT (buffer && size && !(handler == 0));

        readHandler = handler;
        return BaseClassType::InternalAsyncRead (buffer, size);
    }

    template <typename Handler>
    int AsyncWrite (const void* buffer, std::size_t size, Handler handler)
    {
        DVASSERT (buffer && size && !(handler == 0));

        WriteRequest* request = new WriteRequest (handler);
        return BaseClassType::InternalAsyncWrite (request, buffer, size);
    }

    void HandleClose ();

    void HandleConnect (int error);

    void HandleRead (int error, size_t nread, const uv_buf_t* buffer);

    void HandleWrite (WriteRequest* request, int error);

private:
    bool               autoDeleteOnClose;   // TODO: do I really need this flag?
    CloseHandlerType   closeHandler;
    ConnectHandlerType connectHandler;
    ReadHandlerType    readHandler;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPSOCKET_H__
