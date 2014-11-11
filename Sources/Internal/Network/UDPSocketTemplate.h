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

#ifndef __DAVAENGINE_UDPSocketTemplate_H__
#define __DAVAENGINE_UDPSocketTemplate_H__

#include "Endpoint.h"
#include "Buffer.h"
#include "HandleBase.h"

namespace DAVA
{

class IOLoop;

/*
 Template class UDPSocketTemplate provides basic datagram socket capabilities
 Template parameter T specifies type that inherits UDPSocketTemplate and implements necessary methods (CRTP idiom).

 Type specified by T should implement methods:
    1. void HandleClose(), called after socket handle has been closed by libuv
    2. void HandleAlloc(Buffer* buffer), called before read operation to allow specify read buffer
        Parameters:
            buffer - new read buffer
    3. void HandleReceive(int32 error, std::size_t nread, const Endpoint& endpoint, bool partial), called after datagram has been arrived
        Parameters:
            error    - nonzero if error has occured
            nread    - number of bytes placed in read buffer
            endpoint - who sent data
            partial  - if true only part of datagram has been placed in read buffer, remaining part has been discarded by OS
    4.template<typename U>
      void HandleSend(int32 error, const Buffer* buffers, std::size_t bufferCount, U& requestData), called after data have been sent to destination
        Parameters:
            error       - nonzero if error has occured
            buffers     - buffers that have been sent (opportunity to delete them)
            bufferCount - number of buffers
            requestData - some data specified by derived class when issuing send request
*/
template <typename T>
class UDPSocketTemplate : public HandleBase<uv_udp_t>
{
public:
    typedef HandleBase<uv_udp_t> BaseClassType;
    typedef T                    DerivedClassType;

protected:
    // Maximum write buffers that can be sent in one operation
    static const std::size_t maxWriteBuffers = 6;

    /*
     UDPSendRequest - wrapper for libuv's uv_udp_send_t request with some necessary fields and
     auxilliary data specified by derived class
    */
    template<typename U>
    struct UDPSendRequest
    {
        UDPSendRequest (DerivedClassType* derived, U& reqData) : request()
                                                               , pthis(derived)
                                                               , requestData(reqData)
        {
            request.data = this;
        }
        uv_udp_send_t     request;
        DerivedClassType* pthis;
        Buffer            buffers[maxWriteBuffers];
        std::size_t       bufferCount;
        U                 requestData;
    };

public:
    UDPSocketTemplate(IOLoop* ioLoop);
    ~UDPSocketTemplate() {}

    void Close();

    int32 Bind(const Endpoint& endpoint, bool reuseAddrOption = false);
    int32 Bind(const char8* ipaddr, uint16 port, bool reuseAddrOption = false);
    int32 Bind(uint16 port, bool reuseAddrOption = false);

    int32 JoinMulticastGroup(const char8* multicastAddr, const char8* interfaceAddr = NULL);
    int32 LeaveMulticastGroup(const char8* multicastAddr, const char8* interfaceAddr = NULL);

    std::size_t SendQueueSize() const;
    std::size_t SendRequestCount() const;

    int32 LocalEndpoint(Endpoint& endpoint);

protected:
    int32 InternalAsyncReceive();
    template<typename U>
    int32 InternalAsyncSend(const Buffer* buffers, std::size_t bufferCount, const Endpoint& endpoint, U& requestData);

private:
    // Methods should be implemented in derived class
    void HandleClose();
    void HandleAlloc(Buffer* buffer);
    void HandleReceive(int32 error, std::size_t nread, const Endpoint& endpoint, bool partial);
    template<typename U>
    void HandleSend(int32 error, const Buffer* buffers, std::size_t bufferCount, U& requestData);

