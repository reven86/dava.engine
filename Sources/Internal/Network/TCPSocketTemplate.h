/*==================================================================================
    Copyright(c) 2008, binaryzebra
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

#include "Endpoint.h"
#include "Buffer.h"
#include "HandleBase.h"

namespace DAVA
{

class IOLoop;

/*
 Template class TCPSocketTemplate provides basic stream socket capabilities
 Template parameter T specifies type that inherits TCPSocketTemplate and implements necessary methods (CRTP idiom).

 Type specified by T should implement methods:
    1. void HandleClose(), called after socket handle has been closed by libuv
    2. void HandleConnect(int32 error), called after connection to server has been established
        Parameters:
            error - nonzero if error has occured
    3. void HandleAlloc(Buffer* buffer), called before read operation to allow specify read buffer
        Parameters:
            buffer - new read buffer
    4. void HandleRead(int32 error, std::size_t nread), called after some data have been arrived
        Parameters:
            error    - nonzero if error has occured; if connection has been closed IsEOF(error) returns true
            nread    - number of bytes placed in read buffer
    5. template<typename U>
       void HandleWrite(int32 error, const Buffer* buffers, std::size_t bufferCount, U& requestData)
        Parameters:
           error       - nonzero if error has occured
           buffers     - buffers that have been sent (opportunity to delete them)
           bufferCount - number of buffers
           requestData - some data specified by derived class when issuing send request
*/
template <typename T>
class TCPSocketTemplate : public HandleBase<uv_tcp_t>
{
private:
    typedef HandleBase<uv_tcp_t> BaseClassType;
    typedef T                    DerivedClassType;

protected:
    // Maximum write buffers that can be sent in one operation
    static const std::size_t maxWriteBuffers = 6;

    /*
     TCPWriteRequest - wrapper for libuv's uv_write_t request with some necessary fields and
     auxilliary data specified by derived class
    */
    template<typename U>
    struct TCPWriteRequest
    {
        TCPWriteRequest (DerivedClassType* derived, U& reqData) : request()
                                                                , pthis(derived)
                                                                , requestData(reqData)
        {
            request.data = this;
        }
        uv_write_t        request;
        DerivedClassType* pthis;
        Buffer            buffers[maxWriteBuffers];
        std::size_t       bufferCount;
        U                 requestData;
    };

public:
    TCPSocketTemplate(IOLoop* ioLoop);
    ~TCPSocketTemplate() {}

    void Close();

    int32 LocalEndpoint(Endpoint& endpoint);
    int32 RemoteEndpoint(Endpoint& endpoint);

    std::size_t WriteQueueSize() const;

    bool IsEOF(int32 error) const { return UV_EOF == error; }

protected:
    int32 InternalAsyncConnect(const Endpoint& endpoint);
    int32 InternalAsyncRead();
    template<typename U>
    int32 InternalAsyncWrite(const Buffer* buffers, std::size_t bufferCount, U& requestData);

private:
    // Methods should be implemented in derived class
    void HandleClose();
    void HandleConnect(int32 error);
    void HandleAlloc(Buffer* buffer);
    void HandleRead(int32 error, std::size_t nread);
    template<typename U>
    void HandleWrite(int32 error, const Buffer* buffers, std::size_t bufferCount, U& requestData);

    // Thunks between C callbacks and C++ class methods
    static void HandleCloseThunk(uv_handle_t* handle);
    static void HandleConnectThunk(uv_connect_t* connectRequest, int error);
    static void HandleAllocThunk(uv_handle_t* handle, std::size_t suggested_size, uv_buf_t* buffer);
    static void HandleReadThunk(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buffer);
    template<typename U>
    static void HandleWriteThunk(uv_write_t* writeRequest, int error);

protected:
    uv_connect_t connectRequest;
};

