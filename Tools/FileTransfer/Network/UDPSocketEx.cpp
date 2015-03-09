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

#include "UDPSocketEx.h"

namespace DAVA
{

UDPSocketEx::UDPSocketEx(IOLoop* ioLoop) : BaseClassType(ioLoop)
                                         , closeHandler()
                                         , receiveHandler()
{

}

void UDPSocketEx::SetCloseHandler(CloseHandlerType handler)
{
    closeHandler = handler;
}

int32 UDPSocketEx::AsyncReceive(void* buffer, std::size_t size, ReceiveHandlerType handler)
{
    DVASSERT(buffer != NULL && size > 0 && handler != 0);

    receiveHandler = handler;
    return BaseClassType::InternalAsyncReceive(buffer, size);
}

int32 UDPSocketEx::AsyncSend(const Endpoint& endpoint, const void* buffer, std::size_t size, SendHandlerType handler)
{
    DVASSERT(buffer != NULL && size > 0 && handler != 0);

    SendRequest* request = new SendRequest();
    request->sendHandler = handler;
    return BaseClassType::InternalAsyncSend(request, buffer, size, endpoint);
}

void UDPSocketEx::HandleClose()
{
    if(closeHandler != 0)
    {
        closeHandler(this);
    }
}

void UDPSocketEx::HandleReceive(int32 error, std::size_t nread, const uv_buf_t* buffer, const Endpoint& endpoint, bool partial)
{
    receiveHandler(this, error, nread, buffer->base, endpoint, partial);
}

void UDPSocketEx::HandleSend(SendRequest* request, int32 error)
{
    request->sendHandler(this, error, request->buffer.base);
    delete request;
}

}   // namespace DAVA
