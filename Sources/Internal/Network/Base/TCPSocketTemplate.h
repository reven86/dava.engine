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

#include <Base/BaseTypes.h>
#include <Base/Noncopyable.h>

#include <Network/Base/IOLoop.h>
#include <Network/Base/Endpoint.h>
#include <Network/Base/Buffer.h>

namespace DAVA
{
namespace Net
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
    static const size_t MAX_WRITE_BUFFERS = 4;

public:
    TCPSocketTemplate(IOLoop* ioLoop);
    ~TCPSocketTemplate();

    int32 LocalEndpoint(Endpoint& endpoint);
    int32 RemoteEndpoint(Endpoint& endpoint);

    bool IsEOF(int32 error) const;

    bool IsOpen() const;
    bool IsClosing() const;

protected:
    int32 DoOpen();
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
TCPSocketTemplate<T>::~TCPSocketTemplate()
{
    // libuv handle should be closed before destroying object
    DVASSERT(false == isOpen && false == isClosing);
}

template <typename T>
int32 TCPSocketTemplate<T>::LocalEndpoint(Endpoint& endpoint)
{
    DVASSERT(true == isOpen && false == isClosing);
    int size = endpoint.Size();
    return uv_tcp_getsockname(&uvhandle, endpoint.CastToSockaddr(), &size);
}

template <typename T>
int32 TCPSocketTemplate<T>::RemoteEndpoint(Endpoint& endpoint)
{
    DVASSERT(true == isOpen && false == isClosing);
    int size = static_cast<int>(endpoint.Size());
    //UNCOMMENT
    //return uv_tcp_getpeername(&uvhandle, endpoint.CastToSockaddr(), &size);
    return 0;
}

template <typename T>
bool TCPSocketTemplate<T>::IsEOF(int32 error) const
{
    return UV_EOF == error;
}

template <typename T>
bool TCPSocketTemplate<T>::IsOpen() const
{
    return isOpen;
}

template <typename T>
bool TCPSocketTemplate<T>::IsClosing() const
{
    return isClosing;
}

template <typename T>
int32 TCPSocketTemplate<T>::DoOpen()
{
    DVASSERT(false == isOpen && false == isClosing);
    //UNCOMMENT
    int32 error = 0;//uv_tcp_init(loop->Handle(), &uvhandle);
    if (0 == error)
    {
        isOpen = true;
        uvhandle.data = this;
        uvconnect.data = this;
        uvwrite.data = this;
        uvshutdown.data = this;
    }
    return error;
}

template <typename T>
int32 TCPSocketTemplate<T>::DoConnect(const Endpoint& endpoint)
{
    DVASSERT(false == isClosing);
    int32 error = 0;
    if (false == isOpen)
        error = DoOpen();   // Automatically open on first call
    //UNCOMMENT
    //if (0 == error)
      //  error = uv_tcp_connect(&uvconnect, &uvhandle, endpoint.CastToSockaddr(), &HandleConnectThunk);
    return error;
}

template <typename T>
int32 TCPSocketTemplate<T>::DoStartRead()
{
    DVASSERT(true == isOpen && false == isClosing);
    //UNCOMMENT
    //return uv_read_start(reinterpret_cast<uv_stream_t*>(&uvhandle), &HandleAllocThunk, &HandleReadThunk);
    return 0;
}

template <typename T>
int32 TCPSocketTemplate<T>::DoWrite(const Buffer* buffers, size_t bufferCount)
{
    DVASSERT(true == isOpen && false == isClosing);
    DVASSERT(buffers != NULL && 0 < bufferCount && bufferCount <= MAX_WRITE_BUFFERS);
    DVASSERT(0 == writeBufferCount);    // Next write is allowed only after previous write completion

    writeBufferCount = bufferCount;
    for (size_t i = 0;i < bufferCount;++i)
    {
        DVASSERT(buffers[i].base != NULL && buffers[i].len > 0);
        writeBuffers[i] = buffers[i];
    }

    //UNCOMMENT
    //return uv_write(&uvwrite, reinterpret_cast<uv_stream_t*>(&uvhandle), writeBuffers, static_cast<unsigned int>(writeBufferCount), &HandleWriteThunk);
    return 0;
}

template <typename T>
int32 TCPSocketTemplate<T>::DoShutdown()
{
    DVASSERT(true == isOpen && false == isClosing);
    //UNCOMMENT
    //return uv_shutdown(&uvshutdown, reinterpret_cast<uv_stream_t*>(&uvhandle), &HandleShutdownThunk);
    return 0;
}

template <typename T>
void TCPSocketTemplate<T>::DoClose()
{
    DVASSERT(true == isOpen && false == isClosing);
    isOpen = false;
    isClosing = true;
    //UNCOMMENT
    //uv_close(reinterpret_cast<uv_handle_t*>(&uvhandle), &HandleCloseThunk);
}

///   Thunks   ///////////////////////////////////////////////////////////
template <typename T>
void TCPSocketTemplate<T>::HandleCloseThunk(uv_handle_t* handle)
{
    TCPSocketTemplate* self = static_cast<TCPSocketTemplate*>(handle->data);
    self->isClosing = false;    // Mark socket has been closed
    self->writeBufferCount = 0;
    // And clear handle and requests
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
        error = static_cast<int32>(nread);
        nread = 0;
    }
    TCPSocketTemplate* self = static_cast<TCPSocketTemplate*>(handle->data);
    static_cast<T*>(self)->HandleRead(error, nread);
}

template <typename T>
void TCPSocketTemplate<T>::HandleWriteThunk(uv_write_t* request, int error)
{
    TCPSocketTemplate* self = static_cast<TCPSocketTemplate*>(request->data);
    size_t bufferCount = self->writeBufferCount;
    self->writeBufferCount = 0;     // Mark write operation has completed
    static_cast<T*>(self)->HandleWrite(error, self->writeBuffers, bufferCount);
}

}   // namespace Net
}	// namespace DAVA

#endif  // __DAVAENGINE_TCPSOCKETTEMPLATE_H__
