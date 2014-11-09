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

#ifndef __DAVAENGINE_UDPSocketTemplateEx_H__
#define __DAVAENGINE_UDPSocketTemplateEx_H__

#include "Endpoint.h"
#include "Buffer.h"
#include "UDPSocketBase.h"

namespace DAVA
{

class IOLoop;

/*
 Template class UDPSocketTemplateEx provides basic capabilities: reading from and sending to datagram socket
 Template parameter T specifies type that inherits UDPSocketTemplateEx(CRTP idiom)
 Bool template parameter autoRead specifies read behaviour:
    when autoRead is true libuv automatically issues next read operations until StopAsyncReceive is called
    when autoRead is false user should explicitly issue next read operation
 Multiple simultaneous read operations lead to undefined behaviour.

 Type specified by T should implement methods:
    void HandleReceive(int32 error, std::size_t nread, const uv_buf_t* buffer, const Endpoint& endpoint, bool partial)
        This method is called after data with length of nread bytes has been arrived
        Parameter error is non zero on error
        Parameter partial is true when read buffer was too small and rest of the message was discarded by OS
    template<typename SendRequestType>
    void HandleSend(SendRequestType* request, int32 error)
        This method is called after data has been sent
    void HandleClose()
        This method is called after underlying socket has been closed by libuv

 Summary of methods that should be implemented by T:
    void HandleReceive(int32 error, std::size_t nread, const uv_buf_t* buffer, const Endpoint& endpoint, bool partial);
    template<typename SendRequestType>
    void HandleSend(SendRequestType* request, int32 error);
    void HandleClose();
*/

template <typename T>
class UDPSocketTemplateEx : public UDPSocketBase
{
public:
    typedef UDPSocketBase BaseClassType;
    typedef T             DerivedClassType;

    // Maximum simultaneous buffers that can be sent in one operation
    static const std::size_t MAX_WRITE_BUFFERS = 10;

private:
    /*
     UDPSendRequest - wrapper for libuv's uv_udp_send_t with some necessary fields.
    */
    struct UDPSendRequest
    {
        uv_udp_send_t     request;
        DerivedClassType* pthis;
        Buffer            buffers[MAX_WRITE_BUFFERS];
        std::size_t       bufferCount;
        int32             inProgress;
    };

public:
    explicit UDPSocketTemplateEx(IOLoop* ioLoop);
    ~UDPSocketTemplateEx() {}

    void Close();

    int32 StopAsyncReceive();

    int32 LocalEndpoint(Endpoint& endpoint);

protected:
    int32 InternalAsyncReceive(Buffer buffer);
    int32 InternalAsyncSend(const Buffer* buffers, std::size_t bufferCount, const Endpoint& endpoint);

private:
    bool LockSendRequest();
    void UnlockSendRequest();

    void HandleAlloc(std::size_t suggested_size, uv_buf_t* buffer);

    // Methods should be implemented in derived class
    void HandleClose() {}
    void HandleReceive(int32 error, std::size_t nread, const Endpoint& endpoint, bool partial) {}
    void HandleSend(int32 error, const Buffer* buffers, std::size_t bufferCount) {}

