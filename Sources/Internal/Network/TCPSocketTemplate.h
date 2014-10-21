#ifndef __DAVAENGINE_TCPSOCKETTEMPLATE_H__
#define __DAVAENGINE_TCPSOCKETTEMPLATE_H__

#include <cstring>

#include <libuv/uv.h>

#include <Debug/DVAssert.h>

#include "Endpoint.h"
#include "TCPSocketBase.h"

namespace DAVA {

class IOLoop;

/*
 Template class TCPSocketTemplate provides basic capabilities: reading from and sending to socket
 Template parameter T specifies type that inherits TCPSocketTemplate (CRTP idiom)
 Type specified by T should implement methods:
    void HandleConnect (int error)
        This method is called after connection to TCP server has been established
        Parameter error is non zero on error
    void HandleRead (int error, std::size_t nread, const uv_buf_t* buffer)
        This method is called after data with length of nread bytes has been arrived
        Parameter error is non zero on error, UV_EOF when remote peer has closed connection or 0 on no error
    template<typename WriteRequestType>
    void HandleWrite (WriteRequestType* request, int error)
        This method is called after data has been written to
    void HandleClose ()
        This method is called after underlying socket has been closed by libuv
*/
template <typename T>
class TCPSocketTemplate : public TCPSocketBase
{
public:
    typedef TCPSocketTemplate<T> ThisClassType;
    typedef T                    DerivedClassType;

public:
    TCPSocketTemplate (IOLoop* ioLoop, bool oneShotReadFlag) : TCPSocketBase (ioLoop), oneShotRead (oneShotReadFlag), externalReadBuffer (NULL), externalReadBufferSize (0)
    {
        DVASSERT (ioLoop);

        memset (&connectRequest, 0, sizeof (connectRequest));
        handle.data         = static_cast<DerivedClassType*> (this);
        connectRequest.data = static_cast<DerivedClassType*> (this);
    }

    ~TCPSocketTemplate () {}

    void Close ()
    {
        InternalClose (&HandleCloseThunk);
    }

    void StopAsyncRead ()
    {
        uv_read_stop (HandleAsStream ());
    }

    int LocalEndpoint (Endpoint& endpoint)
    {
        int size = endpoint.Size ();
        return uv_tcp_getsockname (Handle (), endpoint.CastToSockaddr (), &size);
    }

    int RemoteEndpoint (Endpoint& endpoint)
    {
        int size = endpoint.Size ();
        return uv_tcp_getpeername (Handle (), endpoint.CastToSockaddr (), &size);
    }

protected:
    int InternalAsyncConnect (const Endpoint& endpoint)
    {
        return uv_tcp_connect (0, Handle (), endpoint.CastToSockaddr (), &HandleConnectThunk);
    }

    void InternalAsyncRead (void* buffer, std::size_t size)
    {
        DVASSERT (buffer && size > 0);

        externalReadBuffer     = buffer;
        externalReadBufferSize = size;
        uv_read_start (HandleAsStream (), &HandleAllocThunk, &HandleReadThunk);
    }

    template<typename WriteRequestType>
    void InternalAsyncWrite (WriteRequestType* request, const void* buffer, std::size_t size)
    {
        DVASSERT (request && buffer && size > 0);

        memset (&request->request, 0, sizeof (request->request));
        request->pthis        = static_cast<DerivedClassType*> (this);
        request->buffer       = uv_buf_init (static_cast<char*> (const_cast<void*> (buffer)), size);    // uv_buf_init doesn't modify buffer
        request->request.data = request;

        uv_write (&request->request, HandleAsStream (), &request->buffer, 1, &HandleWriteThunk<WriteRequestType>);
    }

private:
    void HandleClose () {}
    void HandleAlloc (std::size_t /*suggested_size*/, uv_buf_t* buffer)
    {
        *buffer = uv_buf_init (static_cast<char*> (externalReadBuffer), externalReadBufferSize);
    }
    void HandleConnect (int /*error*/) {}
    void HandleRead (int /*error*/, std::size_t /*nread*/, const uv_buf_t* /*buffer*/) {}

    template<typename WriteRequestType>
    void HandleWrite (WriteRequestType* /*request*/, int /*error*/) {}

    static void HandleCloseThunk (uv_handle_t* handle)
    {
        DerivedClassType* pthis = static_cast<DerivedClassType*> (handle->data);
        pthis->HandleClose ();
    }

    static void HandleAllocThunk (uv_handle_t* handle, std::size_t suggested_size, uv_buf_t* buffer)
    {
        DerivedClassType* pthis = static_cast<DerivedClassType*> (handle->data);
        pthis->HandleAlloc (suggested_size, buffer);
    }

    static void HandleConnectThunk (uv_connect_t* request, int error)
    {
        DerivedClassType* pthis = static_cast<DerivedClassType*> (request->data);
        pthis->HandleConnect (error);
    }

    static void HandleReadThunk (uv_stream_t* handle, ssize_t nread, const uv_buf_t* buffer)
    {
        int error = 0;
        if (nread < 0)
        {
            error = nread;
            nread = 0;
        }
        DerivedClassType* pthis = static_cast<DerivedClassType*> (handle->data);
        pthis->HandleRead (error, nread, buffer);
        if (pthis->oneShotRead && 0 == error)
            pthis->StopAsyncRead ();
    }

    template<typename WriteRequestType>
    static void HandleWriteThunk (uv_write_t* request, int error)
    {
        WriteRequestType* req = static_cast<WriteRequestType*> (request->data);
        req->pthis->HandleWrite (req, error);
    }

protected:
    bool         oneShotRead;
    uv_connect_t connectRequest;
    void*        externalReadBuffer;
    std::size_t  externalReadBufferSize;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPSOCKETTEMPLATE_H__
