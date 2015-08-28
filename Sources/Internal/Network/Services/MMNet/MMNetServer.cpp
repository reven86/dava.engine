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

#include "Functional/Function.h"
#include "Debug/DVAssert.h"
#include "DLC/Patcher/ZLibStream.h"
#include "Platform/SystemTimer.h"
#include "Utils/Random.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/File.h"
#include "FileSystem/Logger.h"

#include "MemoryManager/MemoryManager.h"

#include "Network/Services/MMNet/MMNetServer.h"
#include "Network/Services/MMNet/MMBigDataTransferService.h"

namespace DAVA
{
namespace Net
{

MMNetServer::MMNetServer()
    : NetService()
    , connToken(Random::Instance()->Rand())
    , baseTimePoint(SystemTimer::Instance()->AbsoluteMS())
    , transferService(new MMBigDataTransferService(SERVER_ROLE))
{
    MemoryManager::Instance()->SetCallbacks(MakeFunction(this, &MMNetServer::OnUpdate),
                                            MakeFunction(this, &MMNetServer::OnTag));
}

MMNetServer::~MMNetServer() = default;

void MMNetServer::OnUpdate()
{
    if (tokenRequested)
    {
        uint64 curTimestamp = SystemTimer::Instance()->AbsoluteMS();
        if (curTimestamp - lastGatheredStatTimestamp >= statGatherFreq)
        {
            AutoReplyStat(curTimestamp - baseTimePoint);
            lastGatheredStatTimestamp = curTimestamp;
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

    const uint32 MAX_PACKET_SIZE = 1024 * 30;
    const uint32 FRAMES_PER_SECOND = 1000 / 16;
    maxStatItemsPerPacket = statGatherFreq > 0 ? statSendFreq / statGatherFreq + 1
                                               : statSendFreq / FRAMES_PER_SECOND + 1;
    maxStatItemsPerPacket = std::min(maxStatItemsPerPacket, MAX_PACKET_SIZE / statItemSize);
}

void MMNetServer::ChannelClosed(const char8* /*message*/)
{
    tokenRequested = false;
    lastGatheredStatTimestamp = 0;
    statSentStatTimestamp = 0;
    statItemsInPacket = 0;
    freePoolEntries = 0;
    totalPoolEntries = 0;
    for (MMNetProto::Packet& p : packetPool)
    {
        p = std::move(MMNetProto::Packet());
    }
    packetQueue.clear();
    transferService->Stop();
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
            break;
        }
    }
}

void MMNetServer::ProcessRequestToken(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength)
{
    bool newSession = inHeader->token != connToken;
    SendPacket(CreateReplyTokenPacket(newSession));
    transferService->Start(newSession, connToken);
}

void MMNetServer::ProcessRequestSnapshot(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength)
{
    DVASSERT(tokenRequested == true);
    uint64 curTimestamp = SystemTimer::Instance()->AbsoluteMS();
    uint16 status = GetAndSaveSnapshot(curTimestamp - baseTimePoint) ? MMNetProto::STATUS_SUCCESS
                                                                     : MMNetProto::STATUS_ERROR;
    SendPacket(CreateHeaderOnlyPacket(MMNetProto::TYPE_REPLY_SNAPSHOT, status));
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
        if (freePoolEntries < totalPoolEntries)
        {   // Return packet back to pool if it has at least one occupied entry
            packetPool[totalPoolEntries - freePoolEntries - 1] = std::move(packet);
            freePoolEntries += 1;
        }
        else if (totalPoolEntries < MAX_POOL_SIZE)
        {   // Or increase pool if its size is less than maximum permitted size
            packetPool[totalPoolEntries] = std::move(packet);
            totalPoolEntries += 1;
            freePoolEntries += 1;
        }
        // If no conditions above are true simply discard packet
    }

    if (!packetQueue.empty())
    {
        MMNetProto::Packet& x = packetQueue.front();
        Send(x.PlainBytes(), x.Header()->length);
    }
}

void MMNetServer::AutoReplyStat(uint64 curTimestamp)
{
    // Do not send anything if outgoing queue is greater some reasonable size
    if (packetQueue.size() > 256)
        return;

    if (0 == statSentStatTimestamp)
    {
        curStatPacket = ObtainStatPacket();
        statSentStatTimestamp = curTimestamp;
    }

    MemoryManager::Instance()->GetCurStat(curTimestamp, curStatPacket.Data<void>(statItemSize * statItemsInPacket), statItemSize);

    MMNetProto::PacketHeader* header = curStatPacket.Header();
    header->length += statItemSize;
    header->itemCount += 1;

    statItemsInPacket += 1;
    if (statItemsInPacket == maxStatItemsPerPacket || (curTimestamp - statSentStatTimestamp) >= statSendFreq)
    {
        statItemsInPacket = 0;
        statSentStatTimestamp = 0;
        SendPacket(std::forward<MMNetProto::Packet>(curStatPacket));
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

MMNetProto::Packet MMNetServer::ObtainStatPacket()
{
    MMNetProto::Packet packet;
    if (freePoolEntries > 0)
    {   // Pool has free entry, so get packet from entry with highest index
        packet = std::move(packetPool[totalPoolEntries - freePoolEntries]);
        freePoolEntries -= 1;
    }
    else
    {   // Pool has no free entries or empty, so allocate new packet
        packet = CreateReplyStatPacket(maxStatItemsPerPacket);
    }

    MMNetProto::PacketHeader* header = packet.Header();
    header->length = sizeof(MMNetProto::PacketHeader);
    header->type = MMNetProto::TYPE_AUTO_STAT;
    header->status = MMNetProto::STATUS_SUCCESS;
    header->itemCount = 0;
    header->token = connToken;
    return packet;
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
    static Atomic<uint32> curSnapshotIndex{0};

    bool result = false;
    FilePath filePath("~doc:");
    filePath += Format("msnap_%u.bin", curSnapshotIndex++);

    bool erase = false;
    {
        ScopedPtr<File> file(File::Create(filePath, File::CREATE | File::WRITE));
        if (file)
        {
            if (MemoryManager::Instance()->GetMemorySnapshot(curTimestamp, file.get(), nullptr))
            {
                file.reset(nullptr);
                transferService->TransferSnapshot(filePath);
                result = true;
            }
            erase = !result;
        }
    }
    if (erase)
    {   // Erase snapshot file if something went wrong
        FileSystem::Instance()->DeleteFile(filePath);
    }
    return result;
}

}   // namespace Net
}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)
