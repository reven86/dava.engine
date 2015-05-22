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
#include "AssetCache/TCPConnection/TCPClient.h"
#include "FileSystem/KeyedArchive.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
    
namespace AssetCache
{
    
Client::Client()
{

}
    
Client::~Client()
{
    delegate = nullptr;
    SafeDelete(netClient);
}

bool Client::Connect(const String &ip, uint16 port)
{
    DVASSERT(nullptr == netClient);
    
    netClient = TCPClient::Connect(NET_SERVICE_ID, Net::Endpoint(ip.c_str(), port));
    netClient->SetDelegate(this);
    
    return (nullptr != netClient);
}
    
void Client::Disconnect()
{
    DVASSERT(nullptr != netClient);
    SafeDelete(netClient);
}
    
bool Client::IsConnected()
{
    if(netClient)
    {
        return netClient->IsConnected();
    }

    return false;
}
    
bool Client::AddToCache(const CacheItemKey &key, const CachedFiles &files)
{
    if(IsConnected())
    {
        ScopedPtr<KeyedArchive> archieve(new KeyedArchive());
        archieve->SetUInt32("PacketID", PACKET_ADD_FILES_REQUEST);

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
    
void Client::OnAddToCache(KeyedArchive * archieve)
{
    if(delegate)
    {
        KeyedArchive *keyArchieve = archieve->GetArchive("key");
        DVASSERT(keyArchieve);
        CacheItemKey key;
        key.Deserialize(keyArchieve);
        
        bool added = archieve->GetBool("added");
        
        delegate->OnAddedToCache(key, added);
    }
}
    

bool Client::IsInCache(const CacheItemKey &key)
{
    if(IsConnected())
    {
        ScopedPtr<KeyedArchive> archieve(new KeyedArchive());
        archieve->SetUInt32("PacketID", PACKET_CHECK_FILE_IN_CACHE_REQUEST);
        
        ScopedPtr<KeyedArchive> keyArchieve(new KeyedArchive());
        key.Serialize(keyArchieve);
        archieve->SetArchive("key", keyArchieve);
        
        return SendArchieve(archieve);
    }
    
    return false;
}
    
void Client::OnIsInCache(KeyedArchive * archieve)
{
    if(delegate)
    {
        KeyedArchive *keyArchieve = archieve->GetArchive("key");
        DVASSERT(keyArchieve);
        CacheItemKey key;
        key.Deserialize(keyArchieve);
        
        bool isInCache = archieve->GetBool("isInCache");
        delegate->OnIsInCache(key, isInCache);
    }
}


bool Client::GetFromCache(const CacheItemKey &key)
{
    if(IsConnected())
    {
        ScopedPtr<KeyedArchive> archieve(new KeyedArchive());
        archieve->SetUInt32("PacketID", PACKET_GET_FILES_REQUEST);
        
        ScopedPtr<KeyedArchive> keyArchieve(new KeyedArchive());
        key.Serialize(keyArchieve);
        archieve->SetArchive("key", keyArchieve);
        
        return SendArchieve(archieve);
    }
    
    return false;
}

void Client::OnGetFromCache(KeyedArchive * archieve)
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
        
        delegate->OnReceivedFromCache(key, files);
    }
}

    
bool Client::SendArchieve(KeyedArchive * archieve)
{
    DVASSERT(archieve);
    
    auto packedSize = archieve->Serialize(nullptr, 0);
    uint8 *packedData = new uint8[packedSize];
    
    DVVERIFY(packedSize == archieve->Serialize(packedData, packedSize));
    
    auto packedId = netClient->SendData(packedData, packedSize);
    return (packedId != 0);
}

    
void Client::ChannelOpen()
{
}

void Client::ChannelClosed(const char8* message)
{
}

void Client::PacketReceived(const void* packet, size_t length)
{
    if(length)
    {
        ScopedPtr<KeyedArchive> archieve(new KeyedArchive());

        const uint8 *packetData = reinterpret_cast<const uint8 *>(packet);
        
        archieve->Deserialize(packetData, length);
        
        auto packetID = archieve->GetUInt32("PacketID", PACKET_UNKNOWN);
        
        switch (packetID)
        {
            case PACKET_ADD_FILES_RESPONCE:
                OnAddToCache(archieve);
                break;

            case PACKET_CHECK_FILE_IN_CACHE_RESPONCE:
                OnIsInCache(archieve);
                break;

            case PACKET_GET_FILES_RESPONCE:
                OnGetFromCache(archieve);
                break;

            default:
                Logger::Error("[Client::%s] Cannot parce packet (%d)", __FUNCTION__, packetID);
                break;
        }
        
    }
}

void Client::PacketSent()
{
}

void Client::PacketDelivered()
{
}
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

