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

#include "Base/FunctionTraits.h"

#include "Network/NetworkCommon.h"
#include "Network/NetConfig.h"
#include "Network/NetCore.h"

#include "Thread/LockGuard.h"

namespace DAVA
{
    
Set<uint32> TCPConnection::registeredServices;
Mutex TCPConnection::serviceMutex;
    
TCPConnection::TCPConnection(Net::eNetworkRole _role, uint32 _service, const Net::Endpoint & _endpoint)
    : Net::NetService()
    , service(_service)
    , role(_role)
    , endpoint(_endpoint)
    , controllerId(Net::NetCore::INVALID_TRACK_ID)
{
    bool registered = RegisterService(service);
    if(registered)
    {
        Net::NetConfig config(role);
        config.AddTransport(Net::TRANSPORT_TCP, endpoint);
        config.AddService(service);

        controllerId = Net::NetCore::Instance()->CreateController(config, this);
        if(Net::NetCore::INVALID_TRACK_ID == controllerId)
        {
            Logger::Error("[TCPConnection::TCPConnection] Cannot create controller");
        }
    }
    else
    {
        Logger::Error("[TCPConnection::TCPConnection] Cannot register service(%d)", service);
    }
}
    
    
TCPConnection::~TCPConnection()
{
    if(Net::NetCore::INVALID_TRACK_ID != controllerId)
    {
        Net::NetCore::Instance()->DestroyController(controllerId);
        controllerId = Net::NetCore::INVALID_TRACK_ID;
    }
}
    
    
bool TCPConnection::RegisterService(uint32 service)
{
    LockGuard<Mutex> guard(serviceMutex);
    
    auto registered = registeredServices.count(service) > 0;
    if(!registered)
    {
        registered = Net::NetCore::Instance()->RegisterService(service,
                                                               MakeFunction(&TCPConnection::Create),
                                                               MakeFunction(&TCPConnection::Delete));

        if(registered)
        {
            registeredServices.insert(service);
        }
    }
    
    return registered;
}
    
    
Net::IChannelListener * TCPConnection::Create(uint32 serviceId, void* context)
{
    return reinterpret_cast<Net::IChannelListener *>(context);
}

void TCPConnection::Delete(Net::IChannelListener* obj, void* context)
{
    //not need do anything - called from descructor of TCPClient
}

uint32 TCPConnection::SendData(const uint8 * data, const size_t dataSize)
{
    uint32 packetID = 0;
    bool sent = Send(data, dataSize, &packetID);
    if(sent)
    {
        return packetID;
    }
    
    return 0;
}
    
    
void TCPConnection::ChannelOpen()
{
    if(delegate)
    {
        delegate->ChannelOpen();
    }
}

void TCPConnection::ChannelClosed(const char8* message)
{
    if(delegate)
    {
        delegate->ChannelClosed(message);
    }
}

void TCPConnection::PacketReceived(const void* packet, size_t length)
{
    if(delegate)
    {
        delegate->PacketReceived(packet, length);
    }
}

void TCPConnection::PacketSent()
{
    if(delegate)
    {
        delegate->PacketSent();
    }
}

void TCPConnection::PacketDelivered()
{
    if(delegate)
    {
        delegate->PacketDelivered();
    }
}
    


}; // end of namespace DAVA

