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

#include "AssetCache/ServerNetProxy.h"
#include "AssetCache/AssetCacheConstants.h"
#include "AssetCache/CachedItemValue.h"
#include "AssetCache/CachePacket.h"
#include "Debug/DVAssert.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/DynamicMemoryFile.h"

namespace DAVA
{
namespace AssetCache
{
ServerNetProxy::~ServerNetProxy()
{
}

void ServerNetProxy::Listen(uint16 port)
{
    listenPort = port;
    DVASSERT(!netServer);

    netServer.reset(new Connection(Net::SERVER_ROLE, Net::Endpoint(listenPort), this));
}

void ServerNetProxy::Disconnect()
{
    netServer.reset();
}

void ServerNetProxy::OnPacketReceived(Net::IChannel* channel, const void* packetData, size_t length)
{
    if (nullptr == delegate)
    { // do not need to process data in case of nullptr delegate
        return;
    }

    if (length > 0)
    {
        std::unique_ptr<CachePacket> packet = CachePacket::Create(static_cast<const uint8*>(packetData), length);
        if (packet)
        {
            switch (packet->type)
            {
            case PACKET_ADD_REQUEST:
            {
                AddRequestPacket* p = static_cast<AddRequestPacket*>(packet.get());
                delegate->OnAddToCache(channel, p->key, std::forward<CachedItemValue>(p->value));
                break;
            }
            case PACKET_GET_REQUEST:
            {
                GetRequestPacket* p = static_cast<GetRequestPacket*>(packet.get());
                delegate->OnRequestedFromCache(channel, p->key);
                break;
            }
            case PACKET_WARMING_UP_REQUEST:
            {
                WarmupRequestPacket* p = static_cast<WarmupRequestPacket*>(packet.get());
                delegate->OnWarmingUp(channel, p->key);
                break;
            }
            default:
            {
                Logger::Error("[AssetCache::ServerNetProxy::%s] Unexpected packet type: (%d). Closing channel", __FUNCTION__, packet->type);
                DVASSERT(false);
                break;
            }
            }
        }
        else
        {
            DVASSERT(false && "Invalid packet received");
        }
    }
    else
    {
        Logger::Error("[AssetCache::ServerNetProxy::%s] Empty packet is received.", __FUNCTION__);
    }
}

void ServerNetProxy::OnPacketSent(Net::IChannel* channel, const void* buffer, size_t length)
{
    CachePacket::PacketSent(static_cast<const uint8*>(buffer), length);
}

void ServerNetProxy::OnChannelClosed(Net::IChannel* channel, const char8* message)
{
    if (delegate)
    {
        delegate->OnChannelClosed(channel, message);
    }
}

bool ServerNetProxy::AddedToCache(Net::IChannel* channel, const CacheItemKey& key, bool added)
{
    if (channel)
    {
        AddResponsePacket packet(key, added);
        return packet.SendTo(channel);
    }

    return false;
}

bool ServerNetProxy::Send(Net::IChannel* channel, const CacheItemKey& key, const CachedItemValue& value)
{
    if (channel)
    {
        GetResponsePacket packet(key, value);
        return packet.SendTo(channel);
    }

    return false;
}

}; // end of namespace AssetCache
}; // end of namespace DAVA

