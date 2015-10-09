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


#include "Network/SimpleNetworking/Private/LogNetworkError.h"
#include "Network/SimpleNetworking/Private/SimpleTcpSocket.h"
#include <libuv/uv.h>

namespace DAVA
{
namespace Net
{

SimpleTcpSocket::SimpleTcpSocket(IOPool* ioPoolPtr, const Endpoint& endPoint)
    : ioPool(ioPoolPtr)
    , socketEndPoint(endPoint)
{
    socketId = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

SimpleTcpSocket::~SimpleTcpSocket()
{
    Shutdown();
    Close();
}

bool SimpleTcpSocket::Shutdown()
{
    if (!IsConnectionEstablished())
    {
        if (socketId != DV_INVALID_SOCKET)
        {
            Close();
            return true;
        }
        return false;
    }

    if (socketId == DV_INVALID_SOCKET)
    {
        return false;
    }
    
    ::shutdown(socketId, DV_SD_BOTH);
    Close();
    return true;
}

bool SimpleTcpSocket::Send(IOPool::BufferPtr&& buf, size_t bufSize, const IOPool::SendCallback& cb)
{
    if (!IsConnectionEstablished())
        return false;

    IOPool::SendCallback callback(cb);
    auto sendCallback = [this, callback] (size_t size)
    {
        if (!CheckSocketResult(static_cast<int>(size)))
        {
            Close();
            callback(0);
            return;
        }

        callback(size);
    };

    ioPool->AddSendOperation(socketId, std::move(buf), bufSize, sendCallback);
    return true;
}

bool SimpleTcpSocket::Recv(const IOPool::RecvCallback& cb, size_t* recvBytesCount)
{
    if (!IsConnectionEstablished())
        return false;

    IOPool::RecvCallback callback(cb);
    auto sendCallback = [this, callback](const char* buf, size_t size)
    {
        if (!CheckSocketResult(static_cast<int>(size)))
        {
            Close();
            callback(nullptr, 0);
            return;
        }

        callback(buf, size);
    };

    ioPool->AddReceiveOperation(socketId, cb, recvBytesCount);
    return true;
}

void SimpleTcpSocket::Close()
{
    if (socketId != DV_INVALID_SOCKET)
    {
        socket_t socket = socketId;
        socketId = DV_INVALID_SOCKET;
        connectionEstablished = false;
        socketEndPoint = Endpoint();

        CloseSocket(socket);
    }
}
    
}  // namespace Net
}  // namespace DAVA