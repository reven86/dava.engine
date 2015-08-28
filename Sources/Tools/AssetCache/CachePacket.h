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


#ifndef __DAVAENGINE_ASSET_CACHE_SERVER_CACHE_PACKET_H__
#define __DAVAENGINE_ASSET_CACHE_SERVER_CACHE_PACKET_H__

#include <memory>
#include "FileSystem/DynamicMemoryFile.h"
#include "AssetCache/CacheItemKey.h"
#include "AssetCache/CachedItemValue.h"
#include "AssetCache/AssetCacheConstants.h"

namespace DAVA 
{

namespace Net
{
    struct IChannel;
}

namespace AssetCache 
{

#pragma pack(1)

#pragma pack(push, 1) // exact fit - no padding
struct CachePacketHeader
{
    CachePacketHeader() {};
    CachePacketHeader(uint16 header, uint8 _version, ePacketID type) : headerID(header), version(_version), packetType(type) {}

    uint16 headerID = 0;
    uint8 version = 0;
    ePacketID packetType = PACKET_UNKNOWN;
};
#pragma pack(pop) //back to whatever the previous packing mode was



class CachePacket
{ 
public:
    virtual ~CachePacket() {};  // this is base class for asset cache network packets

    static std::unique_ptr<CachePacket> Create(const uint8* buffer, uint32 length);

    bool SendTo(Net::IChannel* channel);
    static void PacketSent(const uint8* buffer, size_t length);

protected:
    CachePacket(ePacketID type, bool createBuffer);

    static std::unique_ptr<CachePacket> CreateByType(ePacketID type);

    void WriteHeader(File *file) const;
    virtual bool Load(File *file) = 0;

public:
    ePacketID type = PACKET_UNKNOWN;
    ScopedPtr<DynamicMemoryFile> serializationBuffer;

private:
    static List<ScopedPtr<DynamicMemoryFile>> sendingPackets;
};


class AddRequestPacket : public CachePacket
{
public:
    AddRequestPacket() : CachePacket(PACKET_ADD_REQUEST, false) {}
    AddRequestPacket(const CacheItemKey& key, const CachedItemValue& value);

protected:
    bool Load(File *file) override;

public:
    CacheItemKey key;
    CachedItemValue value;
};


class AddResponsePacket : public CachePacket
{
public:
    AddResponsePacket() : CachePacket(PACKET_ADD_RESPONSE, false) {}
    AddResponsePacket(const CacheItemKey& key, bool added);

protected:
    bool Load(File *file) override;

public:
    CacheItemKey key;
    bool added = false;
};

class GetRequestPacket : public CachePacket
{
public:
    GetRequestPacket() : CachePacket(PACKET_GET_REQUEST, false) {}
    GetRequestPacket(const CacheItemKey& key);

protected:
    bool Load(File *file) override;

public:

    CacheItemKey key;
};

class GetResponsePacket : public CachePacket
{
public:
    GetResponsePacket() : CachePacket(PACKET_GET_RESPONSE, false) {}
    GetResponsePacket(const CacheItemKey& key, const CachedItemValue& value);

protected:
    bool Load(File *file) override;

public:
    CacheItemKey key;
    CachedItemValue value;
};

class WarmupRequestPacket : public CachePacket
{
public:
    WarmupRequestPacket() : CachePacket(PACKET_WARMING_UP_REQUEST, false) {}
    WarmupRequestPacket(const CacheItemKey& key);

protected:
    bool Load(File *file) override;

public:
    CacheItemKey key;
};

} // end of namespace AssetCache
} // end of namespace DAVA

#endif