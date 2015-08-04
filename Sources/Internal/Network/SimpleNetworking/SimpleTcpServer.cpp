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


#include "Network/SimpleNetworking/SimpleTcpServer.h"

#include <libuv/uv.h>
#include "Debug/DVAssert.h"
#include "Network/SimpleNetworking/SimpleNetworking.h"

namespace DAVA
{
namespace Net
{

namespace TCP
{
    
SimpleTcpServer::SimpleTcpServer()
{
    socket_id = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (socket_id == INVALID_SOCKET)
    {
        LogNetworkError("Failed to create socket");
    }
    else
    {
        connectionEstablished = true;
    }
}

SimpleTcpServer::~SimpleTcpServer()
{
    Shutdown();
    Close();
}

void SimpleTcpServer::Listen(const Endpoint& endPoint)
{
    if (socket_id == INVALID_SOCKET)
    {
        DVASSERT_MSG(false, "Unable to listen server - it is invalid");
        return;
    }
    
    Bind(endPoint);
    
    int listenRes = ::listen(socket_id, 1);
    if (listenRes == SOCKET_ERROR)
    {   
        LogNetworkError("Failed to listen socket");
        Close();
    }
}

void SimpleTcpServer::Accept() 
{
    if (socket_id == INVALID_SOCKET)
    {
        DVASSERT_MSG(false, "Unable to accept server - it is invalid");
        return;
    }
    
    SOCKET acceptSocket = ::accept(socket_id, nullptr, nullptr);
    if (acceptSocket == INVALID_SOCKET)
    {
        LogNetworkError("Failed to accept socket");
        return;
    }
    
    ::closesocket(socket_id);
    socket_id = acceptSocket;
    connectionEstablished = true;
}

const Endpoint& SimpleTcpServer::GetEndpoint()
{
    return socketEndPoint;
}

void SimpleTcpServer::Shutdown()
{
    if (!connectionEstablished)
        return;

    if (socket_id == INVALID_SOCKET)
    {
        DVASSERT_MSG(false, "Unable to shutdown server - it is invalid");
        return;
    }
    
    int res = ::shutdown(socket_id, SD_BOTH);
    if (res == SOCKET_ERROR)
    {
        LogNetworkError("Failed to shutdown connection");
    }
}

size_t SimpleTcpServer::Send(const char* buf, size_t bufSize)
{
    int size = ::send(socket_id, buf, bufSize, 0);
    
    if (size == SOCKET_ERROR)
    {
        LogNetworkError("Failed to send data");
        Close();

        return 0;
    }
    
    return static_cast<size_t>(size);
}

size_t SimpleTcpServer::Recv(char* buf, size_t bufSize, bool recvAll)
{
    int flags = recvAll ? MSG_WAITALL : 0;
    int size = ::recv(socket_id, buf, bufSize, flags);
    
    if (size == SOCKET_ERROR)
    {
        LogNetworkError("Failed to receive data");
        Close();

        return 0;
    }
    
    return static_cast<size_t>(size);
}

void SimpleTcpServer::Bind(const Endpoint& endPoint)
{
    const sockaddr* addr = reinterpret_cast<const sockaddr*>(endPoint.CastToSockaddrIn());

    int bindRes = ::bind(socket_id, addr, endPoint.Size());
    if (bindRes == SOCKET_ERROR)
    {
        LogNetworkError("Failed to bind socket");
        Close();

        return;
    }

    socketEndPoint = endPoint;
}

void SimpleTcpServer::Close()
{
    if (socket_id != INVALID_SOCKET)
    {
        ::closesocket(socket_id);
        socket_id = INVALID_SOCKET;
        connectionEstablished = false;
        socketEndPoint = Endpoint();
    }
}
    
}  // namespace TCP

}  // namespace Net
}  // namespace DAVA