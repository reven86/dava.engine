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
#include "Network/SimpleNetworking/Private/SimpleTcpServer.h"

namespace DAVA
{
namespace Net
{
    
unsigned ConnectionManager::GetAvailableConnectionRoles()
{
#if WINAPI_FAMILY_PARTITION(WINAPI_FAMILY_PHONE_APP)
    return kServerRole;
#else
    return kServerRole | kClientRole;
#endif
}

IConnectionPtr ConnectionManager::CreateConnection(ConnectionRole role, const Endpoint& endPoint)
{
    if (role == kClientRole)
        return CreateClientConnection(endPoint);
    return CreateServerConnection(endPoint);
}

IConnectionPtr ConnectionManager::CreateServerConnection(const Endpoint& endPoint)
{
    //create socket, listen connection and accept it
    auto socket = std::make_unique<SimpleTcpServer>();
    socket->Listen(endPoint);
    socket->Accept();

    return MakeRefPtr<Connection>(std::move(socket));
}

IConnectionPtr ConnectionManager::CreateClientConnection(const Endpoint& endPoint)
{
    return IConnectionPtr();
}

}  // namespace Net
}  // namespace DAVA