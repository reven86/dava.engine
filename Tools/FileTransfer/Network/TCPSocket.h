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
class TCPSocket : public TCPSocketTemplate<TCPSocket, true>
{
public:
    typedef Function<void(TCPSocket* socket)>                                               CloseHandlerType;
    typedef Function<void(TCPSocket* socket, int32 error)>                                  ConnectHandlerType;
    typedef Function<void(TCPSocket* socket, int32 error, std::size_t nread, void* buffer)> ReadHandlerType;
    typedef Function<void(TCPSocket* socket, int32 error, const void* buffer)>              WriteHandlerType;

private:
    typedef TCPSocketTemplate<TCPSocket, true> BaseClassType;

    struct WriteRequest : public BaseClassType::WriteRequestBase
    {
        WriteRequest () : BaseClassType::WriteRequestBase(), writeHandler() {}
        WriteHandlerType writeHandler;
    };

public:
    explicit TCPSocket(IOLoop* ioLoop);
    ~TCPSocket() {}

    void SetCloseHandler(CloseHandlerType handler);

    int32 AsyncConnect(const Endpoint& endpoint, ConnectHandlerType handler);
    int32 AsyncRead(void* buffer, std::size_t size, ReadHandlerType handler);
    int32 AsyncWrite(const void* buffer, std::size_t size, WriteHandlerType handler);

    void HandleClose();
    void HandleConnect(int32 error);
    void HandleRead(int32 error, size_t nread, const uv_buf_t* buffer);
    void HandleWrite(WriteRequest* request, int32 error);

private:
    CloseHandlerType   closeHandler;
    ConnectHandlerType connectHandler;
    ReadHandlerType    readHandler;
    WriteRequest       writeRequest;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPSOCKET_H__
