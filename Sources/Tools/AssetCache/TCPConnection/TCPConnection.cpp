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
#include "Debug/DVAssert.h"
#include "FileSystem/Logger.h"

#include "Network/NetworkCommon.h"
#include "Network/NetConfig.h"
#include "Network/NetCore.h"
#include "Network/Base/Endpoint.h"

#include "Concurrency/LockGuard.h"

namespace DAVA
{
    
TCPConnection::TCPConnection(Net::eNetworkRole _role, uint32 _service, const Net::Endpoint & _endpoint, Net::IChannelListener * _listener)
    : endpoint(_endpoint)
	, listener(_listener)
{
    Connect(_role, _service);
}
    
    
TCPConnection::~TCPConnection()
{
    if(Net::NetCore::INVALID_TRACK_ID != controllerId)
    {
        DisconnectBlocked();
    }
}
    
    
bool TCPConnection::Connect(Net::eNetworkRole role, uint32 service)
{
    bool isRegistered = Net::NetCore::Instance()->IsServiceRegistered(service);
    if(!isRegistered)
    {
        isRegistered = Net::NetCore::Instance()->RegisterService(service,
                                                                 MakeFunction(&TCPConnection::Create),
                                                                 MakeFunction(&TCPConnection::Delete));
    }
    
    if(isRegistered)
    {
        Net::NetConfig config(role);
        config.AddTransport(Net::TRANSPORT_TCP, endpoint);
        config.AddService(service);
        
        controllerId = Net::NetCore::Instance()->CreateController(config, this);
        if(Net::NetCore::INVALID_TRACK_ID != controllerId)
        {
			return true;
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

    return false;
}
    
void TCPConnection::DisconnectBlocked()
{
    DVASSERT(Net::NetCore::INVALID_TRACK_ID != controllerId);
    DVASSERT(Net::NetCore::Instance() != nullptr);

    Net::NetCore::Instance()->DestroyControllerBlocked(controllerId);
	listener = nullptr;
	controllerId = Net::NetCore::INVALID_TRACK_ID;
}
    
    
Net::IChannelListener * TCPConnection::Create(uint32 serviceId, void* context)
{
	auto connection = static_cast<TCPConnection *>(context);
	return connection->listener;
}

void TCPConnection::Delete(Net::IChannelListener* obj, void* context)
{
	//do nothing
	//listener has external creation and deletion
}

  
    


}; // end of namespace DAVA

