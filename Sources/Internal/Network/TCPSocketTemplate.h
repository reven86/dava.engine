#ifndef __DAVAENGINE_TCPSOCKETTEMPLATE_H__
#define __DAVAENGINE_TCPSOCKETTEMPLATE_H__

#include <libuv/uv.h>

#include <Debug/DVAssert.h>

#include "Endpoint.h"
#include "TCPSocketBase.h"

namespace DAVA {

class IOLoop;

/*
 Template class TCPSocketTemplate provides basic capabilities: reading from and sending to stream socket
 Template parameter T specifies type that inherits TCPSocketTemplate (CRTP idiom)
 Bool template parameter autoRead specifies read behaviour:
    when autoRead is true libuv automatically issues next read operations until StopAsyncRead is called
    when autoRead is false user should explicitly issue next read operation
 Multiple simultaneous read operations lead to undefined behaviour.

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

 Summary of methods that should be implemented by T:
    void HandleConnect (int error);
    void HandleRead (int error, std::size_t nread, const uv_buf_t* buffer);
    template<typename WriteRequestType>
    void HandleWrite (WriteRequestType* request, int error);
    void HandleClose ();
*/
template <typename T, bool autoRead = false>
class TCPSocketTemplate : public TCPSocketBase
{
public:
    typedef TCPSocketBase                  BaseClassType;
    typedef TCPSocketTemplate<T, autoRead> ThisClassType;
    typedef T                              DerivedClassType;

    static const bool autoReadFlag = autoRead;

protected:
    struct WriteRequestBase
    {
        DerivedClassType* pthis;
        uv_buf_t          buffer;
        uv_write_t        request;
    };

public:
    TCPSocketTemplate (IOLoop* ioLoop) : TCPSocketBase (ioLoop), connectRequest (), externalReadBuffer (NULL), externalReadBufferSize (0)
    {
        DVASSERT (ioLoop);

        handle.data         = static_cast<DerivedClassType*> (this);
        connectRequest.data = static_cast<DerivedClassType*> (this);
    }

    ~TCPSocketTemplate () {}

    void Close ()
    {
        BaseClassType::InternalClose (&HandleCloseThunk);
    }

    int StopAsyncRead ()
    {
        return uv_read_stop (HandleAsStream ());
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
        return uv_tcp_connect (&connectRequest, Handle (), endpoint.CastToSockaddr (), &HandleConnectThunk);
    }

    int InternalAsyncRead (void* buffer, std::size_t size)
    {
        DVASSERT (buffer && size > 0);

        externalReadBuffer     = buffer;
        externalReadBufferSize = size;
        return uv_read_start (HandleAsStream (), &HandleAllocThunk, &HandleReadThunk);
    }

    template<typename WriteRequestType>
    int InternalAsyncWrite (WriteRequestType* request, const void* buffer, std::size_t size)
    {
        /*
         WriteRequestType should have following public members:
            DerivedClassType* pthis   - pointer to DerivedClassType instance (can be pointer to void)
            uv_buf_t          buffer  - libuv buffer to write
            uv_write_t        request - libuv write request
        */
        DVASSERT (request && buffer && size > 0);

        request->pthis        = static_cast<DerivedClassType*> (this);
        request->buffer       = uv_buf_init (static_cast<char*> (const_cast<void*> (buffer)), size);    // uv_buf_init doesn't modify buffer
        request->request.data = request;

        return uv_write (&request->request, HandleAsStream (), &request->buffer, 1, &HandleWriteThunk<WriteRequestType>);
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
        pthis->CleanUpBeforeNextUse ();
        pthis->HandleClose ();
    }

    static void HandleAllocThunk (uv_handle_t* handle, std::size_t suggested_size, uv_buf_t* buffer)
    {
        DerivedClassType* pthis = static_cast<DerivedClassType*> (handle->data);
        pthis->HandleAlloc (suggested_size, buffer);
    }

    static void HandleConnectThunk (uv_connect_t* connectRequest, int error)
    {
        DerivedClassType* pthis = static_cast<DerivedClassType*> (connectRequest->data);
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
        if (!autoReadFlag && 0 == error)
            pthis->StopAsyncRead ();
        pthis->HandleRead (error, nread, buffer);
    }

    template<typename WriteRequestType>
    static void HandleWriteThunk (uv_write_t* writeRequest, int error)
    {
        WriteRequestType* request = static_cast<WriteRequestType*> (writeRequest->data);
        request->pthis->HandleWrite (request, error);
    }

protected:
    uv_connect_t connectRequest;
    void*        externalReadBuffer;
    std::size_t  externalReadBufferSize;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPSOCKETTEMPLATE_H__
