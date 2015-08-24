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



#include "AssetCache/AssetCacheClient.h"
#include "AssetCache/AssetCacheConstants.h"
#include "AssetCache/CachedItemValue.h"
#include "AssetCache/CachePacket.h"
#include "FileSystem/KeyedArchive.h"
#include "Debug/DVAssert.h"
#include "FileSystem/DynamicMemoryFile.h"

namespace DAVA {
namespace AssetCache {

Client::Client() : addressResolver(*this)
{}

bool Client::Connect(const String &ip, uint16 port)
{
    DVASSERT(nullptr == netClient);
    DVASSERT(nullptr == openedChannel);
    DVASSERT(addressResolver.GetState() != Net::AddressResolver::State::RESOLVING)

    return addressResolver.StartResolving(ip.c_str(), port);
}


void Client::Disconnect()
{
    addressResolver.Stop();
    netClient.reset();
    openedChannel = nullptr;
}

void Client::OnAddressResolved()
{
    DVASSERT(!netClient);
    DVASSERT(nullptr == openedChannel);

    if (addressResolver.GetState() == Net::AddressResolver::State::RESOLVED)
    {
        auto& addr = addressResolver.Result();
        netClient.reset(new Connection(Net::CLIENT_ROLE, Net::Endpoint(addr.ai_addr), this));
    }
}

bool Client::AddToCache(const CacheItemKey &key, const CachedItemValue &value)
{
    if(openedChannel)
    {
        AddRequestPacket packet(key, value);
        return packet.SendTo(openedChannel);
    }

    return false;
}


bool Client::RequestFromCache(const CacheItemKey &key)
{
    if(openedChannel)
    {
        GetRequestPacket packet(key);
        return packet.SendTo(openedChannel);
    }

    return false;
}

bool Client::WarmingUp(const CacheItemKey &key)
{
    if(openedChannel)
    {
        WarmupRequestPacket packet(key);
        return packet.SendTo(openedChannel);
    }

    return false;
}

void Client::OnChannelOpen(DAVA::Net::IChannel* channel)
{
    DVASSERT(openedChannel == nullptr);
    openedChannel = channel;
}

void Client::OnChannelClosed(DAVA::Net::IChannel* channel, const char8* )
{
    DVASSERT(openedChannel == channel);
    openedChannel = nullptr;
    StateChanged();
}

void Client::StateChanged()
{
    for (auto& listener : listeners)
    {
        listener->OnAssetClientStateChanged();
    }
}

void Client::OnPacketReceived(DAVA::Net::IChannel* channel, const void* packetData, size_t length)
{
    if(listeners.empty())
    {    // do not need to process data in case of nullptr listener
        return;
    }

    DVASSERT(openedChannel == channel);
    if(length > 0)
    {
        CachePacket* packet = CachePacket::Create(static_cast<const uint8 *>(packetData), length);
        if(packet != nullptr)
        {
            switch (packet->type)
            {
            case PACKET_ADD_RESPONSE:
                {
                    AddResponsePacket *p = static_cast<AddResponsePacket*>(packet);
                    for (auto& listener : listeners)
                        listener->OnAddedToCache(p->key, p->added);
                    break;
                }
            case PACKET_GET_RESPONSE:
                {
                    GetResponsePacket* p = static_cast<GetResponsePacket*>(packet);
                    for (auto& listener : listeners)
                        listener->OnReceivedFromCache(p->key, std::forward<CachedItemValue>(p->value));
                    break;
                }
            default:
                {
                    Logger::Error("[AssetCache::Server::%s] Unexpected packet type: (%d).", __FUNCTION__, packet->type);
                    DVASSERT(false);
                    break;
                }
            }

            delete packet;
        }
        else
        {
            DVASSERT(false && "Invalid packet received");
        }
    }
    else
    {
        Logger::Error("[AssetCache::Client::%s] Empty packet is received.", __FUNCTION__);
    }
}

void Client::OnPacketSent(Net::IChannel* channel, const void* buffer, size_t length)
{
    CachePacket::PacketSent(static_cast<const uint8 *> (buffer), length);
}

void Client::AddListener(ClientListener* listener)
{
    DVASSERT(listener != nullptr);
    listeners.insert(listener);
}

void Client::RemoveListener(ClientListener* listener)
{
    DVASSERT(listener != nullptr);
    listeners.erase(listener);
}

} // end of namespace AssetCache
} // end of namespace DAVA

