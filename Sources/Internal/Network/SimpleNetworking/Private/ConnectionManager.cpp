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


#include "Network/SimpleNetworking/Private/ConnectionManager.h"

#include "Network/SimpleNetworking/Private/Connection.h"
#include "Network/SimpleNetworking/Private/SimpleTcpClient.h"
#include "Network/SimpleNetworking/Private/SimpleTcpServer.h"

namespace DAVA
{
namespace Net
{
    
IConnectionPtr ConnectionManager::CreateConnection(ConnectionRole role, const Endpoint& endPoint)
{
    if (role == ClientRole)
        return CreateClientConnection(endPoint);
    return CreateServerConnection(endPoint);
}

void ConnectionManager::Shutdown() 
{
    for (auto&& x : sockets)
    {
        ISimpleAbstractSocketPtr socket = x.lock();
        if (socket)
        {
            socket->Shutdown();
        }
    }
}

IConnectionPtr ConnectionManager::CreateServerConnection(const Endpoint& endPoint)
{
    //create socket, listen connection and accept it
    auto socket = CreateSocket<SimpleTcpServer>(endPoint);
    if (!socket->Listen() || !socket->Accept())
        return IConnectionPtr();

    //TODO: use MakeRefPtr
    return IConnectionPtr(new Connection(socket));
}

IConnectionPtr ConnectionManager::CreateClientConnection(const Endpoint& endPoint)
{
    //create socket and connect it
    auto socket = CreateSocket<SimpleTcpClient>(endPoint);
    if (!socket->Connect())
        return IConnectionPtr();

    //TODO: use MakeRefPtr
    return IConnectionPtr(new Connection(socket));
}

template <typename T>
std::shared_ptr<T> ConnectionManager::CreateSocket(const Endpoint& endPoint)
{
    static_assert(std::is_base_of<ISimpleAbstractSocket, T>::value, "Not a ISimpleAbstractSocket");
    auto socket = std::make_shared<T>(endPoint);

    sockets.push_back(socket);
    return socket;
}

}  // namespace Net
}  // namespace DAVA