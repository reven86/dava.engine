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


#ifndef __DAVAENGINE_UDPSOCKET_H__
#define __DAVAENGINE_UDPSOCKET_H__

#include <Debug/DVAssert.h>
#include <Base/Function.h>

#include "UDPSocketTemplate.h"

namespace DAVA
{

/*
 Class UDPSocket - fully functional UDP socket implementation which can be used in most cases.
 Can receieve from and send data to socket.
 User can provide functional object which is called on completion.
 Functional objects prototypes:
    CloseHandlerType - called when socket has been closed
        void f(UDPSocket* socket)
    ReceiveHandlerType - called on read operation completion
        void f(UDPSocket* socket, int32 error, std::size_t nread, void* buffer, const Endpoint& endpoint, bool partial);
    SendHandlerType - called on write operation completion
        void f(UDPSocket* socket, int32 error, const void* buffer);
 User is responsible for error processing.
*/

class UDPSocket : public UDPSocketTemplate<UDPSocket, true>
{
public:
    typedef Function<void(UDPSocket* socket)>                                      CloseHandlerType;
    typedef Function<void(UDPSocket* socket, int32 error, std::size_t nread,
                            void* buffer, const Endpoint& endpoint, bool partial)> ReceiveHandlerType;
    typedef Function<void(UDPSocket* socket, int32 error, const void* buffer)>     SendHandlerType;

private:
    typedef UDPSocketTemplate<UDPSocket, true> BaseClassType;

    struct SendRequest : public BaseClassType::SendRequestBase
    {
        SendHandlerType sendHandler;
    };

public:
    explicit UDPSocket(IOLoop* ioLoop);
    ~UDPSocket() {}

    void SetCloseHandler(CloseHandlerType handler);

    int32 AsyncReceive(void* buffer, std::size_t size, ReceiveHandlerType handler);
    int32 AsyncSend(const Endpoint& endpoint, const void* buffer, std::size_t size, SendHandlerType handler);

    void HandleClose();
    void HandleReceive(int32 error, std::size_t nread, const uv_buf_t* buffer, const Endpoint& endpoint, bool partial);
    void HandleSend(SendRequest* request, int32 error);

private:
    CloseHandlerType   closeHandler;
    ReceiveHandlerType receiveHandler;
    SendRequest        sendRequest;
};

}   // namespace DAVA

#endif  // __DAVAENGINE_UDPSOCKET_H__
