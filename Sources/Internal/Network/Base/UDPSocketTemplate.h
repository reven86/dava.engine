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

#ifndef __DAVAENGINE_UDPSOCKETTEMPLATE_H__
#define __DAVAENGINE_UDPSOCKETTEMPLATE_H__

#include "Base/BaseTypes.h"
#include "Base/Noncopyable.h"

#include "Network/Base/IOLoop.h"
#include "Network/Base/Endpoint.h"
#include "Network/Base/Buffer.h"

namespace DAVA
{
namespace Net
{

/*
 Template class UDPSocketTemplate wraps UDP socket from underlying network library and provides interface to user
 through CRTP idiom. Class specified by template parameter T should inherit UDPSocketTemplate and provide some
 members that will be called by base class (UDPSocketTemplate) using compile-time polymorphism.
*/
template <typename T>
class UDPSocketTemplate : private Noncopyable
{
    // Maximum write buffers that can be sent in one operation
    static const size_t MAX_WRITE_BUFFERS = 4;

public:
    UDPSocketTemplate(IOLoop* ioLoop);
    ~UDPSocketTemplate();

    int32 LocalEndpoint(Endpoint& endpoint);

    int32 JoinMulticastGroup(const char8* multicastAddr, const char8* interfaceAddr = NULL);
    int32 LeaveMulticastGroup(const char8* multicastAddr, const char8* interfaceAddr = NULL);

    int32 Bind(const Endpoint& endpoint, bool reuseAddrOption = false);

    bool IsOpen() const;
    bool IsClosing() const;

protected:
    int32 DoOpen();
    int32 DoStartReceive();
    int32 DoSend(const Buffer* buffers, size_t bufferCount, const Endpoint& endpoint);
    void DoClose();

private:
    // Thunks between C callbacks and C++ class methods
    static void HandleCloseThunk(uv_handle_t* handle);
    static void HandleAllocThunk(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buffer);
    static void HandleReceiveThunk(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buffer, const sockaddr* addr, unsigned int flags);
    static void HandleSendThunk(uv_udp_send_t* request, int error);

private:
    uv_udp_t uvhandle;                      // libuv handle itself
    IOLoop* loop;                           // IOLoop object handle is attached to
    bool isOpen;                            // Handle has been initialized and can be used in operations
    bool isClosing;                         // Close has been issued and waiting for close operation complete, used mainly for asserts
    uv_udp_send_t uvsend;                   // libuv request for send
    Buffer sendBuffers[MAX_WRITE_BUFFERS];  // Send buffers participating in current send operation
    size_t sendBufferCount;                 // Number of send buffers participating in current send operation
};

//////////////////////////////////////////////////////////////////////////
template <typename T>
UDPSocketTemplate<T>::UDPSocketTemplate(IOLoop* ioLoop) : uvhandle()
                                                        , loop(ioLoop)
                                                        , isOpen(false)
                                                        , isClosing(false)
                                                        , uvsend()
                                                        , sendBufferCount(0)
{
    DVASSERT(ioLoop != NULL);
    Memset(sendBuffers, 0, sizeof(sendBuffers));
}

template <typename T>
UDPSocketTemplate<T>::~UDPSocketTemplate()
{
    // libuv handle should be closed before destroying object
    DVASSERT(false == isOpen && false == isClosing);
}

template <typename T>
int32 UDPSocketTemplate<T>::LocalEndpoint(Endpoint& endpoint)
{
#ifdef __DAVAENGINE_WIN_UAP__
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return -1;
#else
    DVASSERT(true == isOpen && false == isClosing);
    int size = static_cast<int> (endpoint.Size());
    return uv_udp_getsockname(&uvhandle, endpoint.CastToSockaddr(), &size);
#endif
}

template <typename T>
int32 UDPSocketTemplate<T>::JoinMulticastGroup(const char8* multicastAddr, const char8* interfaceAddr)
{
#ifdef __DAVAENGINE_WIN_UAP__
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return -1;
#else
    DVASSERT(true == isOpen && false == isClosing && multicastAddr != NULL);
    return uv_udp_set_membership(&uvhandle, multicastAddr, interfaceAddr, UV_JOIN_GROUP);
#endif
}

template <typename T>
int32 UDPSocketTemplate<T>::LeaveMulticastGroup(const char8* multicastAddr, const char8* interfaceAddr)
{
#ifdef __DAVAENGINE_WIN_UAP__
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return -1;
#else
    DVASSERT(true == isOpen && false == isClosing && multicastAddr != NULL);
    return uv_udp_set_membership(&uvhandle, multicastAddr, interfaceAddr, UV_LEAVE_GROUP);
#endif
}

template <typename T>
int32 UDPSocketTemplate<T>::Bind(const Endpoint& endpoint, bool reuseAddrOption)
{
#ifdef __DAVAENGINE_WIN_UAP__
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return -1;
#else
    DVASSERT(false == isClosing);
    int32 error = 0;
    if (false == isOpen)
        error = DoOpen();   // Automatically open on first call
    if (0 == error)
        error = uv_udp_bind(&uvhandle, endpoint.CastToSockaddr(), reuseAddrOption ? UV_UDP_REUSEADDR : 0);
    return error;
#endif
}

template <typename T>
bool UDPSocketTemplate<T>::IsOpen() const
{
    return isOpen;
}

template <typename T>
bool UDPSocketTemplate<T>::IsClosing() const
{
    return isClosing;
}

template <typename T>
int32 UDPSocketTemplate<T>::DoOpen()
{
#ifdef __DAVAENGINE_WIN_UAP__
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return -1;
#else
    DVASSERT(false == isOpen && false == isClosing);
    int32 error = uv_udp_init(loop->Handle(), &uvhandle);
    if (0 == error)
    {
        isOpen = true;
        uvhandle.data = this;
        uvsend.data = this;
    }
    return error;
#endif
}

template <typename T>
int32 UDPSocketTemplate<T>::DoStartReceive()
{
#ifdef __DAVAENGINE_WIN_UAP__
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return -1;
#else
    DVASSERT(false == isClosing);
    int32 error = 0;
    if (false == isOpen)
        error = DoOpen();   // Automatically open on first call
    if (0 == error)
        error = uv_udp_recv_start(&uvhandle, &HandleAllocThunk, &HandleReceiveThunk);
    return error;
#endif
}

template <typename T>
int32 UDPSocketTemplate<T>::DoSend(const Buffer* buffers, size_t bufferCount, const Endpoint& endpoint)
{
#ifdef __DAVAENGINE_WIN_UAP__
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return -1;
#else
    DVASSERT(true == isOpen && false == isClosing);
    DVASSERT(buffers != NULL && 0 < bufferCount && bufferCount <= MAX_WRITE_BUFFERS);
    DVASSERT(0 == sendBufferCount);    // Next send is allowed only after previous send completion

    sendBufferCount = bufferCount;
    for (size_t i = 0;i < bufferCount;++i)
    {
        DVASSERT(buffers[i].base != NULL && buffers[i].len > 0);
        sendBuffers[i] = buffers[i];
    }

    return uv_udp_send(&uvsend, &uvhandle, sendBuffers, static_cast<uint32>(sendBufferCount), endpoint.CastToSockaddr(), &HandleSendThunk);
#endif
}

template <typename T>
void UDPSocketTemplate<T>::DoClose()
{
#ifdef __DAVAENGINE_WIN_UAP__
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
#else
    DVASSERT(true == isOpen && false == isClosing);
    isOpen = false;
    isClosing = true;
    uv_close(reinterpret_cast<uv_handle_t*>(&uvhandle), &HandleCloseThunk);
#endif
}

///   Thunks   ///////////////////////////////////////////////////////////
template <typename T>
void UDPSocketTemplate<T>::HandleAllocThunk(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buffer)
{
    UDPSocketTemplate* self = static_cast<UDPSocketTemplate*>(handle->data);
    static_cast<T*>(self)->HandleAlloc(buffer);
}

template <typename T>
void UDPSocketTemplate<T>::HandleCloseThunk(uv_handle_t* handle)
{
    UDPSocketTemplate* self = static_cast<UDPSocketTemplate*>(handle->data);
    self->isClosing = false;    // Mark socket has been closed
    self->sendBufferCount = 0;
    // And clear handle and requests
    Memset(&self->uvhandle, 0, sizeof(self->uvhandle));
    Memset(&self->uvsend, 0, sizeof(self->uvsend));

    static_cast<T*>(self)->HandleClose();
}

template <typename T>
void UDPSocketTemplate<T>::HandleReceiveThunk(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buffer, const sockaddr* addr, unsigned int flags)
{
    // According to libuv documentation under such condition there is nothing to read on UDP socket
    if(0 == nread && NULL == addr) return;

    int32 error = 0;
    if(nread < 0)
    {
        error = static_cast<int32>(nread);
        nread = 0;
    }
    UDPSocketTemplate* self = static_cast<UDPSocketTemplate*>(handle->data);
    static_cast<T*>(self)->HandleReceive(error, nread, Endpoint(addr), UV_UDP_PARTIAL == flags);
}

template <typename T>
void UDPSocketTemplate<T>::HandleSendThunk(uv_udp_send_t* request, int error)
{
    UDPSocketTemplate* self = static_cast<UDPSocketTemplate*>(request->data);
    size_t bufferCount = self->sendBufferCount;
    self->sendBufferCount = 0;     // Mark send operation has completed
    static_cast<T*>(self)->HandleSend(error, self->sendBuffers, bufferCount);
}

}   // namespace Net
}	// namespace DAVA

#endif  // __DAVAENGINE_UDPSOCKETTEMPLATE_H__
