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

#include "AssetCache/CachePacket.h"
#include "AssetCache/TCPConnection/TCPChannel.h"

namespace DAVA {
namespace AssetCache {

const String PACKET_HEADER = "AssetCachePacket";
const uint32 PACKET_VERSION = 1;

CachePacket* CachePacket::Deserialize(const uint8* rawdata, size_t length)
{
    DynamicMemoryFile* dynamicBuffer = DynamicMemoryFile::Create(rawdata, length, File::OPEN | File::READ); // todo: create without copy
    File* buffer = dynamicBuffer;

    String header;
    if (buffer->ReadString(header) == false)
        return nullptr;

    if (header != PACKET_HEADER)
    {
        Logger::Error("[%s] Wrong packet header: %s", __FUNCTION__, header.c_str());
        return nullptr;
    }

    uint32 version = 0;
    if (buffer->Read(&version) != sizeof(version))
        return nullptr;

    if (version != PACKET_VERSION)
    {
        Logger::Error("[%s] Wrong packet version: %d", __FUNCTION__, version);
        return nullptr;
    }

    ePacketID type = PACKET_UNKNOWN;
    if (buffer->Read(&type) != sizeof(type))
        return nullptr;

    switch (type)
    {
    case PACKET_ADD_REQUEST:
    {
        Logger::FrameworkDebug("Deserializing ADD_REQUEST packet");
        AddRequestPacket* packet = new AddRequestPacket();

        if (buffer->Read(packet->key.data(), packet->key.size()) == packet->key.size() &&
            packet->value.Deserialize(buffer))
        {
            return packet;
        }
        else
        {
            Logger::Error("[%s] Wrong packet contents", __FUNCTION__);
            delete packet;
            return nullptr;
        }
    }
    case PACKET_ADD_RESPONSE:
    {
        Logger::FrameworkDebug("Deserializing ADD_RESPONSE packet");
        AddResponsePacket* packet = new AddResponsePacket();

        if (buffer->Read(packet->key.data(), packet->key.size()) == packet->key.size() &&
            buffer->Read(&packet->added) == sizeof(packet->added))
        {
            return packet;
        }
        else
        {
            Logger::Error("[%s] Wrong packet contents", __FUNCTION__);
            delete packet;
            return nullptr;
        }
    }
    case PACKET_GET_REQUEST:
    {
        Logger::FrameworkDebug("Deserializing GET_REQUEST packet");
        GetRequestPacket* packet = new GetRequestPacket();

        if (buffer->Read(packet->key.data(), packet->key.size()) == packet->key.size())
        {
            return packet;
        }
        else
        {
            Logger::Error("[%s] Wrong packet contents", __FUNCTION__);
            delete packet;
            return nullptr;
        }
    }
    case PACKET_GET_RESPONSE:
    {
        Logger::FrameworkDebug("Deserializing GET_RESPONSE packet");
        GetResponsePacket* packet = new GetResponsePacket();

        if (buffer->Read(packet->key.data(), packet->key.size()) == packet->key.size() &&
            packet->value.Deserialize(buffer))
        {
            return packet;
        }
        else
        {
            Logger::Error("[%s] Wrong packet contents", __FUNCTION__);
            delete packet;
            return nullptr;
        }
    }
    case PACKET_WARMING_UP_REQUEST:
    {
        Logger::FrameworkDebug("Deserializing WARMING_UP packet");
        WarmupRequestPacket* packet = new WarmupRequestPacket();

        if (buffer->Read(packet->key.data(), packet->key.size()) == packet->key.size())
        {
            return packet;
        }
        else
        {
            Logger::Error("[%s] Wrong packet contents", __FUNCTION__);
            delete packet;
            return nullptr;
        }
    }
    default:
    {
        Logger::Error("[%s] Wrong packet type: %d", __FUNCTION__, type);
        return nullptr;
    }
    }
}

bool CachePacket::SendTo(TCPChannel* channel)
{
    DVASSERT(channel);
    return channel->SendData(buffer);
}

AddRequestPacket::AddRequestPacket(const CacheItemKey& key, const CachedItemValue& value)
{
    Logger::FrameworkDebug("Constructing ADD_REQUEST packet");

    type = PACKET_ADD_REQUEST;
    buffer.reset(DynamicMemoryFile::Create(File::CREATE | File::WRITE));
    File* file = buffer;

    file->WriteString(PACKET_HEADER);
    file->Write(&PACKET_VERSION);
    file->Write(&type);
    file->Write(key.data(), key.size());
    value.Serialize(file);
}

AddResponsePacket::AddResponsePacket(const CacheItemKey& key, bool _added)
{
    Logger::FrameworkDebug("Constructing ADD_RESPONSE packet");

    type = PACKET_ADD_RESPONSE;
    added = _added;

    buffer.reset(DynamicMemoryFile::Create(File::CREATE | File::WRITE));
    File* file = buffer;

    file->WriteString(PACKET_HEADER);
    file->Write(&PACKET_VERSION);
    file->Write(&type);
    file->Write(key.data(), key.size());
}

GetRequestPacket::GetRequestPacket(const CacheItemKey& key)
{
    Logger::FrameworkDebug("Constructing GET_REQUEST packet");

    type = PACKET_GET_REQUEST;
    buffer.reset(DynamicMemoryFile::Create(File::CREATE | File::WRITE));
    File* file = buffer;

    file->WriteString(PACKET_HEADER);
    file->Write(&PACKET_VERSION);
    file->Write(&type);
    file->Write(key.data(), key.size());
}

GetResponsePacket::GetResponsePacket(const CacheItemKey& key, const CachedItemValue& value)
{
    Logger::FrameworkDebug("Constructing GET_RESPONSE packet");

    type = PACKET_GET_RESPONSE;
    buffer.reset(DynamicMemoryFile::Create(File::CREATE | File::WRITE));
    File* file = buffer;

    file->WriteString(PACKET_HEADER);
    file->Write(&PACKET_VERSION);
    file->Write(&type);
    file->Write(key.data(), key.size());
    value.Serialize(file);
}

WarmupRequestPacket::WarmupRequestPacket(const CacheItemKey& key)
{
    Logger::FrameworkDebug("Constructing WARM_UP packet");

    type = PACKET_WARMING_UP_REQUEST;
    buffer.reset(DynamicMemoryFile::Create(File::CREATE | File::WRITE));
    File* file = buffer;

    file->WriteString(PACKET_HEADER);
    file->Write(&PACKET_VERSION);
    file->Write(&type);
    file->Write(key.data(), key.size());
}

}}
