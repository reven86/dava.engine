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



#include "AssetCache/AssetCacheServer.h"
#include "AssetCache/AssetCacheConstants.h"
#include "AssetCache/CachedItemValue.h"
#include "AssetCache/CacheItemKey.h"
#include "AssetCache/CachePacket.h"
#include "AssetCache/TCPConnection/TCPConnection.h"
#include "Debug/DVAssert.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/DynamicMemoryFile.h"


namespace DAVA {
namespace AssetCache {

Server::~Server()
{
}

bool Server::Listen(uint16 port)
{
    listenPort = port;
    DVASSERT(!netServer);
    
    netServer.reset(TCPConnection::CreateServer(NET_SERVICE_ID, Net::Endpoint(listenPort)));
    netServer->SetListener(this);

    return netServer->IsConnected();
}
    
bool Server::IsConnected() const
{
    if(netServer)
    {
        return netServer->IsConnected();
    }
    
    return false;
}

void Server::Disconnect()
{
    if(netServer)
    {
        netServer->Disconnect();
        netServer->SetListener(nullptr);
        netServer.reset();
    }
}
    
void Server::PacketReceived(DAVA::TCPChannel *tcpChannel, const uint8* packetData, size_t length)
{
    Logger::FrameworkDebug("[channel %d] Packet received", tcpChannel);
    if(length && delegate)
    {
        CachePacket* packet = CachePacket::Deserialize(packetData, length);
        if (packet)
        {
            Logger::FrameworkDebug("Packet type: %d, length %d", packet->type, length);

            switch (packet->type)
            {
            case PACKET_ADD_REQUEST:
            {
                AddRequestPacket *p = static_cast<AddRequestPacket*>(packet);
                delegate->OnAddToCache(tcpChannel, p->key, std::forward<CachedItemValue>(p->value));
                return;
            }
            case PACKET_GET_REQUEST:
            {
                GetRequestPacket* p = static_cast<GetRequestPacket*>(packet);
                delegate->OnRequestedFromCache(tcpChannel, p->key);
                return;
            }
            case PACKET_WARMING_UP_REQUEST:
            {
                WarmupRequestPacket* p = static_cast<WarmupRequestPacket*>(packet);
                delegate->OnWarmingUp(tcpChannel, p->key);
                return;
            }
            default:
            {
                Logger::Error("[AssetCache::Server::%s] Unexpected packet type: (%d). Closing channel", __FUNCTION__, packet->type);
                netServer->DestroyChannel(tcpChannel);
                return;
            }
            }
        }
        else
        {
            Logger::Error("[AssetCache::Server::%s] Invalid packet received. Closing channel", __FUNCTION__, packet->type);
            netServer->DestroyChannel(tcpChannel);
            return;
        }
    }
    else
    {
        if (!length)
        {
            Logger::Error("[AssetCache::Server::%s] Empty packet is received. Closing channel", __FUNCTION__);
            netServer->DestroyChannel(tcpChannel);
        }
        if (!delegate)
        {
            Logger::Error("Server delegate is not set");
        }
    }
}
    
void Server::ChannelClosed(TCPChannel *tcpChannel, const char8* message)
{
    Logger::FrameworkDebug("[channel %d] closed");
    if(delegate)
    {
        delegate->OnChannelClosed(tcpChannel, message);
    }
}

    
bool Server::AddedToCache(DAVA::TCPChannel *tcpChannel, const CacheItemKey &key, bool added)
{
    if(tcpChannel)
    {
        AddResponsePacket packet(key, added);
        return packet.SendTo(tcpChannel);
    }
    else
    {
        return false;
    }
}
    
    
bool Server::Send(DAVA::TCPChannel *tcpChannel, const CacheItemKey &key, const CachedItemValue &value)
{
    Logger::FrameworkDebug("Sending response packet, value size: %d", value.GetSize());
    if (tcpChannel)
    {
        GetResponsePacket packet(key, value);
        return packet.SendTo(tcpChannel);
    }
    else
    {
        Logger::FrameworkDebug("Cancel send: no tcp channel");
        return false;
    }
}

}; // end of namespace AssetCache
}; // end of namespace DAVA

