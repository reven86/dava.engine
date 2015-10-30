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


#ifndef __DAVAENGINE_SIMPLE_NET_CORE_H__
#define __DAVAENGINE_SIMPLE_NET_CORE_H__

#include <memory.h>

#include "Concurrency/Atomic.h"
#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Network/NetService.h"
#include "Network/Base/Endpoint.h"
#include "Network/SimpleNetworking/IConnection.h"

namespace DAVA
{
namespace Net
{

struct IConnectionManager
{
    enum ConnectionRole
    {
        ServerRole = 0x1,
        ClientRole = 0x2
    };
    virtual IConnectionPtr CreateConnection(ConnectionRole role,
                                            const Endpoint& endPoint) = 0;

    virtual ~IConnectionManager() {}
};

class SimpleNetService;

class SimpleNetCore : public Singleton<SimpleNetCore>
{
public:
    //Port number for connecting to UWP app on local machine
    static const uint16 UWPLocalPort = 777;
    //Port number for connecting to UWP app on mobile device (via IpOverUSB service)
    static const uint16 UWPRemotePort = 1911; //0x777

    SimpleNetCore();
    ~SimpleNetCore();
    
    IConnectionManager* GetConnectionManager();

    bool IsServiceRegistered(size_t serviceId) const;
    bool IsServiceRegistered(const String& serviceName) const;

    const SimpleNetService* RegisterService(
        std::unique_ptr<NetService>&& service,
        IConnectionManager::ConnectionRole role,
        const Endpoint& endPoint,
        const String& serviceName,
        bool sendOnly = false);

    const SimpleNetService* GetService(size_t serviceId) const;
    const SimpleNetService* GetService(const String& serviceName) const;
	
	Atomic<bool> interruptionFlag;
        
private:
    std::unique_ptr<class SimpleNetCorePrivate> pimpl;
};

}  // namespace Net
}  // namespace DAVA

#endif  // __DAVAENGINE_SIMPLE_NET_CORE_H__