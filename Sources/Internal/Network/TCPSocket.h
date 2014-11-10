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

#include <Base/Function.h>

#include "TCPSocketTemplate.h"

namespace DAVA
{

/*
 Class TCPSocket - fully functional TCP socket implementation which can be used in most cases.
 User can provide functional objects for tracking operation completion on socket.
 Following operation can be tracked:
 1. Socket close, called when socket has been closed
        void (TCPSocket* socket)
 2. Connection established
        void(TCPSocket* socket, int32 error)
 3. Some data has arrived
        void (TCPSocket* socket, int32 error, std::size_t nread)
 4. Data has been sent
        void (TCPSocket* socket, int32 error, const Buffer* buffers, std::size_t bufferCount)

 Functional objects are executed in IOLoop's thread context, and they should not block to allow other operation to complete.
 User is responsible for error processing.

 Methods AsyncConnect, AsyncRead, AsyncWrite and Close should be called from IOLoop's thread, e.g. from inside user's functional objects
*/
class TCPSocket : public TCPSocketTemplate<TCPSocket>
{
private:
    typedef TCPSocketTemplate<TCPSocket> BaseClassType;
    friend BaseClassType;   // Make base class friend to allow him to call my Handle... methods

public:
    typedef Function<void(TCPSocket* socket)>                                                              CloseHandlerType;
    typedef Function<void(TCPSocket* socket, int32 error)>                                                 ConnectHandlerType;
    typedef Function<void(TCPSocket* socket, int32 error, std::size_t nread)>                              ReadHandlerType;
    typedef Function<void(TCPSocket* socket, int32 error, const Buffer* buffers, std::size_t bufferCount)> WriteHandlerType;

public:
    explicit TCPSocket(IOLoop* ioLoop);
    ~TCPSocket() {}

    // Overload Close member to accept handler and unhide Close from base class
    using BaseClassType::Close;
    void Close(CloseHandlerType handler);

    int32 AsyncConnect(const Endpoint& endpoint, ConnectHandlerType handler);
    int32 AsyncRead(Buffer buffer, ReadHandlerType handler);
    int32 AsyncWrite(const Buffer* buffers, std::size_t bufferCount, WriteHandlerType handler);

private:
    void HandleClose();
    void HandleConnect(int32 error);
    void HandleRead(int32 error, size_t nread);
    void HandleWrite(int32 error, const Buffer* buffers, std::size_t bufferCount, WriteHandlerType& handler);

private:
    CloseHandlerType   closeHandler;
    ConnectHandlerType connectHandler;
    ReadHandlerType    readHandler;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPSOCKET_H__
