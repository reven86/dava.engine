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



#include "AssetCache/TCPConnection/TCPConnection.h"
#include "AssetCache/TCPConnection/TCPChannel.h"

#include "Base/FunctionTraits.h"
#include "Debug/DVAssert.h"
#include "FileSystem/Logger.h"

#include "Network/NetworkCommon.h"
#include "Network/NetConfig.h"
#include "Network/NetCore.h"

#include "Concurrency/LockGuard.h"

namespace DAVA
{
    
Set<uint32> TCPConnection::registeredServices;
Mutex TCPConnection::serviceMutex;
    
TCPConnection * TCPConnection::CreateClient(uint32 service, const Net::Endpoint & endpoint)
{
    return new TCPConnection(Net::CLIENT_ROLE, service, endpoint);
}

TCPConnection * TCPConnection::CreateServer(uint32 service, const Net::Endpoint & endpoint)
{
    return new TCPConnection(Net::SERVER_ROLE, service, endpoint);
}
    
    
TCPConnection::TCPConnection(Net::eNetworkRole _role, uint32 _service, const Net::Endpoint & _endpoint)
    : service(_service)
    , role(_role)
    , endpoint(_endpoint)
    , controllerId(Net::NetCore::INVALID_TRACK_ID)
{
    Connect();
}
    
    
TCPConnection::~TCPConnection()
{
    if(Net::NetCore::INVALID_TRACK_ID != controllerId && Net::NetCore::Instance())
    {
        Disconnect();
        //need wait for diconnecting ?
    }
}
    
    
bool TCPConnection::Connect()
{
    isConnected = false;
    bool isRegistered = RegisterService(service);
    if(isRegistered)
    {
        Net::NetConfig config(role);
        config.AddTransport(Net::TRANSPORT_TCP, endpoint);
        config.AddService(service);
        
        controllerId = Net::NetCore::Instance()->CreateController(config, this);
        if(Net::NetCore::INVALID_TRACK_ID != controllerId)
        {
            isConnected = true;
        }
        else
        {
            Logger::Error("[TCPConnection::%s] Cannot create controller", __FUNCTION__);
        }
    }
    else
    {
        Logger::Error("[TCPConnection::%s] Cannot register service(%d)", __FUNCTION__, service);
    }

    return isConnected;
}
    
void TCPConnection::Disconnect()
{
    DVASSERT(Net::NetCore::INVALID_TRACK_ID != controllerId);
    DVASSERT(Net::NetCore::Instance() != nullptr);

    isConnected = false;

    Net::NetCore::Instance()->DestroyControllerBlocked(controllerId);
    controllerId = Net::NetCore::INVALID_TRACK_ID;
}
    
bool TCPConnection::RegisterService(uint32 serviceId)
{
    LockGuard<Mutex> guard(serviceMutex);
    
    auto isRegistered = registeredServices.find(serviceId) != registeredServices.end();
    if(!isRegistered)
    {
        isRegistered = Net::NetCore::Instance()->RegisterService(serviceId,
                                                               MakeFunction(&TCPConnection::Create),
                                                               MakeFunction(&TCPConnection::Delete));

        if(isRegistered)
        {
            registeredServices.insert(serviceId);
        }
    }
    
    return isRegistered;
}
    
    
Net::IChannelListener * TCPConnection::Create(uint32 serviceId, void* context)
{
    auto connection = static_cast<TCPConnection *>(context);
    return connection->CreateChannel();
}

void TCPConnection::Delete(Net::IChannelListener* obj, void* context)
{
    auto connection = static_cast<TCPConnection *>(context);
    auto channel = DynamicTypeCheck<TCPChannel *>(obj);
    return connection->DestroyChannel(channel);
}

TCPChannel * TCPConnection::CreateChannel()
{
    LockGuard<Mutex> guard(channelMutex);

    auto newChannel = new TCPChannel();
    newChannel->SetListener(listener);

    channels.push_back(newChannel);
    return newChannel;
}

void TCPConnection::DestroyChannel(TCPChannel *channel)
{
    LockGuard<Mutex> guard(channelMutex);

    auto found = std::find(channels.cbegin(), channels.cend(), channel);
    if(found != channels.cend())
    {
        channels.erase(found);
    }
    
    delete channel;
}

void TCPConnection::SetListener(TCPChannelListener * _listener)
{
    listener = _listener;
    
    LockGuard<Mutex> guard(channelMutex);
    for(auto ch: channels)
    {
        ch->SetListener(listener);
    }
}
    
const Net::Endpoint & TCPConnection::GetEndpoint() const
{
    return endpoint;
}
    


}; // end of namespace DAVA

