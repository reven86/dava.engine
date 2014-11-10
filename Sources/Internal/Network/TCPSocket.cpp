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

#include "TCPSocket.h"

namespace DAVA
{

TCPSocket::TCPSocket(IOLoop* ioLoop) : BaseClassType(ioLoop)
                                     , closeHandler()
                                     , connectHandler()
                                     , readHandler()
{

}

void TCPSocket::Close(CloseHandlerType handler)
{
    DVASSERT(handler != 0);

    closeHandler = handler;
    BaseClassType::Close();
}

int32 TCPSocket::AsyncConnect(const Endpoint& endpoint, ConnectHandlerType handler)
{
    DVASSERT (handler != 0);
    connectHandler = handler;
    return BaseClassType::InternalAsyncConnect(endpoint);
}

int32 TCPSocket::AsyncRead(Buffer buffer, ReadHandlerType handler)
{
    DVASSERT(buffer.base != NULL && buffer.len > 0 && handler != 0);

    readHandler = handler;
    return BaseClassType::InternalAsyncRead(buffer);
}

int32 TCPSocket::AsyncWrite(const Buffer* buffers, std::size_t bufferCount, WriteHandlerType handler)
{
    DVASSERT(buffers != NULL && bufferCount > 0 && handler != 0);

    return BaseClassType::InternalAsyncWrite(buffers, bufferCount, handler);
}

void TCPSocket::HandleClose()
{
    if(closeHandler != 0)
    {
        closeHandler(this);
    }
}

void TCPSocket::HandleConnect(int32 error)
{
    connectHandler(this, error);
}

void TCPSocket::HandleRead(int32 error, size_t nread)
{
    readHandler(this, error, nread);
}

void TCPSocket::HandleWrite(int32 error, const Buffer* buffers, std::size_t bufferCount, WriteHandlerType& handler)
{
    handler(this, error, buffers, bufferCount);
}

}   // namespace DAVA
