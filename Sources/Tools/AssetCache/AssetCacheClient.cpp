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
#include "FileSystem/KeyedArchive.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
    
namespace AssetCache
{
    
Client::~Client()
{
    listener = nullptr;
}

bool Client::Connect(const String &ip, uint16 port)
{
    DVASSERT(nullptr == netClient);
    DVASSERT(nullptr == openedChannel);
    
	netClient.reset(new Connection(Net::CLIENT_ROLE, Net::Endpoint(ip.c_str(), port), this));
    
    return true;
}
    
void Client::Disconnect()
{
	netClient.reset();
    openedChannel = nullptr;
}
    
    
bool Client::AddToCache(const CacheItemKey &key, const CachedItemValue &value)
{
    if(openedChannel)
    {
        ScopedPtr<KeyedArchive> archieve(new KeyedArchive());
        archieve->SetUInt32("PacketID", PACKET_ADD_REQUEST);

        ScopedPtr<KeyedArchive> keyArchieve(new KeyedArchive());
        SerializeKey(key, keyArchieve);
        archieve->SetArchive("key", keyArchieve);
        
		ScopedPtr<KeyedArchive> valueArchieve(new KeyedArchive());
		value.Serialize(valueArchieve, true);
		archieve->SetArchive("value", valueArchieve);
        
		return SendArchieve(openedChannel, archieve);
	}
    
    return false;
}
    
void Client::OnAddedToCache(KeyedArchive * archieve)
{
    if(listener)
    {
        KeyedArchive *keyArchieve = archieve->GetArchive("key");
        DVASSERT(keyArchieve);
        CacheItemKey key;
        DeserializeKey(key, keyArchieve);
        
        bool added = archieve->GetBool("added");
        
        listener->OnAddedToCache(key, added);
    }
}
    

bool Client::RequestFromCache(const CacheItemKey &key)
{
    if(openedChannel)
    {
        ScopedPtr<KeyedArchive> archieve(new KeyedArchive());
        archieve->SetUInt32("PacketID", PACKET_GET_REQUEST);
        
        ScopedPtr<KeyedArchive> keyArchieve(new KeyedArchive());
        SerializeKey(key, keyArchieve);
        archieve->SetArchive("key", keyArchieve);
        
		return SendArchieve(openedChannel, archieve);
	}
    
    return false;
}

void Client::OnGotFromCache(KeyedArchive * archieve)
{
    if(listener)
    {
        KeyedArchive *keyArchieve = archieve->GetArchive("key");
        DVASSERT(keyArchieve);
        CacheItemKey key;
        DeserializeKey(key, keyArchieve);
        
		KeyedArchive *valueArchieve = archieve->GetArchive("value");
		DVASSERT(valueArchieve);
		CachedItemValue value;
		value.Deserialize(valueArchieve);
        
		listener->OnReceivedFromCache(key, std::forward<CachedItemValue>(value));
    }
}
    
bool Client::WarmingUp(const CacheItemKey &key)
{
    if(openedChannel)
    {
        ScopedPtr<KeyedArchive> archieve(new KeyedArchive());
        archieve->SetUInt32("PacketID", PACKET_WARMING_UP_REQUEST);
        
        ScopedPtr<KeyedArchive> keyArchieve(new KeyedArchive());
        SerializeKey(key, keyArchieve);
        archieve->SetArchive("key", keyArchieve);
        
		return SendArchieve(openedChannel, archieve);
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
}

void Client::OnPacketReceived(DAVA::Net::IChannel* channel, const void* packet, size_t length)
{
	DVASSERT(openedChannel == channel);
	if (length && openedChannel == channel)
    {
        ScopedPtr<KeyedArchive> archieve(new KeyedArchive());
        archieve->Load(static_cast<const uint8 *>(packet), length);
        
        auto packetID = archieve->GetUInt32("PacketID", PACKET_UNKNOWN);
        
        switch (packetID)
        {
            case PACKET_ADD_RESPONSE:
                OnAddedToCache(archieve);
                break;

            case PACKET_GET_RESPONSE:
                OnGotFromCache(archieve);
                break;

            default:
                Logger::Error("[Client::%s] Cannot parce packet (%d)", __FUNCTION__, packetID);
                break;
        }
    }
}

void Client::OnPacketSent(Net::IChannel* channel, const void* buffer, size_t length)
{
    DVASSERT(openedChannel == channel);
    delete[] static_cast<const uint8*>(buffer);
}



}; // end of namespace AssetCache
}; // end of namespace DAVA

