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

#include <Base/Function.h>

#include "UDPSocketTemplate.h"

namespace DAVA
{

/*
 Class UDPSocket - fully functional UDP socket implementation which can be used in most cases.
 User can provide functional objects for tracking operation completion on socket.
 Following operation can be tracked:
 1. Socket close, called when socket has been closed
        void (UDPSocket* socket)
 2. Datagram has arrived
        void (UDPSocket* socket, int32 error, std::size_t nread, const Endpoint& endpoint, bool partial)
 3. Datagram has been sent
        void (UDPSocket* socket, int32 error, const Buffer* buffers, std::size_t bufferCount)

 Functional objects are executed in IOLoop's thread context, and they should not block to allow other operation to complete.
 User is responsible for error processing.

 Methods AsyncReceive, AsyncSend and Close should be called from IOLoop's thread, e.g. from inside user's functional objects
*/

class UDPSocket : public UDPSocketTemplate<UDPSocket>
{
private:
    typedef UDPSocketTemplate<UDPSocket> BaseClassType;
    friend BaseClassType;   // Make base class friend to allow him to call my Handle... methods

public:
    typedef Function<void(UDPSocket* socket)>                                                                         CloseHandlerType;
    typedef Function<void(UDPSocket* socket, int32 error, std::size_t nread, const Endpoint& endpoint, bool partial)> ReceiveHandlerType;
    typedef Function<void(UDPSocket* socket, int32 error, const Buffer* buffers, std::size_t bufferCount)>            SendHandlerType;

public:
    UDPSocket(IOLoop* ioLoop);
    ~UDPSocket() {}

    // Overload Close member to accept handler and unhide Close from base class
    using BaseClassType::Close;
    void Close(CloseHandlerType handler);

    int32 AsyncReceive(Buffer buffer, ReceiveHandlerType handler);
    int32 AsyncSend(const Endpoint& endpoint, const Buffer* buffers, std::size_t bufferCount, SendHandlerType handler);

    void AdjustReadBuffer(Buffer buffer);

private:
    void HandleClose();
    void HandleAlloc(Buffer* buffer);
    void HandleReceive(int32 error, std::size_t nread, const Endpoint& endpoint, bool partial);
    void HandleSend(int32 error, const Buffer* buffers, std::size_t bufferCount, SendHandlerType& handler);

private:
    Buffer             readBuffer;
    CloseHandlerType   closeHandler;
    ReceiveHandlerType receiveHandler;
};

}   // namespace DAVA

#endif  // __DAVAENGINE_UDPSOCKET_H__
