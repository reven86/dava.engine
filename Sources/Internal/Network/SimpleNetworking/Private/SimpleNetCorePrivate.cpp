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


#include "Network/SimpleNetworking/Private/SimpleNetCorePrivate.h"

#include "Network/SimpleNetworking/SimpleConnectionListener.h"
#include "Network/SimpleNetworking/SimpleNetService.h"
#include "Network/SimpleNetworking/Private/SimpleConnectionListenerPrivate.h"

namespace DAVA
{
namespace Net
{

size_t SimpleNetCorePrivate::RegisterService(std::unique_ptr<NetService>&& service,
                                            IConnectionManager::ConnectionRole role,
                                            const Endpoint& endPoint,
                                            const String& serviceName,
                                            NotificationType notifType)
{
    if (IsServiceRegistered(serviceName))
        return 0;

    size_t serviceId = serviceIdGenerator++;

    //connection wait function
    IConnectionManager* connManager = GetConnectionManager();
    auto connWaiter = [=](const Endpoint& endPoint) -> IConnectionPtr
    {
        unsigned roles = connManager->GetAvailableConnectionRoles();
        if ((roles & role) == 0)
        {
            DVASSERT_MSG(false, "Requsted connection role for NetService is unavailable");
            return IConnectionPtr();
        }

        return connManager->CreateConnection(role, endPoint);
    };

    //create net service
    ConnectionListener connListener(connWaiter, endPoint, notifType);
    SimpleNetService netService
        (serviceId, std::move(service), endPoint, serviceName, std::move(connListener));

    services.emplace(serviceId, std::move(netService));
    return serviceId;
}

bool SimpleNetCorePrivate::IsServiceRegistered(size_t serviceId) const
{
    auto iter = services.find(serviceId);
    return iter != services.end();
}

bool SimpleNetCorePrivate::IsServiceRegistered(const String& serviceName) const
{
    for (const auto& iter : services)
    {
        if (iter.second.GetServiceName() == serviceName)
        {
            return true;
        }
    }

    return false;
}

void SimpleNetCorePrivate::UnregisterAllServices()
{
    services.clear();
}

String SimpleNetCorePrivate::GetServiceName(size_t serviceId) const
{
    auto iter = services.find(serviceId);

    if (iter != services.end())
        return iter->second.GetServiceName();
    return "";
}

size_t SimpleNetCorePrivate::GetServiceId(const String& serviceName) const
{
    if (serviceName.empty())
        return 0;

    for (const auto& iter : services)
    {
        if (iter.second.GetServiceName() == serviceName)
        {
            return iter.second.GetServiceId();
        }
    }

    return 0;
}

Endpoint SimpleNetCorePrivate::GetServiceEndpoint(size_t serviceId) const
{
    auto iter = services.find(serviceId);

    if (iter != services.end())
        return iter->second.GetServiceEndpoint();
    return Endpoint();
}

}  // namespace Net
}  // namespace DAVA