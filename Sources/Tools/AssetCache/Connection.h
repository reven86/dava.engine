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


#ifndef __DAVAENGINE_ASSETCACHE_CONNECTION_H__
#define __DAVAENGINE_ASSETCACHE_CONNECTION_H__

#include "Network/Base/Endpoint.h"
#include "Network/NetworkCommon.h"
#include "Network/NetCore.h"

namespace DAVA
{
namespace Net
{
    struct IChannel;
}

class KeyedArchive;
namespace AssetCache
{

bool SendArchieve(Net::IChannel* channel, KeyedArchive *archieve);
    
class Connection final
{
public:
    Connection(Net::eNetworkRole role, const Net::Endpoint & endpoint, Net::IChannelListener * listener, Net::eTransportType transport = Net::TRANSPORT_TCP);
    ~Connection();

    const Net::Endpoint & GetEndpoint() const;

private:

    bool Connect(Net::eNetworkRole _role, Net::eTransportType transport);
    void DisconnectBlocked();

    static Net::IChannelListener * Create(uint32 serviceId, void* context);
    static void Delete(Net::IChannelListener* obj, void* context);

private:
    
    Net::Endpoint endpoint;
    Net::NetCore::TrackId controllerId = Net::NetCore::INVALID_TRACK_ID;

    Net::IChannelListener *listener = nullptr;
};

inline const Net::Endpoint & Connection::GetEndpoint() const
{
    return endpoint;
}

} // end of namespace AssetCache
} // end of namespace DAVA

#endif // __DAVAENGINE_ASSETCACHE_CONNECTION_H__