    // Thunks between C callbacks and C++ class methods
    static void HandleAllocThunk(uv_handle_t* handle, std::size_t suggested_size, uv_buf_t* buffer);
    static void HandleCloseThunk(uv_handle_t* handle);
    static void HandleReceiveThunk(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buffer, const sockaddr* addr, unsigned int flags);
    template<typename U>
    static void HandleSendThunk(uv_udp_send_t* request, int error);
};

//////////////////////////////////////////////////////////////////////////
template <typename T>
UDPSocketTemplate<T>::UDPSocketTemplate(IOLoop* ioLoop) : BaseClassType(ioLoop)
{
    SetHandleData(static_cast<DerivedClassType*>(this));
}

template <typename T>
void UDPSocketTemplate<T>::Close()
{
    BaseClassType::InternalClose(&HandleCloseThunk);
}

template <typename T>
int32 UDPSocketTemplate<T>::Bind(const Endpoint& endpoint, bool reuseAddrOption)
{
    return uv_udp_bind(Handle<uv_udp_t>(), endpoint.CastToSockaddr(), reuseAddrOption ? UV_UDP_REUSEADDR : 0);
}

template <typename T>
int32 UDPSocketTemplate<T>::Bind(const char8* ipaddr, uint16 port, bool reuseAddrOption)
{
    DVASSERT(ipaddr != NULL);

    Endpoint endpoint;
    int32 result = uv_ip4_addr(ipaddr, port, endpoint.CastToSockaddrIn());
    if(0 == result)
    {
        result = Bind(endpoint, reuseAddrOption);
    }
    return result;
}

template <typename T>
int32 UDPSocketTemplate<T>::Bind(uint16 port, bool reuseAddrOption)
{
    return Bind(Endpoint(port), reuseAddrOption);
}

template <typename T>
int32 UDPSocketTemplate<T>::JoinMulticastGroup(const char8* multicastAddr, const char8* interfaceAddr)
{
    DVASSERT(multicastAddr != NULL);
    return uv_udp_set_membership(Handle<uv_udp_t>(), multicastAddr, interfaceAddr, UV_JOIN_GROUP);
}

template <typename T>
int32 UDPSocketTemplate<T>::LeaveMulticastGroup(const char8* multicastAddr, const char8* interfaceAddr)
{
    DVASSERT(multicastAddr != NULL);
    return uv_udp_set_membership(Handle<uv_udp_t>(), multicastAddr, interfaceAddr, UV_LEAVE_GROUP);
}

template <typename T>
std::size_t UDPSocketTemplate<T>::SendQueueSize() const
{
    return handle.send_queue_size;
}

template <typename T>
std::size_t UDPSocketTemplate<T>::SendRequestCount() const
{
    return handle.send_queue_count;
}

template <typename T>
int32 UDPSocketTemplate<T>::LocalEndpoint(Endpoint& endpoint)
{
    int size = static_cast<int> (endpoint.Size());
    return uv_udp_getsockname(Handle(), endpoint.CastToSockaddr(), &size);
}

template <typename T>
int32 UDPSocketTemplate<T>::InternalAsyncReceive()
{
    return uv_udp_recv_start(Handle<uv_udp_t>(), &HandleAllocThunk, &HandleReceiveThunk);
}

template <typename T>
template<typename U>
int32 UDPSocketTemplate<T>::InternalAsyncSend(const Buffer* buffers, std::size_t bufferCount, const Endpoint& endpoint, U& requestData)
{
    DVASSERT(buffers != NULL && 0 < bufferCount && bufferCount <= maxWriteBuffers);

    UDPSendRequest<U>* sendRequest = new UDPSendRequest<U>(static_cast<DerivedClassType*>(this), requestData);
    sendRequest->bufferCount = bufferCount;
    for (std::size_t i = 0;i < bufferCount;++i)
    {
        DVASSERT(buffers[i].base != NULL && buffers[i].len > 0);
        sendRequest->buffers[i] = buffers[i];
    }

    int32 error = uv_udp_send(&sendRequest->request, Handle<uv_udp_t>(), sendRequest->buffers
                                                                       , sendRequest->bufferCount
                                                                       , endpoint.CastToSockaddr()
                                                                       , &HandleSendThunk<U>);
    if (error != 0)
        delete sendRequest;
    return error;
}

///   Thunks   ///////////////////////////////////////////////////////////
template <typename T>
void UDPSocketTemplate<T>::HandleAllocThunk(uv_handle_t* handle, std::size_t suggested_size, uv_buf_t* buffer)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->HandleAlloc(buffer);
}

template <typename T>
void UDPSocketTemplate<T>::HandleCloseThunk(uv_handle_t* handle)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->InternalInit();
    pthis->HandleClose();
}

template <typename T>
void UDPSocketTemplate<T>::HandleReceiveThunk(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buffer, const sockaddr* addr, unsigned int flags)
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
template<typename U>
void UDPSocketTemplate<T>::HandleSendThunk(uv_udp_send_t* request, int error)
{
    UDPSendRequest<U>* sendRequest = static_cast<UDPSendRequest<U>*>(request->data);
    DVASSERT(sendRequest != NULL && sendRequest->pthis != NULL);

    sendRequest->pthis->HandleSend(error, sendRequest->buffers, sendRequest->bufferCount, sendRequest->requestData);
    delete sendRequest;
}

}	// namespace DAVA

#endif  // __DAVAENGINE_UDPSocketTemplate_H__
