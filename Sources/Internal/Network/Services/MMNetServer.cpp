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

#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include "Debug/DVAssert.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "DLC/Patcher/ZLibStream.h"
#include "Platform/SystemTimer.h"
#include "Utils/Random.h"

#include "MemoryManager/MemoryManager.h"

#include "MMNetServer.h"

#include <cstdlib>
#include <ctime>

namespace DAVA
{
namespace Net
{

MMNetServer::MMNetServer()
    : NetService()
    , sessionId(0)
    , commInited(false)
    , timerBegin(0)
    , statPeriod(250)
    , periodCounter(0)
    , outHeader(reinterpret_cast<MMNetProto::Header*>(outbuf))
    , outData(OffsetPointer<void>(outbuf, sizeof(MMNetProto::Header)))
    , zipFile(nullptr)
{
    sessionId = Random::Instance()->Rand();
    timerBegin = SystemTimer::Instance()->AbsoluteMS();
}

MMNetServer::~MMNetServer()
{

}

void MMNetServer::Update(float32 timeElapsed)
{
    if (!commInited) return;

    periodCounter += static_cast<size_t>(timeElapsed * 1000.0f);
    if (periodCounter >= statPeriod)
    {
        SendMemoryStat();
        periodCounter = 0;
    }
}

void MMNetServer::ChannelOpen()
{

}

void MMNetServer::ChannelClosed(const char8* message)
{
    commInited = false;

    for (auto& x : queue)
        DeleteParcelData(x);
    queue.clear();
}

void MMNetServer::PacketReceived(const void* packet, size_t length)
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
    case MMNetProto::TYPE_DUMP:
        ProcessTypeDump(reinterpret_cast<const MMNetProto::HeaderDump*>(header), packetData, packetDataSize);
        break;
    default:
        break;
    }
}

void MMNetServer::PacketDelivered()
{
    DVASSERT(!queue.empty());

    Parcel& parcel = queue.front();
    parcel.dataSent += parcel.chunkSize;
    DVASSERT(parcel.dataSent <= parcel.dataSize);
    if (parcel.dataSent < parcel.dataSize)
    {
        SendParcel(parcel);
    }
    else
    {
        DeleteParcelData(parcel);
        if (MMNetProto::TYPE_INIT == parcel.header.type && MMNetProto::STATUS_OK == parcel.header.status)
        {
            commInited = true;
        }
        queue.pop_front();
        if (!queue.empty())
        {
            SendParcel(queue.front());
        }
    }
}

void MMNetServer::ProcessTypeInit(const MMNetProto::HeaderInit* header, const void* packetData, size_t dataLength)
{
    DVASSERT(false == commInited);

    uint32 status = MMNetProto::STATUS_OK;

    uint32 configSize = 0;
    MMStatConfig* config = nullptr;
    if (header->sessionId != sessionId)
    {
        config = MemoryManager::Instance()->GetStatConfig();
        if (nullptr == config)
            status = MMNetProto::STATUS_ERROR;
        else
            configSize = config->size;
    }

    Parcel parcel(config, configSize);
    MMNetProto::HeaderInit* outHeader = reinterpret_cast<MMNetProto::HeaderInit*>(&parcel.header);
    outHeader->type = MMNetProto::TYPE_INIT;
    outHeader->status = status;
    outHeader->length = 0;
    outHeader->totalLength = configSize;
    outHeader->sessionId = sessionId;

    EnqueueParcel(parcel);
}

void MMNetServer::ProcessTypeDump(const MMNetProto::HeaderDump* header, const void* packetData, size_t dataLength)
{
    DVASSERT(true == commInited);
    GatherDump();
}

void MMNetServer::SendMemoryStat()
{
    uint64 timeBeginStat = SystemTimer::Instance()->AbsoluteMS();
    MMCurStat* stat = MemoryManager::Instance()->GetCurStat();
    if (stat != nullptr)
    {
        Parcel parcel(stat, stat->size);
        MMNetProto::HeaderStat* header = reinterpret_cast<MMNetProto::HeaderStat*>(&parcel.header);
        header->type = MMNetProto::TYPE_STAT;
        header->status = MMNetProto::STATUS_OK;
        header->length = 0;
        header->totalLength = stat->size;

        stat->timestamp = timeBeginStat - timerBegin;
        EnqueueParcel(parcel);
    }
    else
    {
        Parcel parcel;
        MMNetProto::HeaderStat* header = reinterpret_cast<MMNetProto::HeaderStat*>(&parcel.header);
        header->type = MMNetProto::TYPE_STAT;
        header->status = MMNetProto::STATUS_ERROR;
        header->length = 0;
        header->totalLength = 0;

        EnqueueParcel(parcel);
    }
}