    // Thunks between C callbacks and C++ class methods
    static void HandleAllocThunk(uv_handle_t* handle, std::size_t suggested_size, uv_buf_t* buffer);
    static void HandleCloseThunk(uv_handle_t* handle);
    static void HandleReceiveThunk(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buffer, const sockaddr* addr, unsigned int flags);
    static void HandleSendThunk(uv_udp_send_t* request, int error);

protected:
    Buffer         readBuffer;
    UDPSendRequest sendRequest;
};

//////////////////////////////////////////////////////////////////////////
template <typename T>
UDPSocketTemplateEx<T>::UDPSocketTemplateEx(IOLoop* ioLoop) : UDPSocketBase(ioLoop)
                                                            , readBuffer()
                                                            , sendRequest()
{
    handle.data              = static_cast<DerivedClassType*>(this);
    sendRequest.request.data = &sendRequest;
    sendRequest.pthis        = static_cast<DerivedClassType*>(this);
}

template <typename T>
void UDPSocketTemplateEx<T>::Close()
{
    BaseClassType::InternalClose(&HandleCloseThunk);
}

template <typename T>
int32 UDPSocketTemplateEx<T>::StopAsyncReceive()
{
    return uv_udp_recv_stop(Handle());
}

template <typename T>
int32 UDPSocketTemplateEx<T>::LocalEndpoint(Endpoint& endpoint)
{
    int size = static_cast<int> (endpoint.Size());
    return uv_udp_getsockname(Handle(), endpoint.CastToSockaddr(), &size);
}

template <typename T>
int32 UDPSocketTemplateEx<T>::InternalAsyncReceive(Buffer buffer)
{
    DVASSERT(buffer.base != NULL && buffer.len > 0);

    readBuffer = buffer;
    return uv_udp_recv_start(Handle(), &HandleAllocThunk, &HandleReceiveThunk);
}

template <typename T>
int32 UDPSocketTemplateEx<T>::InternalAsyncSend(const Buffer* buffers, std::size_t bufferCount, const Endpoint& endpoint)
{
    DVASSERT(buffers != NULL && 0 < bufferCount && bufferCount <= MAX_WRITE_BUFFERS);

    if (LockSendRequest())
    {
        sendRequest.bufferCount = bufferCount;
        for (std::size_t i = 0;i < bufferCount;++i)
        {
            DVASSERT(buffers[i].base != NULL && buffers[i].len > 0);
            sendRequest.buffers[i] = buffers[i];
        }

        int32 error = uv_udp_send(&sendRequest.request, Handle(), sendRequest.buffers
                                                                , sendRequest.bufferCount
                                                                , endpoint.CastToSockaddr()
                                                                , &HandleSendThunk);
        if (error != 0)
            UnlockSendRequest();
        return error;
    }

    DVASSERT(false && "Send operation is not serialized");
    return 0;
}

template <typename T>
bool UDPSocketTemplateEx<T>::LockSendRequest()
{
    int32 prevState = sendRequest.inProgress;
    if (0 == sendRequest.inProgress)
    {
        sendRequest.inProgress = 1;
    }
    return 0 == prevState;
}

template <typename T>
void UDPSocketTemplateEx<T>::UnlockSendRequest()
{
    sendRequest.inProgress = 0;
}

template <typename T>
void UDPSocketTemplateEx<T>::HandleAlloc(std::size_t /*suggested_size*/, uv_buf_t* buffer)
{
    *buffer = readBuffer;
}

// Thunks
template <typename T>
void UDPSocketTemplateEx<T>::HandleAllocThunk(uv_handle_t* handle, std::size_t suggested_size, uv_buf_t* buffer)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->HandleAlloc(suggested_size, buffer);
}

template <typename T>
void UDPSocketTemplateEx<T>::HandleCloseThunk(uv_handle_t* handle)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->CleanUpBeforeNextUse();
    pthis->HandleClose();
}

template <typename T>
void UDPSocketTemplateEx<T>::HandleReceiveThunk(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buffer, const sockaddr* addr, unsigned int flags)
{
    // According to libuv documentation under such condition there is nothing to read on UDP socket
    if(0 == nread && NULL == addr) return;

    int32 error = 0;
    if(nread < 0)
    {
        error = nread;
        nread = 0;
    }
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->HandleReceive(error, nread, Endpoint(addr), UV_UDP_PARTIAL == flags);
}

template <typename T>
void UDPSocketTemplateEx<T>::HandleSendThunk(uv_udp_send_t* request, int error)
{
    UDPSendRequest* sendRequest = static_cast<UDPSendRequest*>(request->data);
    DVASSERT(sendRequest != NULL && sendRequest->pthis != NULL);

    sendRequest->pthis->UnlockSendRequest();
    sendRequest->pthis->HandleSend(error, sendRequest->buffers, sendRequest->bufferCount);
}

}	// namespace DAVA

#endif  // __DAVAENGINE_UDPSocketTemplateEx_H__
