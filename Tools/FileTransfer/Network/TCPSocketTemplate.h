/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_TCPSOCKETTEMPLATE_H__
#define __DAVAENGINE_TCPSOCKETTEMPLATE_H__

#include <libuv/uv.h>

#include <Debug/DVAssert.h>

#include "Endpoint.h"
#include "TCPSocketBase.h"

namespace DAVA
{

class IOLoop;

/*
 Template class TCPSocketTemplate provides basic capabilities: reading from and sending to stream socket
 Template parameter T specifies type that inherits TCPSocketTemplate(CRTP idiom)
 Bool template parameter autoRead specifies read behaviour:
    when autoRead is true libuv automatically issues next read operations until StopAsyncRead is called
    when autoRead is false user should explicitly issue next read operation
 Multiple simultaneous read operations lead to undefined behaviour.

 Type specified by T should implement methods:
    void HandleConnect(int32 error)
        This method is called after connection to TCP server has been established
        Parameter error is non zero on error
    void HandleRead(int32 error, std::size_t nread, const uv_buf_t* buffer)
        This method is called after data with length of nread bytes has been arrived
        Parameter error is non zero on error, UV_EOF when remote peer has closed connection or 0 on no error
    template<typename WriteRequestType>
    void HandleWrite(WriteRequestType* request, int32 error)
        This method is called after data has been written to
    void HandleClose()
        This method is called after underlying socket has been closed by libuv

 Summary of methods that should be implemented by T:
    void HandleConnect(int32 error);
    void HandleRead(int32 error, std::size_t nread, const uv_buf_t* buffer);
    template<typename WriteRequestType>
    void HandleWrite(WriteRequestType* request, int32 error);
    void HandleClose();
*/
template <typename T, bool autoRead = false>
class TCPSocketTemplate : public TCPSocketBase
{
private:
    typedef TCPSocketBase BaseClassType;
    typedef T             DerivedClassType;

    static const bool autoReadFlag = autoRead;

protected:
    struct WriteRequestBase
    {
        DerivedClassType* pthis;
        uv_buf_t          buffer;
        uv_write_t        request;
    };

public:
    explicit TCPSocketTemplate(IOLoop* ioLoop);
    ~TCPSocketTemplate() {}

    void Close();

    int32 StopAsyncRead();

    int32 LocalEndpoint(Endpoint& endpoint);
    int32 RemoteEndpoint(Endpoint& endpoint);

protected:
    int32 InternalAsyncConnect(const Endpoint& endpoint);
    int32 InternalAsyncRead(void* buffer, std::size_t size);

    template<typename WriteRequestType>
    int32 InternalAsyncWrite(WriteRequestType* request, const void* buffer, std::size_t size);

private:
    void HandleClose() {}
    void HandleAlloc(std::size_t /*suggested_size*/, uv_buf_t* buffer);
    void HandleConnect(int32 /*error*/) {}
    void HandleRead(int32 /*error*/, std::size_t /*nread*/, const uv_buf_t* /*buffer*/) {}
    template<typename WriteRequestType>
    void HandleWrite(WriteRequestType* /*request*/, int32 /*error*/) {}

