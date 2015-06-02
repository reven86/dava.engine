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


#include <algorithm>

#include <Debug/DVAssert.h>

#include <Network/NetConfig.h>

namespace DAVA
{
namespace Net
{

bool operator == (const NetConfig::TransportConfig& left, const NetConfig::TransportConfig& right)
{
    return left.type == right.type && left.endpoint == right.endpoint;
}

NetConfig::NetConfig() : role()
{}

NetConfig::NetConfig(eNetworkRole aRole) : role(aRole)
{}

NetConfig::~NetConfig() {}

bool NetConfig::Validate() const
{
    return false == transports.empty() && false == services.empty();
}

NetConfig NetConfig::Mirror(const IPAddress& addr) const
{
    DVASSERT(true == Validate());
    NetConfig result(SERVER_ROLE == role ? CLIENT_ROLE : SERVER_ROLE);
    result.transports = transports;
    result.services = services;
    for (Vector<TransportConfig>::iterator i = result.transports.begin(), e = result.transports.end();i != e;++i)
    {
        uint16 port = (*i).endpoint.Port();
        (*i).endpoint = Endpoint(addr, port);
    }
    return result;
}

void NetConfig::SetRole(eNetworkRole aRole)
{
    DVASSERT(true == transports.empty() && true == services.empty());
    role = aRole;
}

bool NetConfig::AddTransport(eTransportType type, const Endpoint& endpoint)
{
    TransportConfig config(type, endpoint);
    DVASSERT(std::find(transports.begin(), transports.end(), config) == transports.end());
    if (std::find(transports.begin(), transports.end(), config) == transports.end())
    {
        transports.push_back(config);
        return true;
    }
    return false;
}

bool NetConfig::AddService(uint32 serviceId)
{
    DVASSERT(std::find(services.begin(), services.end(), serviceId) == services.end());
    if (std::find(services.begin(), services.end(), serviceId) == services.end())
    {
        services.push_back(serviceId);
        return true;
    }
    return false;
}

}   // namespace Net
}   // namespace DAVA
