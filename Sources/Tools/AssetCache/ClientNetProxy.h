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

#ifndef __DAVAENGINE_ASSET_CACHE_CLIENT_H__
#define __DAVAENGINE_ASSET_CACHE_CLIENT_H__

#include "Base/BaseTypes.h"

#include "Network/IChannel.h"

#include "AssetCache/Connection.h"
#include "AssetCache/CacheItemKey.h"
#include "Network/Base/AddressResolver.h"

namespace DAVA
{
namespace AssetCache
{
class CachedItemValue;

class ClientNetProxyListener
{
public:
    virtual void OnAssetClientStateChanged(){};
    virtual void OnAddedToCache(const CacheItemKey& key, bool added){};
    virtual void OnReceivedFromCache(const CacheItemKey& key, CachedItemValue&& value){};
};

class ClientNetProxy : public DAVA::Net::IChannelListener
{
public:
    ClientNetProxy();

    void AddListener(ClientNetProxyListener*);
    void RemoveListener(ClientNetProxyListener*);

    bool Connect(const String& ip, uint16 port);
    void Disconnect();

    bool ChannelIsOpened();

    bool AddToCache(const CacheItemKey& key, const CachedItemValue& value);
    bool RequestFromCache(const CacheItemKey& key);
    bool WarmingUp(const CacheItemKey& key);

    Connection* GetConnection() const;

    //Net::IChannelListener
    // Channel is open (underlying transport has connection) and can receive and send data through IChannel interface
    void OnChannelOpen(Net::IChannel* channel) override;
    // Channel is closed (underlying transport has disconnected) with reason
    void OnChannelClosed(Net::IChannel* channel, const char8* message) override;
    // Some data arrived into channel
    void OnPacketReceived(Net::IChannel* channel, const void* buffer, size_t length) override;
    // Buffer has been sent and can be reused or freed
    void OnPacketSent(Net::IChannel* channel, const void* buffer, size_t length) override;
    // Data packet with given ID has been delivered to other side
    void OnPacketDelivered(Net::IChannel* channel, uint32 packetId) override{};

    void OnAddressResolved(const Net::Endpoint& endpoint, int32 status);

    void StateChanged();

private:
    Net::AddressResolver addressResolver;
    std::unique_ptr<Connection> netClient;
    DAVA::Net::IChannel* openedChannel = nullptr;

    Set<ClientNetProxyListener*> listeners;
};

inline bool ClientNetProxy::ChannelIsOpened()
{
    return (openedChannel != nullptr);
}

inline Connection* ClientNetProxy::GetConnection() const
{
    return netClient.get();
}

}; // end of namespace AssetCache
}; // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_CLIENT_H__