void MMNetServer::DeleteParcelData(Parcel& parcel)
{
    switch (parcel.header.type)
    {
    case MMNetProto::TYPE_INIT:
    case MMNetProto::TYPE_STAT:
    case MMNetProto::TYPE_DUMP:
        if (parcel.data != nullptr)
        {
            MemoryManager::Instance()->FreeStatMemory(parcel.data);
            parcel.data = nullptr;
        }
        break;
    default:
        break;
    }
}

void MMNetServer::EnqueueParcel(const Parcel& parcel)
{
    bool wasEmpty = queue.empty();
    queue.push_back(parcel);
    if (wasEmpty)
    {
        SendParcel(queue.front());
    }
}

void MMNetServer::SendParcel(Parcel& parcel)
{
    if (0 == parcel.dataSent)
        *outHeader = parcel.header;
    parcel.chunkSize = std::min(parcel.dataSize - parcel.dataSent, OUTBUF_USEFUL_SIZE);
    outHeader->length = parcel.chunkSize;
    if (parcel.chunkSize > 0)
        Memcpy(outData, OffsetPointer<void>(parcel.data, parcel.dataSent), parcel.chunkSize);
    Send(outbuf, parcel.chunkSize + sizeof(MMNetProto::Header));
}

void MMNetServer::GatherDump()
{
    uint64 timeBeginDump = SystemTimer::Instance()->AbsoluteMS();
    MMDump* dump = MemoryManager::Instance()->GetMemoryDump();
    uint64 timeEndDump = SystemTimer::Instance()->AbsoluteMS();

    if (dump != nullptr)
    {
        Parcel parcel(dump, dump->size);
        MMNetProto::HeaderDump* header = reinterpret_cast<MMNetProto::HeaderDump*>(&parcel.header);
        header->type = MMNetProto::TYPE_DUMP;
        header->status = MMNetProto::STATUS_OK;
        header->length = 0;
        header->totalLength = dump->size;
        header->isPacked = 0;

        dump->timestamp = timeBeginDump - timerBegin;
        dump->collectTime = uint16((timeEndDump - timeBeginDump) / 10);
        MMCurStat* curStat = OffsetPointer<MMCurStat>(dump, sizeof(MMDump));
        curStat->timestamp = timeBeginDump - timerBegin;

        EnqueueParcel(parcel);
    }
    else
    {
        Parcel parcel;
        MMNetProto::HeaderDump* header = reinterpret_cast<MMNetProto::HeaderDump*>(&parcel.header);
        header->type = MMNetProto::TYPE_DUMP;
        header->status = MMNetProto::STATUS_ERROR;
        header->length = 0;
        header->totalLength = 0;
        header->isPacked = 0;

        EnqueueParcel(parcel);
    }
}
/*
void MMNetServer::OnDumpRequest(int32 type, uint32 tagOrCheckpoint, uint32 blockBegin, uint32 blockEnd)
{
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    if (!commInited) return;
    if (zipFile) return;

    // TODO: ZLibOStream needs update to use not only files but memory buffer
    void* buf = nullptr;
    uint64 timerStart = SystemTimer::Instance()->AbsoluteMS();

    size_t dumpSize = MemoryManager::Instance()->GetDump(0, &buf, blockBegin, blockEnd);
    uint64 timePoint2 = SystemTimer::Instance()->AbsoluteMS();

    MMDump temp = {0};
    Memcpy(&temp, buf, sizeof(MMDump));
    zipFile = DynamicMemoryFile::Create(File::CREATE | File::READ | File::WRITE);
    zipFile->Seek(sizeof(MMProtoHeader) + sizeof(MMDump), File::SEEK_FROM_START);
    {
        ZLibOStream zipStream(zipFile);
        zipStream.Write(static_cast<char8*>(buf)+sizeof(MMDump), static_cast<uint32>(dumpSize));
    }
    MemoryManager::Instance()->FreeDump(buf);
    uint64 timePoint3 = SystemTimer::Instance()->AbsoluteMS();

    Parcel parcel = CreateParcel(zipFile->GetSize(), zipFile->GetData());

    MMProtoHeader* outHdr = static_cast<MMProtoHeader*>(parcel.buffer);
    MMDump* dump = reinterpret_cast<MMDump*>(outHdr + 1);
    *dump = temp;
    dump->timestampBegin = timerStart;
    dump->collectTime = timePoint3 - timerStart;
    dump->zipTime = timePoint3 - timePoint2;

    outHdr->sessionId = sessionId;
    outHdr->cmd = static_cast<uint32>(eMMProtoCmd::DUMP);
    outHdr->status = static_cast<uint32>(dumpSize);
    outHdr->length = static_cast<uint32>(parcel.size - sizeof(MMProtoHeader));

    EnqueueAndSend(parcel);
#endif
}
*/
}   // namespace Net
}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)
