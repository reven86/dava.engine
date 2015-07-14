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


#include <Debug/DVAssert.h>

#include <Network/Base/UDPSocket.h>

namespace DAVA
{
namespace Net
{

UDPSocket::UDPSocket(IOLoop* ioLoop) : UDPSocketTemplate<UDPSocket>(ioLoop)
                                     , readBuffer()
                                     , closeHandler()
                                     , receiveHandler()
{

}

int32 UDPSocket::StartReceive(Buffer buffer, ReceiveHandlerType handler)
{
    DVASSERT(buffer.base != nullptr && buffer.len > 0 && handler != nullptr);
    readBuffer = buffer;
    receiveHandler = handler;
    return DoStartReceive();
}

int32 UDPSocket::Send(const Endpoint& endpoint, const Buffer* buffers, size_t bufferCount, SendHandlerType handler)
{
    DVASSERT(buffers != nullptr && bufferCount > 0 && handler != nullptr);
    sendHandler = handler;
    return DoSend(buffers, bufferCount, endpoint);
}

void UDPSocket::Close(CloseHandlerType handler)
{
    closeHandler = handler;
    IsOpen() ? DoClose()
             : HandleClose();   // Execute user handle in any case
}

void UDPSocket::ReceiveHere(Buffer buffer)
{
    DVASSERT(buffer.base != nullptr && buffer.len > 0);
    readBuffer = buffer;
}

void UDPSocket::HandleClose()
{
    if (closeHandler != nullptr)
    {
        closeHandler(this);
    }
}

void UDPSocket::HandleAlloc(Buffer* buffer)
{
    *buffer = readBuffer;
}

void UDPSocket::HandleReceive(int32 error, size_t nread, const Endpoint& endpoint, bool partial)
{
    receiveHandler(this, error, nread, endpoint, partial);
}

void UDPSocket::HandleSend(int32 error, const Buffer* buffers, size_t bufferCount)
{
    sendHandler(this, error, buffers, bufferCount);
}

}   // namespace Net
}   // namespace DAVA