    static void HandleCloseThunk(uv_handle_t* handle);
    static void HandleAllocThunk(uv_handle_t* handle, std::size_t suggested_size, uv_buf_t* buffer);
    static void HandleConnectThunk(uv_connect_t* connectRequest, int error);
    static void HandleReadThunk(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buffer);
    template<typename WriteRequestType>
    static void HandleWriteThunk(uv_write_t* writeRequest, int error);

protected:
    uv_connect_t connectRequest;
    void*        externalReadBuffer;
    std::size_t  externalReadBufferSize;
};

//////////////////////////////////////////////////////////////////////////
template <typename T, bool autoRead>
TCPSocketTemplate<T, autoRead>::TCPSocketTemplate(IOLoop* ioLoop) : TCPSocketBase(ioLoop)
                                                                  , connectRequest()
                                                                  , externalReadBuffer(NULL)
                                                                  , externalReadBufferSize(0)
{
    DVASSERT(ioLoop != NULL);

    handle.data         = static_cast<DerivedClassType*>(this);
    connectRequest.data = static_cast<DerivedClassType*>(this);
}

template <typename T, bool autoRead>
void TCPSocketTemplate<T, autoRead>::Close()
{
    BaseClassType::InternalClose(&HandleCloseThunk);
}

template <typename T, bool autoRead>
int32 TCPSocketTemplate<T, autoRead>::StopAsyncRead()
{
    return uv_read_stop(HandleAsStream());
}

template <typename T, bool autoRead>
int32 TCPSocketTemplate<T, autoRead>::LocalEndpoint(Endpoint& endpoint)
{
    int size = endpoint.Size();
    return uv_tcp_getsockname(Handle(), endpoint.CastToSockaddr(), &size);
}

template <typename T, bool autoRead>
int32 TCPSocketTemplate<T, autoRead>::RemoteEndpoint(Endpoint& endpoint)
{
    int size = endpoint.Size();
    return uv_tcp_getpeername(Handle(), endpoint.CastToSockaddr(), &size);
}

template <typename T, bool autoRead>
int32 TCPSocketTemplate<T, autoRead>::InternalAsyncConnect(const Endpoint& endpoint)
{
    return uv_tcp_connect(&connectRequest, Handle(), endpoint.CastToSockaddr(), &HandleConnectThunk);
}

template <typename T, bool autoRead>
int32 TCPSocketTemplate<T, autoRead>::InternalAsyncRead(void* buffer, std::size_t size)
{
    DVASSERT(buffer != NULL && size > 0);

    externalReadBuffer     = buffer;
    externalReadBufferSize = size;
    return uv_read_start(HandleAsStream(), &HandleAllocThunk, &HandleReadThunk);
}

template <typename T, bool autoRead>
template<typename WriteRequestType>
int32 TCPSocketTemplate<T, autoRead>::InternalAsyncWrite(WriteRequestType* request, const void* buffer, std::size_t size)
{
    /*
        WriteRequestType should have following public members:
        DerivedClassType* pthis   - pointer to DerivedClassType instance(can be pointer to void)
        uv_buf_t          buffer  - libuv buffer to write
        uv_write_t        request - libuv write request
    */
    DVASSERT(request != NULL && buffer != NULL && size > 0);

    request->pthis        = static_cast<DerivedClassType*>(this);
    request->buffer       = uv_buf_init(static_cast<char*>(const_cast<void*>(buffer)), size);    // uv_buf_init doesn't modify buffer
    request->request.data = request;

    return uv_write(&request->request, HandleAsStream(), &request->buffer, 1, &HandleWriteThunk<WriteRequestType>);
}

template <typename T, bool autoRead>
void TCPSocketTemplate<T, autoRead>::HandleAlloc(std::size_t /*suggested_size*/, uv_buf_t* buffer)
{
    *buffer = uv_buf_init(static_cast<char*>(externalReadBuffer), externalReadBufferSize);
}

template <typename T, bool autoRead>
void TCPSocketTemplate<T, autoRead>::HandleCloseThunk(uv_handle_t* handle)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->CleanUpBeforeNextUse();
    pthis->HandleClose();
}

template <typename T, bool autoRead>
void TCPSocketTemplate<T, autoRead>::HandleAllocThunk(uv_handle_t* handle, std::size_t suggested_size, uv_buf_t* buffer)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->HandleAlloc(suggested_size, buffer);
}

template <typename T, bool autoRead>
void TCPSocketTemplate<T, autoRead>::HandleConnectThunk(uv_connect_t* connectRequest, int error)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(connectRequest->data);
    pthis->HandleConnect(error);
}

template <typename T, bool autoRead>
void TCPSocketTemplate<T, autoRead>::HandleReadThunk(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buffer)
{
    int32 error = 0;
    if(nread < 0)
    {
        error = nread;
        nread = 0;
    }
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    if(!autoReadFlag && 0 == error)
    {
        pthis->StopAsyncRead();
    }
    pthis->HandleRead(error, nread, buffer);
}

template <typename T, bool autoRead>
template<typename WriteRequestType>
void TCPSocketTemplate<T, autoRead>::HandleWriteThunk(uv_write_t* writeRequest, int error)
{
    WriteRequestType* request = static_cast<WriteRequestType*>(writeRequest->data);
    request->pthis->HandleWrite(request, error);
}

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPSOCKETTEMPLATE_H__
