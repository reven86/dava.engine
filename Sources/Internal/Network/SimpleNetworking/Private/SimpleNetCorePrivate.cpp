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

namespace DAVA
{
namespace Net
{

SimpleNetCorePrivate::SimpleNetCorePrivate() 
    : connectionManager(&ioPool)
{
}

SimpleNetCorePrivate::~SimpleNetCorePrivate()
{
    ioPool.Execute(IOPool::SendOperation, true);
    ioPool.CancelAll();

    connectionManager.Shutdown();
    UnregisterAllServices();
}

void SimpleNetCorePrivate::Run()
{
    ioPool.Execute();
}

const SimpleNetService* SimpleNetCorePrivate::RegisterService(
    std::unique_ptr<NetService>&& service,
    IConnectionManager::ConnectionRole role,
    const Endpoint& endPoint,
    const String& serviceName,
    bool waitSuccessfulConnection)
{
    if (serviceName.empty())
        return nullptr;

    const SimpleNetService* serv = GetService(serviceName);
    if (serv)
        return serv;

    size_t serviceId = serviceIdGenerator++;

    //connection wait function
    IConnectionManager* connManager = GetConnectionManager();
    auto connWaiter = [connManager, role] (const Endpoint& endPoint)
    {
        return connManager->CreateConnection(role, endPoint);
    };

    //create net service
    ConnectionListener connListener(connWaiter, endPoint);
    connListener.WaitSuccessfulConnection(waitSuccessfulConnection);
    SimpleNetService netService(serviceId, std::forward<std::unique_ptr<NetService>>(service), 
        endPoint, serviceName, std::move(connListener));

    auto iter = services.emplace(serviceId, std::move(netService));
    return &iter.first->second;
}

IConnectionManager* SimpleNetCorePrivate::GetConnectionManager()
{
    return &connectionManager;
}

bool SimpleNetCorePrivate::IsServiceRegistered(size_t serviceId) const
{
    return GetService(serviceId) != nullptr;
}

bool SimpleNetCorePrivate::IsServiceRegistered(const String& serviceName) const
{
    return GetService(serviceName) != nullptr;
}

void SimpleNetCorePrivate::UnregisterAllServices()
{
    services.clear();
}

const SimpleNetService* SimpleNetCorePrivate::GetService(size_t serviceId) const
{
    auto iter = services.find(serviceId);

    if (iter != services.end())
        return &iter->second;
    return nullptr;
}

const SimpleNetService* SimpleNetCorePrivate::GetService(const String& serviceName) const
{
    if (serviceName.empty())
        return nullptr;

    for (const auto& iter : services)
    {
        if (iter.second.GetServiceName() == serviceName)
        {
            return &iter.second;
        }
    }

    return nullptr;
}

}  // namespace Net
}  // namespace DAVA