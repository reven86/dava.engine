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

#include "Base/FunctionTraits.h"
#include "Debug/DVAssert.h"
#include "DLC/Patcher/ZLibStream.h"
#include "Platform/SystemTimer.h"
#include "Utils/Random.h"

#include "FileSystem/FilePath.h"
#include "FileSystem/Logger.h"

#include "MemoryManager/MemoryManager.h"

#include "Network/Services/MMNet/MMNetServer.h"
#include "Network/Services/MMNet/MMAnotherService.h"

namespace DAVA
{
namespace Net
{

MMNetServer::MMNetServer()
    : NetService()
    , connToken(Random::Instance()->Rand())
    , timerBegin(SystemTimer::Instance()->AbsoluteMS())
    , anotherService(new MMAnotherService(SERVER_ROLE))
{
    MemoryManager::Instance()->SetCallbacks(MakeFunction(this, &MMNetServer::OnUpdate),
                                            MakeFunction(this, &MMNetServer::OnTag));
}

MMNetServer::~MMNetServer()
{
}

void MMNetServer::OnUpdate()
{
    if (tokenRequested)
    {
        statPeriod = 0;
        uint64 curTimestamp = SystemTimer::Instance()->AbsoluteMS();
        if (curTimestamp - lastStatTimestamp >= statPeriod)
        {
            AutoReplyStat(curTimestamp - timerBegin);
            lastStatTimestamp = curTimestamp;
        }
    }
}

void MMNetServer::OnTag(uint32 tag, bool entering)
{
    // Here, we can automatically make memory snapshot
}
    
void MMNetServer::ChannelOpen()
{
    configSize = MemoryManager::Instance()->CalcStatConfigSize();
    statItemSize = MemoryManager::Instance()->CalcCurStatSize();
}

void MMNetServer::ChannelClosed(const char8* /*message*/)
{
    tokenRequested = false;
    anotherService->Stop();
    // do cleanup
}

void MMNetServer::PacketReceived(const void* packet, size_t length)
{
    const size_t dataLength = length - sizeof(MMNetProto::PacketHeader);
    const MMNetProto::PacketHeader* header = static_cast<const MMNetProto::PacketHeader*>(packet);
    if (length >= sizeof(MMNetProto::PacketHeader) && header->length == length)
    {
        switch (header->type)
        {
        case MMNetProto::TYPE_REQUEST_TOKEN:
            ProcessRequestToken(header, static_cast<const void*>(header + 1), dataLength);
            break;
        case MMNetProto::TYPE_REQUEST_SNAPSHOT:
            ProcessRequestSnapshot(header, static_cast<const void*>(header + 1), dataLength);
            break;
        default:
            DVASSERT(0);
            break;
        }
    }
}

void MMNetServer::ProcessRequestToken(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength)
{
    bool newSession = inHeader->token != connToken;
    SendPacket(CreateReplyTokenPacket(newSession));
    anotherService->Start(newSession, connToken);
}

void MMNetServer::ProcessRequestSnapshot(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength)
{
    DVASSERT(tokenRequested == true);
    if (tokenRequested)
    {
        uint16 status = MMNetProto::STATUS_BUSY;
        uint64 curTimestamp = SystemTimer::Instance()->AbsoluteMS();
        if (curTimestamp - lastManualSnapshotTimestamp >= 5000)
        {
            lastManualSnapshotTimestamp = curTimestamp;
            status = GetAndSaveSnapshot(curTimestamp - timerBegin) ? MMNetProto::STATUS_SUCCESS
                                                                   : MMNetProto::STATUS_ERROR;
        }
        SendPacket(CreateHeaderOnlyPacket(MMNetProto::TYPE_REPLY_SNAPSHOT, status));
    }
}

void MMNetServer::AutoReplyStat(uint64 curTimestamp)
{
    if (0 == statTimestamp)
    {
        if (freeCachedPackets > 0)
        {
            curStatPacket = std::move(cachedStatPackets[cachedStatPackets.size() - freeCachedPackets]);
            freeCachedPackets -= 1;
        }
        else
        {
            curStatPacket = CreateReplyStatPacket(maxStatItemsPerPacket);
        }

        MMNetProto::PacketHeader* header = curStatPacket.Header();
        header->length = sizeof(MMNetProto::PacketHeader);
        header->type = MMNetProto::TYPE_AUTO_STAT;
        header->status = MMNetProto::STATUS_SUCCESS;
        header->itemCount = 0;
        header->token = connToken;

        statTimestamp = curTimestamp;
    }

    MemoryManager::Instance()->GetCurStat(curStatPacket.Data<void>(statItemSize * statItemsInPacket), statItemSize);

    MMCurStat* stat = curStatPacket.Data<MMCurStat>(statItemSize * statItemsInPacket);
    stat->timestamp = curTimestamp;

    MMNetProto::PacketHeader* header = curStatPacket.Header();
    header->length += statItemSize;
    header->itemCount += 1;

    statItemsInPacket += 1;
    if (statItemsInPacket == maxStatItemsPerPacket || (curTimestamp - statTimestamp) >= 500)
    {
        statTimestamp = 0;
        statItemsInPacket = 0;
        SendPacket(std::forward<MMNetProto::Packet>(curStatPacket));
    }
}

void MMNetServer::PacketDelivered()
{
    DVASSERT(!packetQueue.empty());

    MMNetProto::Packet packet = std::move(packetQueue.front());
    packetQueue.pop_front();

    MMNetProto::PacketHeader* header = packet.Header();
    if (MMNetProto::TYPE_REPLY_TOKEN == header->type)
    {
        tokenRequested = true;
    }
    else if (MMNetProto::TYPE_AUTO_STAT == header->type)
    {
        if (freeCachedPackets < cachedStatPackets.size())
        {
            cachedStatPackets[cachedStatPackets.size() - freeCachedPackets - 1] = std::move(packet);
            freeCachedPackets += 1;
        }
        else if (cachedStatPackets.size() < maxCachedPackets)
        {
            cachedStatPackets.emplace_back(std::forward<MMNetProto::Packet>(packet));
            freeCachedPackets += 1;
        }
    }

    if (!packetQueue.empty())
    {
        MMNetProto::Packet& x = packetQueue.front();
        Send(x.PlainBytes(), x.Header()->length);
    }
}

void MMNetServer::SendPacket(MMNetProto::Packet&& packet)
{
    bool wasEmpty = packetQueue.empty();
    packetQueue.emplace_back(std::forward<MMNetProto::Packet>(packet));
    if (wasEmpty)
    {
        MMNetProto::Packet& x = packetQueue.front();
        Send(x.PlainBytes(), x.Header()->length);
    }
}

MMNetProto::Packet MMNetServer::CreateHeaderOnlyPacket(uint16 type, uint16 status)
{
    MMNetProto::Packet packet(0);

    MMNetProto::PacketHeader* header = packet.Header();
    header->length = sizeof(MMNetProto::PacketHeader);
    header->type = type;
    header->status = status;
    header->itemCount = 0;
    header->token = connToken;
    return packet;
}

MMNetProto::Packet MMNetServer::CreateReplyTokenPacket(bool newSession)
{
    uint32 dataSize = newSession ? configSize : 0;
    MMNetProto::Packet packet(dataSize);
    if (newSession)
    {
        MemoryManager::Instance()->GetStatConfig(packet.Data<void>(), configSize);
    }

    MMNetProto::PacketHeader* header = packet.Header();
    header->length = sizeof(MMNetProto::PacketHeader) + dataSize;
    header->type = MMNetProto::TYPE_REPLY_TOKEN;
    header->status = MMNetProto::STATUS_SUCCESS;
    header->itemCount = 0;
    header->token = connToken;
    return packet;
}

MMNetProto::Packet MMNetServer::CreateReplyStatPacket(uint32 maxItems)
{
    MMNetProto::Packet packet(statItemSize * maxItems);

    MMNetProto::PacketHeader* header = packet.Header();
    header->length = sizeof(MMNetProto::PacketHeader);
    header->type = MMNetProto::TYPE_AUTO_STAT;
    header->status = MMNetProto::STATUS_SUCCESS;
    header->itemCount = 0;
    header->token = connToken;
    return packet;
}

bool MMNetServer::GetAndSaveSnapshot(uint64 curTimestamp)
{
    bool result = false;
#if 0
    LockGuard<Spinlock> lock(snapshotMutex);

    FilePath filePath("~doc:");
    filePath += Format("msnap_%u.bin", curSnapshotIndex++);
    
    SnapshotInfo snapshotInfo(filePath.GetAbsolutePathname());

    FILE* file = fopen(snapshotInfo.filename.c_str(), "wb");
    if (file != nullptr)
    {
#if defined(__DAVAENGINE_WIN32__)
        // Dirty hack on Win32
        uint8 dummy[1] = {0};
        fwrite(dummy, 1, 1, file);
        fseek(file, -1, SEEK_CUR);
#endif
        if (MemoryManager::Instance()->GetMemorySnapshot(file, curTimestamp, &snapshotInfo.fileSize))
        {
            //readySnapshots.emplace_back(std::forward<SnapshotInfo>(snapshotInfo));
            anotherService->TransferSnapshot(filePath);
            result = true;
        }
        fclose(file);
    }
#endif
    return result;
}

}   // namespace Net
}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)
