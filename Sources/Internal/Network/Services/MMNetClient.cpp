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

#include "Base/Function.h"

#include "Debug/DVAssert.h"

#include "FileSystem/DynamicMemoryFile.h"
#include "DLC/Patcher/ZLibStream.h"
#include "MemoryManager/MemoryManager.h"

#include "MMNetClient.h"

namespace DAVA
{
namespace Net
{

MMNetClient::MMNetClient()
    : NetService()
    , sessionId(0)
    , commInited(false)
    , outbufBusy(false)
    , gettingDump(false)
    , dumpTotalSize(0)
    , dumpRecvSize(0)
{

}

MMNetClient::~MMNetClient()
{

}

void MMNetClient::SetCallbacks(ChOpenCallback onOpen, ChClosedCallback onClosed, StatCallback onStat, DumpCallback onDump)
{
    openCallback = onOpen;
    closeCallback = onClosed;
    statCallback = onStat;
    dumpCallback = onDump;
}

void MMNetClient::RequestDump()
{
    if (commInited && !outbufBusy && !gettingDump)
    {
        SendTypeDump();
    }
}

void MMNetClient::ChannelOpen()
{
    SendTypeInit();
}

void MMNetClient::ChannelClosed(const char8* message)
{
    commInited = false;
    outbufBusy = false;
    gettingDump = false;

    closeCallback(message);
}

void MMNetClient::PacketReceived(const void* packet, size_t length)
{
    DVASSERT(length >= sizeof(MMNetProto::Header));

    const MMNetProto::Header* header = static_cast<const MMNetProto::Header*>(packet);
    const void* packetData = OffsetPointer<void>(packet, sizeof(MMNetProto::Header));
    size_t packetDataSize = length - sizeof(MMNetProto::Header);

    DVASSERT(packetDataSize == header->length);
    switch (header->type)
    {
    case MMNetProto::TYPE_INIT:
        ProcessTypeInit(reinterpret_cast<const MMNetProto::HeaderInit*>(header), packetData, packetDataSize);
        break;
    case MMNetProto::TYPE_STAT:
        ProcessTypeStat(reinterpret_cast<const MMNetProto::HeaderStat*>(header), packetData, packetDataSize);
        break;
    case MMNetProto::TYPE_DUMP:
        ProcessTypeDump(reinterpret_cast<const MMNetProto::HeaderDump*>(header), packetData, packetDataSize);
        break;
    default:
        break;
    }
}

void MMNetClient::PacketDelivered()
{
    outbufBusy = false;
}

void MMNetClient::ProcessTypeInit(const MMNetProto::HeaderInit* header, const void* packetData, size_t dataLength)
{
    DVASSERT(header->status == MMNetProto::STATUS_OK);
    const MMStatConfig* config = nullptr;
    if (header->totalLength > 0)
    {
        sessionId = header->sessionId;
        config = static_cast<const MMStatConfig*>(packetData);
    }
    commInited = true;
    openCallback(config);
}

void MMNetClient::ProcessTypeStat(const MMNetProto::HeaderStat* header, const void* packetData, size_t dataLength)
{
    DVASSERT(header->status == MMNetProto::STATUS_OK);
    const MMCurStat* stat = static_cast<const MMCurStat*>(packetData);
    statCallback(stat);
}

void MMNetClient::ProcessTypeDump(const MMNetProto::HeaderDump* header, const void* packetData, size_t dataLength)
{
    DVASSERT(header->status == MMNetProto::STATUS_OK);
    if (!gettingDump)
    {
        dumpTotalSize = header->totalLength;
        dumpRecvSize = 0;
        dumpData.resize(dumpTotalSize);
        gettingDump = true;
    }
    Memcpy(&*dumpData.begin() + dumpRecvSize, packetData, dataLength);
    dumpRecvSize += dataLength;

    DVASSERT(dumpRecvSize <= dumpTotalSize);
    if (dumpRecvSize < dumpTotalSize)
    {
        dumpCallback(dumpTotalSize, dumpRecvSize, nullptr);
    }
    else
    {
        dumpCallback(dumpTotalSize, dumpRecvSize, &dumpData);
        gettingDump = false;
    }
}

/*void MMNetClient::UnpackDump()
{
    Vector<uint8> v(unpackedDumpSize, 0);
    DynamicMemoryFile* zipFile = DynamicMemoryFile::Create(dumpV.data() + sizeof(MMDump), uint32(dumpV.size() - sizeof(MMDump)), File::CREATE | File::READ);
    ZLibIStream zipStream(zipFile);
    zipStream.Read((char8*)(v.data() + sizeof(MMDump)), uint32(unpackedDumpSize - sizeof(MMDump)));
    Memcpy(v.data(), dumpV.data(), sizeof(MMDump));

    MMDump* dump = reinterpret_cast<MMDump*>(v.data());
    dumpDoneCallback(dump, dumpSize, v);
    gettingDump = false;
}*/

void MMNetClient::SendTypeInit()
{
    MMNetProto::HeaderInit* header = reinterpret_cast<MMNetProto::HeaderInit*>(outbuf);
    header->type = MMNetProto::TYPE_INIT;
    header->status = MMNetProto::STATUS_OK;
    header->length = 0;
    header->totalLength = 0;
    header->sessionId = sessionId;

    outbufBusy = true;
    Send(header);
}

void MMNetClient::SendTypeDump()
{
    MMNetProto::HeaderDump* header = reinterpret_cast<MMNetProto::HeaderDump*>(outbuf);
    header->type = MMNetProto::TYPE_DUMP;
    header->status = MMNetProto::STATUS_OK;
    header->length = 0;
    header->totalLength = 0;
    header->isPacked = 0;

    outbufBusy = true;
    Send(header);
}

}   // namespace Net
}   // namespace DAVA