//////////////////////////////////////////////////////////////////////////
template <typename T>
TCPSocketTemplate<T>::TCPSocketTemplate(IOLoop* ioLoop) : BaseClassType(ioLoop)
                                                        , connectRequest()
{
    SetHandleData(static_cast<DerivedClassType*>(this));
    connectRequest.data = static_cast<DerivedClassType*>(this);
}

template <typename T>
void TCPSocketTemplate<T>::Close()
{
    BaseClassType::InternalClose(&HandleCloseThunk);
}

template <typename T>
std::size_t TCPSocketTemplate<T>::WriteQueueSize() const
{
    return Handle<uv_tcp_t>()->write_queue_size;
}

template <typename T>
int32 TCPSocketTemplate<T>::LocalEndpoint(Endpoint& endpoint)
{
    int size = endpoint.Size();
    return uv_tcp_getsockname(Handle<uv_tcp_t>(), endpoint.CastToSockaddr(), &size);
}

template <typename T>
int32 TCPSocketTemplate<T>::RemoteEndpoint(Endpoint& endpoint)
{
    int size = endpoint.Size();
    return uv_tcp_getpeername(Handle<uv_tcp_t>(), endpoint.CastToSockaddr(), &size);
}

template <typename T>
int32 TCPSocketTemplate<T>::InternalAsyncConnect(const Endpoint& endpoint)
{
    return uv_tcp_connect(&connectRequest, Handle<uv_tcp_t>(), endpoint.CastToSockaddr(), &HandleConnectThunk);
}

template <typename T>
int32 TCPSocketTemplate<T>::InternalAsyncRead()
{
    return uv_read_start(Handle<uv_stream_t>(), &HandleAllocThunk, &HandleReadThunk);
}

template <typename T>
template<typename U>
int32 TCPSocketTemplate<T>::InternalAsyncWrite(const Buffer* buffers, std::size_t bufferCount, U& requestData)
{
    DVASSERT(buffers != NULL && 0 < bufferCount && bufferCount <= maxWriteBuffers);

    TCPWriteRequest<U>* writeRequest = new TCPWriteRequest<U>(static_cast<DerivedClassType*>(this), requestData);
    writeRequest->bufferCount = bufferCount;
    for (std::size_t i = 0;i < bufferCount;++i)
    {
        DVASSERT(buffers[i].base != NULL && buffers[i].len > 0);
        writeRequest->buffers[i] = buffers[i];
    }

    int32 error = uv_write(&writeRequest->request, Handle<uv_stream_t>(), writeRequest->buffers
                                                                        , writeRequest->bufferCount
                                                                        , &HandleWriteThunk<U>);
    if (error != 0)
        delete writeRequest;
    return error;
}

///   Thunks   ///////////////////////////////////////////////////////////
template <typename T>
void TCPSocketTemplate<T>::HandleCloseThunk(uv_handle_t* handle)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->InternalInit();
    pthis->HandleClose();
}

template <typename T>
void TCPSocketTemplate<T>::HandleConnectThunk(uv_connect_t* connectRequest, int error)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(connectRequest->data);
    pthis->HandleConnect(error);
}

template <typename T>
void TCPSocketTemplate<T>::HandleAllocThunk(uv_handle_t* handle, std::size_t /*suggested_size*/, uv_buf_t* buffer)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->HandleAlloc(buffer);
}

template <typename T>
void TCPSocketTemplate<T>::HandleReadThunk(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buffer)
{
    int32 error = 0;
    if(nread < 0)
    {
        error = nread;
        nread = 0;
    }
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->HandleRead(error, nread);
}

template <typename T>
template<typename U>
void TCPSocketTemplate<T>::HandleWriteThunk(uv_write_t* request, int error)
{
    TCPWriteRequest<U>* writeRequest = static_cast<TCPWriteRequest<U>*>(request->data);
    DVASSERT(writeRequest != NULL && writeRequest->pthis != NULL);

    writeRequest->pthis->HandleWrite(error, writeRequest->buffers, writeRequest->bufferCount, writeRequest->requestData);
    delete writeRequest;
}

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPSOCKETTEMPLATE_H__
