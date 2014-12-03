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

#include <Base/Noncopyable.h>
#include <Debug/DVAssert.h>

#include "IOLoop.h"
#include "Endpoint.h"
#include "Buffer.h"

namespace DAVA
{

/*
 Template class TCPSocketTemplate wraps TCP socket from underlying network library and provides interface to user
 through CRTP idiom. Class specified by template parameter T should inherit TCPSocketTemplate and provide some
 members that will be called by base class (TCPSocketTemplate) using compile-time polymorphism.
*/
template <typename T>
class TCPSocketTemplate : private Noncopyable
{
    // Maximum write buffers that can be sent in one operation
    static const size_t MAX_WRITE_BUFFERS = 10;

public:
    TCPSocketTemplate(IOLoop* ioLoop);
    ~TCPSocketTemplate();

    int32 LocalEndpoint(Endpoint& endpoint);
    int32 RemoteEndpoint(Endpoint& endpoint);

    size_t WriteQueueSize() const { return uvhandle.write_queue_size; }
    bool IsEOF(int32 error) const { return UV_EOF == error; }

protected:
    bool IsOpen() const { return isOpen; }
    void DoOpen();

    int32 DoConnect(const Endpoint& endpoint);
    int32 DoStartRead();
    int32 DoWrite(const Buffer* buffers, size_t bufferCount);
    int32 DoShutdown();
    void DoClose();

private:
    // Thunks between C callbacks and C++ class methods
    static void HandleCloseThunk(uv_handle_t* handle);
    static void HandleShutdownThunk(uv_shutdown_t* request, int error);
    static void HandleConnectThunk(uv_connect_t* request, int error);
    static void HandleAllocThunk(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buffer);
    static void HandleReadThunk(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buffer);
    static void HandleWriteThunk(uv_write_t* request, int error);

protected:
    uv_tcp_t uvhandle;                      // libuv handle itself
    IOLoop* loop;                           // IOLoop object handle is attached to
    bool isOpen;                            // Handle has been initialized and can be used in operations
    bool isClosing;                         // Close has been issued and waiting for close operation complete, used mainly for asserts
    uv_connect_t uvconnect;                 // libuv requests for connect
    uv_write_t uvwrite;                     //  write 
    uv_shutdown_t uvshutdown;               //  and shutdown
    Buffer writeBuffers[MAX_WRITE_BUFFERS]; // Write buffers participating in current write operation
    size_t writeBufferCount;                // Number of write buffers participating in current write operation
};

//////////////////////////////////////////////////////////////////////////
template <typename T>
TCPSocketTemplate<T>::TCPSocketTemplate(IOLoop* ioLoop) : uvhandle()
                                                        , loop(ioLoop)
                                                        , isOpen(false)
                                                        , isClosing(false)
                                                        , uvconnect()
                                                        , uvwrite()
                                                        , uvshutdown()
                                                        , writeBufferCount(0)
{
    DVASSERT(ioLoop != NULL);
    Memset(writeBuffers, 0, sizeof(writeBuffers));
}

template <typename T>
inline TCPSocketTemplate<T>::~TCPSocketTemplate()
{
    // libuv handle should be closed before destroying object
    DVASSERT(false == isOpen && false == isClosing);
}

template <typename T>
inline int32 TCPSocketTemplate<T>::LocalEndpoint(Endpoint& endpoint)
{
    DVASSERT(true == isOpen && false == isClosing);
    int size = endpoint.Size();
    return uv_tcp_getsockname(&uvhandle, endpoint.CastToSockaddr(), &size);
}

template <typename T>
inline int32 TCPSocketTemplate<T>::RemoteEndpoint(Endpoint& endpoint)
{
    DVASSERT(true == isOpen && false == isClosing);
    int size = endpoint.Size();
    return uv_tcp_getpeername(&uvhandle, endpoint.CastToSockaddr(), &size);
}

template <typename T>
void TCPSocketTemplate<T>::DoOpen()
{
    DVASSERT(false == isOpen && false == isClosing);
    uv_tcp_init(loop->Handle(), &uvhandle);
    isOpen = true;
    uvhandle.data = this;
    uvconnect.data = this;
    uvwrite.data = this;
    uvshutdown.data = this;
}

template <typename T>
int32 TCPSocketTemplate<T>::DoConnect(const Endpoint& endpoint)
{
    DVASSERT(false == isClosing);
    if (!isOpen)
        DoOpen();   // Automatically open on first call
    return uv_tcp_connect(&uvconnect, &uvhandle, endpoint.CastToSockaddr(), &HandleConnectThunk);
}

template <typename T>
int32 TCPSocketTemplate<T>::DoStartRead()
{
    DVASSERT(true == isOpen && false == isClosing);
    return uv_read_start(reinterpret_cast<uv_stream_t*>(&uvhandle), &HandleAllocThunk, &HandleReadThunk);
}

template <typename T>
int32 TCPSocketTemplate<T>::DoWrite(const Buffer* buffers, size_t bufferCount)
{
    DVASSERT(true == isOpen && false == isClosing);
    DVASSERT(buffers != NULL && 0 < bufferCount && bufferCount <= MAX_WRITE_BUFFERS);

    writeBufferCount = bufferCount;
    for (size_t i = 0;i < bufferCount;++i)
    {
        DVASSERT(buffers[i].base != NULL && buffers[i].len > 0);
        writeBuffers[i] = buffers[i];
    }

    return uv_write(&uvwrite, reinterpret_cast<uv_stream_t*>(&uvhandle), writeBuffers, writeBufferCount, &HandleWriteThunk);
}

template <typename T>
int32 TCPSocketTemplate<T>::DoShutdown()
{
    DVASSERT(false == isClosing);
    return uv_shutdown(&uvshutdown, reinterpret_cast<uv_stream_t*>(&uvhandle), &HandleShutdownThunk);
}

template <typename T>
void TCPSocketTemplate<T>::DoClose()
{
    DVASSERT(true == isOpen && false == isClosing);
    if (true == isOpen)
    {
        isOpen = false;
        isClosing = true;
        uv_close(reinterpret_cast<uv_handle_t*>(&uvhandle), &HandleCloseThunk);
    }
}

///   Thunks   ///////////////////////////////////////////////////////////
template <typename T>
void TCPSocketTemplate<T>::HandleCloseThunk(uv_handle_t* handle)
{
    TCPSocketTemplate* self = static_cast<TCPSocketTemplate*>(handle->data);
    self->isClosing = false;    // Mark socket has been closed and clear handle
    Memset(&self->uvhandle, 0, sizeof(self->uvhandle));
    Memset(&self->uvconnect, 0, sizeof(self->uvconnect));
    Memset(&self->uvwrite, 0, sizeof(self->uvwrite));
    Memset(&self->uvshutdown, 0, sizeof(self->uvshutdown));

    static_cast<T*>(self)->HandleClose();
}

template <typename T>
void TCPSocketTemplate<T>::HandleShutdownThunk(uv_shutdown_t* request, int error)
{
    TCPSocketTemplate* self = reinterpret_cast<TCPSocketTemplate*>(request->data);
    static_cast<T*>(self)->HandleShutdown(error);
}

template <typename T>
void TCPSocketTemplate<T>::HandleConnectThunk(uv_connect_t* request, int error)
{
    TCPSocketTemplate* self = static_cast<TCPSocketTemplate*>(request->data);
    static_cast<T*>(self)->HandleConnect(error);
}

template <typename T>
void TCPSocketTemplate<T>::HandleAllocThunk(uv_handle_t* handle, size_t /*suggested_size*/, uv_buf_t* buffer)
{
    TCPSocketTemplate* self = static_cast<TCPSocketTemplate*>(handle->data);
    static_cast<T*>(self)->HandleAlloc(buffer);
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
    TCPSocketTemplate* self = static_cast<TCPSocketTemplate*>(handle->data);
    static_cast<T*>(self)->HandleRead(error, nread);
}

template <typename T>
void TCPSocketTemplate<T>::HandleWriteThunk(uv_write_t* request, int error)
{
    TCPSocketTemplate* self = static_cast<TCPSocketTemplate*>(request->data);
    static_cast<T*>(self)->HandleWrite(error, self->writeBuffers, self->writeBufferCount);
    self->writeBufferCount = 0;
}

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPSOCKETTEMPLATE_H__
