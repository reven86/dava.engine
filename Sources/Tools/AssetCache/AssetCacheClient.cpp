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
#include "AssetCache/CachedFiles.h"
#include "AssetCache/CacheItemKey.h"
#include "AssetCache/CachePacket.h"
#include "AssetCache/TCPConnection/TCPConnection.h"
#include "FileSystem/KeyedArchive.h"
#include "Debug/DVAssert.h"
#include "FileSystem/DynamicMemoryFile.h"

namespace DAVA {
namespace AssetCache {


Client::~Client()
{
    listener = nullptr;
}

bool Client::Connect(const String &ip, uint16 port)
{
    DVASSERT(nullptr == netClient);
    DVASSERT(nullptr == openedChannel);
    
    netClient.reset(TCPConnection::CreateClient(NET_SERVICE_ID, Net::Endpoint(ip.c_str(), port)));
    netClient->SetListener(this);
    
    return true;
}
    
void Client::Disconnect()
{
    if(netClient)
    {
        netClient->Disconnect();
        netClient->SetListener(nullptr);
        netClient = nullptr;
        openedChannel = nullptr;
    }
}
    
    
bool Client::AddToCache(const CacheItemKey &key, const CachedFiles &files)
{
    if(openedChannel)
    {
        CachePacket packet;
        packet.type = PACKET_ADD_FILES_REQUEST;
        packet.key = key;
        packet.files = files;
        return (packet.Serialize() && openedChannel->SendData(packet.buffer));
    }
    else
    {
        return false;
    }
}

bool Client::RequestFromCache(const CacheItemKey &key)
{
    if(openedChannel)
    {
        CachePacket packet;
        packet.type = PACKET_GET_FILES_REQUEST;
        packet.key = key;
        return (packet.Serialize() && openedChannel->SendData(packet.buffer));
    }
    else
    {
        return false;
    }
}

bool Client::WarmingUp(const CacheItemKey &key)
{
    if(openedChannel)
    {
        CachePacket packet;
        packet.type = PACKET_WARMING_UP_REQUEST;
        packet.key = key;
        return (packet.Serialize() && openedChannel->SendData(packet.buffer));
    }
    {
        return false;
    }
}


void Client::ChannelOpened(TCPChannel *tcpChannel)
{
    DVASSERT(openedChannel == nullptr);
    openedChannel = tcpChannel;
}

void Client::ChannelClosed(TCPChannel *tcpChannel, const char8* message)
{
    DVASSERT(openedChannel == tcpChannel);
    openedChannel = nullptr;
}

void Client::PacketReceived(DAVA::TCPChannel *tcpChannel, const uint8* packetData, size_t length)
{
    DVASSERT(openedChannel == tcpChannel);
    if(length)
    {
        CachePacket packet;
        packet.buffer.reset(DynamicMemoryFile::Create(packetData, length, File::OPEN | File::READ)); // todo: create without copy

        if (!packet.Deserialize())
        {
            Logger::Error("[AssetCache::Server::%s] Can't deserialize packet. Closing channel", __FUNCTION__);
            netClient->DestroyChannel(tcpChannel);
            return;
        }

        switch (packet.type)
        {
        case PACKET_ADD_FILES_RESPONSE:
        {
            if (listener)
                listener->OnAddedToCache(packet.key, packet.added);
            return;
        }
        case PACKET_GET_FILES_RESPONSE:
        {
            if (listener)
                listener->OnReceivedFromCache(packet.key, packet.files);
            break;
        }
        default:
        {
            Logger::Error("[AssetCache::Server::%s] Invalid packet id: (%d). Closing channel", __FUNCTION__, packet.type);
            netClient->DestroyChannel(tcpChannel);
            break;
        }
        }
    }
    else
    {
        Logger::Error("[AssetCache::Client::%s] Empty packet is received. Closing channel", __FUNCTION__);
        netClient->DestroyChannel(tcpChannel);
    }
}
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

