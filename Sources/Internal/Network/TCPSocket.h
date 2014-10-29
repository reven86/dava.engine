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

#ifndef __DAVAENGINE_TCPSOCKET_H__
#define __DAVAENGINE_TCPSOCKET_H__

#include <Debug/DVAssert.h>
#include <Base/Function.h>

#include "TCPSocketTemplate.h"

namespace DAVA
{

/*
 Class TCPSocket - fully functional TCP socket implementation which can be used in most cases.
 Can connect to remote socket, read and write data.
 User can provide functional object which is called on completion.
 Functional objects prototypes:
    ConnectHandlerType - called on connect operation completion
        void f(TCPSocket* socket, int32 error);
    ReadHandlerType - called on read operation completion
        void f(TCPSocket* socket, int32 error, std::size_t nread, void* buffer);
    WriteHandlerType - called on write operation completion
        void f(TCPSocket* socket, int32 error, const void* buffer);
 User is responsible for error processing.
*/
class TCPSocket : public TCPSocketTemplate<TCPSocket, false>
{
private:
    typedef TCPSocketTemplate<TCPSocket, false> BaseClassType;

public:
    typedef Function<void(TCPSocket* socket)>                                               CloseHandlerType;
    typedef Function<void(TCPSocket* socket, int32 error)>                                  ConnectHandlerType;
    typedef Function<void(TCPSocket* socket, int32 error, std::size_t nread, void* buffer)> ReadHandlerType;
    typedef Function<void(TCPSocket* socket, int32 error, const void* buffer)>              WriteHandlerType;

private:
    struct WriteRequest : public BaseClassType::WriteRequestBase
    {
        WriteRequest(WriteHandlerType handler) : BaseClassType::WriteRequestBase(), writeHandler(handler) {}
        WriteHandlerType writeHandler;
    };

public:
    explicit TCPSocket(IOLoop* ioLoop, bool autoDeleteOnCloseFlag = false);
    ~TCPSocket() {}

    template <typename Handler>
    void SetCloseHandler(Handler handler);

    template <typename Handler>
    int32 AsyncConnect(const Endpoint& endpoint, Handler handler);
    template <typename Handler>
    int32 AsyncRead(void* buffer, std::size_t size, Handler handler);
    template <typename Handler>
    int32 AsyncWrite(const void* buffer, std::size_t size, Handler handler);

    void HandleClose();
    void HandleConnect(int32 error);
    void HandleRead(int32 error, size_t nread, const uv_buf_t* buffer);
    void HandleWrite(WriteRequest* request, int32 error);

private:
    bool               autoDeleteOnClose;   // TODO: do I really need this flag?
    CloseHandlerType   closeHandler;
    ConnectHandlerType connectHandler;
    ReadHandlerType    readHandler;
};

//////////////////////////////////////////////////////////////////////////
template <typename Handler>
inline void TCPSocket::SetCloseHandler(Handler handler)
{
    closeHandler = handler;
}

template <typename Handler>
inline int32 TCPSocket::AsyncConnect(const Endpoint& endpoint, Handler handler)
{
    connectHandler = handler;
    return BaseClassType::InternalAsyncConnect(endpoint);
}

template <typename Handler>
inline int32 TCPSocket::AsyncRead(void* buffer, std::size_t size, Handler handler)
{
    DVASSERT(buffer != NULL && size > 0);

    readHandler = handler;
    return BaseClassType::InternalAsyncRead(buffer, size);
}

template <typename Handler>
inline int32 TCPSocket::AsyncWrite(const void* buffer, std::size_t size, Handler handler)
{
    DVASSERT(buffer != NULL && size > 0);

    WriteRequest* request = new WriteRequest(handler);
    return BaseClassType::InternalAsyncWrite(request, buffer, size);
}

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPSOCKET_H__
