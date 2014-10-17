#ifndef __DAVAENGINE_UDPSOCKETTEMPLATE_H__
#define __DAVAENGINE_UDPSOCKETTEMPLATE_H__

#include "Endpoint.h"
#include "UDPSocketBase.h"

namespace DAVA {

class IOLoop;

template <typename T>
class UDPSocketTemplate : public UDPSocketBase
{
public:
    typedef UDPSocketTemplate<T> ThisClassType;
    typedef T                    DerivedClassType;

public:
    UDPSocketTemplate (IOLoop* ioLoop, bool oneShotReadFlag) : UDPSocketBase (ioLoop), oneShotRead (oneShotReadFlag), externalReadBuffer (NULL), externalReadBufferSize (0)
    {
        handle.data = static_cast<DerivedClassType*> (this);
    }

    ~UDPSocketTemplate () {}

    void Close ()
    {
        InternalClose (&HandleCloseThunk);
    }

    void StopAsyncRead ()
    {
        uv_udp_recv_stop (Handle ());
    }

    int LocalEndpoint (Endpoint& endpoint)
    {
        int size = endpoint.size ();
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

    template <typename WriteRequestType>
    void InternalAsyncSend (WriteRequestType* request, const void* buffer, std::size_t size, const Endpoint& endpoint)
    {
        DVASSERT (request && buffer && size > 0);

        memset (&request->request, 0, sizeof (request->request));
        request->pthis        = static_cast<DerivedClassType*> (this);
        request->buffer       = uv_buf_init (static_cast<char*> (const_cast<void*> (buffer)), size);    // uv_buf_init doesn't modify buffer
        request->request.data = request;

        uv_udp_send (&request->request, Handle (), &request->buffer, 1, endpoint.CastToSockaddr (), &HandleSendThunk<WriteRequestType>);
    }

private:
    void HandleClose () {}
    void HandleAlloc (std::size_t /*suggested_size*/, uv_buf_t* buffer)
    {
        *buffer = uv_buf_init (static_cast<char*> (externalReadBuffer), externalReadBufferSize);
    }

    void HandleReceive (int /*error*/, std::size_t /*nread*/, const uv_buf_t* /*buffer*/, const Endpoint& /*endpoint*/, unsigned int /*flags*/) {}

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
        pthis->HandleReceive (error, nread, buffer, Endpoint (addr), flags);
        if (pthis->oneShotRead && 0 == error)
            pthis->StopAsyncRead ();
    }

    template <typename WriteRequestType>
    static void HandleSendThunk (uv_udp_send_t* request, int error)
    {
        WriteRequestType* req = static_cast<WriteRequestType*> (request->data);
        req->pthis->HandleSend (req, error);
    }

protected:
    bool        oneShotRead;
    void*       externalReadBuffer;
    std::size_t externalReadBufferSize;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_UDPSOCKETTEMPLATE_H__
