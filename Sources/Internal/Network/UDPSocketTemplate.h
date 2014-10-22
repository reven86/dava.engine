#ifndef __DAVAENGINE_UDPSOCKETTEMPLATE_H__
#define __DAVAENGINE_UDPSOCKETTEMPLATE_H__

#include "Endpoint.h"
#include "UDPSocketBase.h"

namespace DAVA {

class IOLoop;

/*
 Template class UDPSocketTemplate provides basic capabilities: reading from and sending to datagram socket
 Template parameter T specifies type that inherits UDPSocketTemplate (CRTP idiom)
 Bool template parameter autoRead specifies read behaviour:
    when autoRead is true libuv automatically issues next read operations until StopAsyncReceive is called
    when autoRead is false user should explicitly issue next read operation
 Multiple simultaneous read operations lead to undefined behaviour.

 Type specified by T should implement methods:
    void HandleReceive (int error, std::size_t nread, const uv_buf_t* buffer, const Endpoint& endpoint, bool partial)
        This method is called after data with length of nread bytes has been arrived
        Parameter error is non zero on error
        Parameter partial is true when read buffer was too small and rest of the message was discarded by OS
    template<typename SendRequestType>
    void HandleSend (SendRequestType* request, int error)
        This method is called after data has been sent
    void HandleClose ()
        This method is called after underlying socket has been closed by libuv

 Summary of methods that should be implemented by T:
    void HandleReceive (int error, std::size_t nread, const uv_buf_t* buffer, const Endpoint& endpoint, bool partial);
    template<typename SendRequestType>
    void HandleSend (SendRequestType* request, int error);
    void HandleClose ();
*/
template <typename T, bool autoRead = false>
class UDPSocketTemplate : public UDPSocketBase
{
public:
    typedef UDPSocketBase                  BaseClassType;
    typedef UDPSocketTemplate<T, autoRead> ThisClassType;
    typedef T                              DerivedClassType;

    static const bool autoReadFlag = autoRead;

protected:
    struct SendRequestBase
    {
        DerivedClassType* pthis;
        uv_buf_t          buffer;
        uv_udp_send_t     request;
    };

public:
    UDPSocketTemplate (IOLoop* ioLoop) : UDPSocketBase (ioLoop), externalReadBuffer (NULL), externalReadBufferSize (0)
    {
        handle.data = static_cast<DerivedClassType*> (this);
    }

    ~UDPSocketTemplate () {}

    void Close ()
    {
        BaseClassType::InternalClose (&HandleCloseThunk);
    }

    void StopAsyncReceive ()
    {
        uv_udp_recv_stop (Handle ());
    }

    int LocalEndpoint (Endpoint& endpoint)
    {
        int size = endpoint.Size ();
        return uv_udp_getsockname (Handle (), endpoint.CastToSockaddr (), &size);
    }

protected:
    void InternalAsyncReceive (void* buffer, std::size_t size)
    {
        DVASSERT (buffer && size > 0);

        externalReadBuffer     = buffer;
        externalReadBufferSize = size;
        uv_udp_recv_start (Handle (), &HandleAllocThunk, &HandleReceiveThunk);
    }

    template <typename SendRequestType>
    void InternalAsyncSend (SendRequestType* request, const void* buffer, std::size_t size, const Endpoint& endpoint)
    {
        /*
         SendRequestType should have following public members:
            DerivedClassType* pthis   - pointer to DerivedClassType instance (can be pointer to void)
            uv_buf_t          buffer  - libuv buffer to write
            uv_udp_send_t     request - libuv UDP send request
        */
        DVASSERT (request && buffer && size > 0);

        request->pthis        = static_cast<DerivedClassType*> (this);
        request->buffer       = uv_buf_init (static_cast<char*> (const_cast<void*> (buffer)), size);    // uv_buf_init doesn't modify buffer
        request->request.data = request;

        uv_udp_send (&request->request, Handle (), &request->buffer, 1, endpoint.CastToSockaddr (), &HandleSendThunk<SendRequestType>);
    }

private:
    void HandleClose () {}

    void HandleAlloc (std::size_t /*suggested_size*/, uv_buf_t* buffer)
    {
        *buffer = uv_buf_init (static_cast<char*> (externalReadBuffer), externalReadBufferSize);
    }

    void HandleReceive (int /*error*/, std::size_t /*nread*/, const uv_buf_t* /*buffer*/, const Endpoint& /*endpoint*/, bool /*partial*/) {}

    template<typename SendRequestType>
    void HandleSend (SendRequestType* /*request*/, int /*error*/) {}

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

    static void HandleReceiveThunk (uv_udp_t* handle, ssize_t nread, const uv_buf_t* buffer, const sockaddr* addr, unsigned int flags)
    {
        // According to libuv documentation under such conditions there is nothing to read
        if (0 == nread && NULL == addr)
            return;

        int error = 0;
        if (nread < 0)
        {
            error = nread;
            nread = 0;
        }
        DerivedClassType* pthis = static_cast<DerivedClassType*> (handle->data);
        pthis->HandleReceive (error, nread, buffer, Endpoint (addr), flags == UV_UDP_PARTIAL);
        if (!autoReadFlag && 0 == error)
            pthis->StopAsyncReceive ();
    }

    template <typename SendRequestType>
    static void HandleSendThunk (uv_udp_send_t* sendRequest, int error)
    {
        SendRequestType* request = static_cast<SendRequestType*> (sendRequest->data);
        request->pthis->HandleSend (request, error);
    }

protected:
    void*       externalReadBuffer;
    std::size_t externalReadBufferSize;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_UDPSOCKETTEMPLATE_H__
