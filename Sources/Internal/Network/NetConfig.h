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


#ifndef __DAVAENGINE_NETCONFIG_H__
#define __DAVAENGINE_NETCONFIG_H__

#include <Base/BaseTypes.h>

#include <Network/Base/Endpoint.h>
#include <Network/NetworkCommon.h>

namespace DAVA
{
namespace Net
{

class NetConfig
{
public:
    struct TransportConfig
    {
        TransportConfig();
        TransportConfig(eTransportType aType, const Endpoint& aEndpoint);

        friend bool operator == (const TransportConfig& left, const TransportConfig& right);

        eTransportType type;
        Endpoint endpoint;
    };

public:
    NetConfig();
    NetConfig(eNetworkRole aRole);
    ~NetConfig();

    bool Validate() const;
    NetConfig Mirror(const IPAddress& addr) const;

    void SetRole(eNetworkRole aRole);
    bool AddTransport(eTransportType type, const Endpoint& endpoint);
    bool AddService(uint32 serviceId);

    eNetworkRole Role() const { return role; }

    const Vector<TransportConfig>& Transports() const { return transports; }
    const Vector<uint32>& Services() const { return services; }

private:
    eNetworkRole role;
    Vector<TransportConfig> transports;
    Vector<uint32> services;
};

//////////////////////////////////////////////////////////////////////////
inline NetConfig::TransportConfig::TransportConfig()
    : type()
    , endpoint()
{}

inline NetConfig::TransportConfig::TransportConfig(eTransportType aType, const Endpoint& aEndpoint)
    : type(aType)
    , endpoint(aEndpoint)
{}

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_NETCONFIG_H__
