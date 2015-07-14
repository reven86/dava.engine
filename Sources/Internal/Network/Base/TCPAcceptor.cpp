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

#include <Network/Base/TCPSocket.h>
#include <Network/Base/TCPAcceptor.h>

namespace DAVA
{
namespace Net
{

TCPAcceptor::TCPAcceptor(IOLoop* ioLoop) : TCPAcceptorTemplate(ioLoop)
                                         , closeHandler()
                                         , connectHandler()
{

}

int32 TCPAcceptor::StartListen(ConnectHandlerType handler, int32 backlog)
{
    DVASSERT(handler != nullptr);
    connectHandler = handler;
    return DoStartListen(backlog);
}

void TCPAcceptor::Close(CloseHandlerType handler)
{
    closeHandler = handler;
    IsOpen() ? DoClose()
             : HandleClose();   // Execute user handle in any case
}

int32 TCPAcceptor::Accept(TCPSocket* socket)
{
    DVASSERT(socket != NULL && false == socket->IsOpen());
    // To do the following TCPAcceptor must be friend of TCPSocket
    int32 error = socket->DoOpen();
    if (0 == error)
        error = DoAccept(&socket->uvhandle);
    return error;
}

void TCPAcceptor::HandleClose()
{
    if(closeHandler != nullptr)
    {
        closeHandler(this);
    }
}

void TCPAcceptor::HandleConnect(int32 error)
{
    connectHandler(this, error);
}

}   // namespace Net
}   // namespace DAVA
