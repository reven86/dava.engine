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
#include "AssetCache/CachedFiles.h"
#include "AssetCache/CacheItemKey.h"
#include "AssetCache/TCPConnection/TCPServer.h"
#include "FileSystem/KeyedArchive.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
    
namespace AssetCache
{
    
Server::Server()
    : netServer(nullptr)
{

}
    
Server::~Server()
{
    SafeDelete(netServer);
}

bool Server::Listen(uint16 port)
{
    DVASSERT(nullptr == netServer);
    
    netServer = TCPServer::Create(NET_SERVICE_ID, Net::Endpoint(port));
    netServer->SetDelegate(this);

    return (nullptr != netServer);
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
    DVASSERT(nullptr != netServer);
    SafeDelete(netServer);
}
    
    
void Server::ChannelOpen()
{
}

void Server::ChannelClosed(const char8* message)
{
}

void Server::PacketReceived(const void* packet, size_t length)
{
    if(length)
    {
        ScopedPtr<KeyedArchive> archieve(new KeyedArchive());
        
        const uint8 *packetData = reinterpret_cast<const uint8 *>(packet);
        
        archieve->Deserialize(packetData, length);
        
        const auto packetID = archieve->GetUInt32("PacketID", PACKET_UNKNOWN);
        switch (packetID)
        {
            case PACKET_ADD_FILES_REQUEST:
                OnAddToCache(archieve);
                break;
                
            case PACKET_CHECK_FILE_IN_CACHE_REQUEST:
                OnIsInCache(archieve);
                break;
                
            case PACKET_GET_FILES_REQUEST:
                OnGetFromCache(archieve);
                break;
                
            default:
                Logger::Error("[Server::%s] Cannot parce packet (%d)", __FUNCTION__, packetID);
                break;
        }
        
    }
}

void Server::PacketSent()
{
}

void Server::PacketDelivered()
{
}
    
bool Server::FilesAddedToCache(const CacheItemKey &key, bool added)
{
    if(IsConnected())
    {
        ScopedPtr<KeyedArchive> archieve(new KeyedArchive());
        archieve->SetUInt32("PacketID", PACKET_ADD_FILES_RESPONCE);

        ScopedPtr<KeyedArchive> keyArchieve(new KeyedArchive());
        key.Serialize(keyArchieve);
        archieve->SetArchive("key", keyArchieve);
       
        archieve->SetBool("added", added);
        
        return SendArchieve(archieve);
    }
    
    return false;
}
    
bool Server::FilesInCache(const CacheItemKey &key, bool isInCache)
{
    if(IsConnected())
    {
        ScopedPtr<KeyedArchive> archieve(new KeyedArchive());
        archieve->SetUInt32("PacketID", PACKET_CHECK_FILE_IN_CACHE_RESPONCE);
        
        ScopedPtr<KeyedArchive> keyArchieve(new KeyedArchive());
        key.Serialize(keyArchieve);
        archieve->SetArchive("key", keyArchieve);

        archieve->SetBool("isInCache", isInCache);
        
        return SendArchieve(archieve);
    }
    
    return false;
}
    
bool Server::SendFiles(const CacheItemKey &key, const CachedFiles &files)
{
    if(IsConnected())
    {
        ScopedPtr<KeyedArchive> archieve(new KeyedArchive());
        archieve->SetUInt32("PacketID", PACKET_GET_FILES_RESPONCE);
        
        ScopedPtr<KeyedArchive> keyArchieve(new KeyedArchive());
        key.Serialize(keyArchieve);
        archieve->SetArchive("key", keyArchieve);

        ScopedPtr<KeyedArchive> filesArchieve(new KeyedArchive());
        files.Serialize(filesArchieve, true);
        archieve->SetArchive("files", filesArchieve);
        
        return SendArchieve(archieve);
    }
    
    return false;
}


void Server::OnAddToCache(KeyedArchive * archieve)
{
    if(delegate)
    {
        KeyedArchive *keyArchieve = archieve->GetArchive("key");
        DVASSERT(keyArchieve);
        
        CacheItemKey key;
        key.Deserialize(keyArchieve);
        
        KeyedArchive *filesArchieve = archieve->GetArchive("files");
        DVASSERT(filesArchieve);
        
        CachedFiles files;
        files.Deserialize(filesArchieve);
        
        delegate->OnAddedToCache(key, files);
    }
    else
    {
        Logger::Error("[Server::%s] delegate not installed", __FUNCTION__);
    }
}
    
void Server::OnIsInCache(KeyedArchive * archieve)
{
    if(delegate)
    {
        KeyedArchive *keyArchieve = archieve->GetArchive("key");
        DVASSERT(keyArchieve);
        
        CacheItemKey key;
        key.Deserialize(keyArchieve);
        
        delegate->OnIsInCache(key);
    }
    else
    {
        Logger::Error("[Server::%s] delegate not installed", __FUNCTION__);
    }
}
    
void Server::OnGetFromCache(KeyedArchive * archieve)
{
    if(delegate)
    {
        KeyedArchive *keyArchieve = archieve->GetArchive("key");
        DVASSERT(keyArchieve);
        
        CacheItemKey key;
        key.Deserialize(keyArchieve);
        
        delegate->OnRequestedFromCache(key);
    }
    else
    {
        Logger::Error("[Server::%s] delegate not installed", __FUNCTION__);
    }
}
    
bool Server::SendArchieve(KeyedArchive * archieve)
{
    DVASSERT(archieve);
    
    auto packedSize = archieve->Serialize(nullptr, 0);
    uint8 *packedData = new uint8[packedSize];
    
    DVVERIFY(packedSize == archieve->Serialize(packedData, packedSize));
    
    auto packedId = netServer->SendData(packedData, packedSize);
    return (packedId != 0);
}

    

}; // end of namespace AssetCache
}; // end of namespace DAVA

