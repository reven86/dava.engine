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
#include "Thread/LockGuard.h"
#include "Debug/DVAssert.h"
#include "DLC/Patcher/ZLibStream.h"
#include "Platform/SystemTimer.h"
#include "Utils/Random.h"

#include "FileSystem/FilePath.h"
#include "FileSystem/Logger.h"

#include "MemoryManager/MemoryManager.h"

#include "MMNetServer.h"

#include <cstdlib>

namespace DAVA
{
namespace Net
{

MMNetServer::MMNetServer()
    : NetService()
    , timerBegin(0)
{
    connToken = Random::Instance()->Rand();
    timerBegin = SystemTimer::Instance()->AbsoluteMS();

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
    statItemSize = MemoryManager::Instance()->CalcCurStatSize();
}

void MMNetServer::ChannelClosed(const char8* /*message*/)
{
    tokenRequested = false;
    Cleanup();
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
    if (inHeader->token != connToken)
    {
        size_t configSize = MemoryManager::Instance()->CalcStatConfigSize();
        ParcelEx parcel(configSize);
        MemoryManager::Instance()->GetStatConfig(parcel.data, configSize);
        
        parcel.header->length = uint32(sizeof(MMNetProto::PacketHeader) + configSize);
        parcel.header->type = MMNetProto::TYPE_REPLY_TOKEN;
        parcel.header->status = MMNetProto::STATUS_SUCCESS;
        parcel.header->itemCount = 0;
        parcel.header->token = connToken;
        
        EnqueueParcel(parcel);
    }
    else
    {
        FastReply(MMNetProto::TYPE_REPLY_TOKEN, MMNetProto::STATUS_SUCCESS);
    }
}

void MMNetServer::ProcessRequestSnapshot(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength)
{
    uint16 status = MMNetProto::STATUS_TOKEN;
    if (tokenRequested)
    {
        status = MMNetProto::STATUS_BUSY;
        uint64 curTimestamp = SystemTimer::Instance()->AbsoluteMS();
        if (curTimestamp - lastManualSnapshotTimestamp >= 5000)
        {
            lastManualSnapshotTimestamp = curTimestamp;
            status = GetAndSaveSnapshot(curTimestamp - timerBegin) ? MMNetProto::STATUS_SUCCESS
                                                               : MMNetProto::STATUS_BUSY;
        }
    }
    FastReply(MMNetProto::TYPE_REPLY_SNAPSHOT, status);
}

void MMNetServer::AutoReplyStat(uint64 curTimestamp)
{
    ParcelEx parcel(statItemSize);
    MemoryManager::Instance()->GetCurStat(parcel.data, statItemSize);

    MMCurStat* stat = static_cast<MMCurStat*>(parcel.data);
    stat->timestamp = curTimestamp;

    parcel.header->length = uint32(sizeof(MMNetProto::PacketHeader) + statItemSize);
    parcel.header->type = MMNetProto::TYPE_AUTO_STAT;
    parcel.header->status = MMNetProto::STATUS_SUCCESS;
    parcel.header->itemCount = 1;
    parcel.header->token = connToken;

    EnqueueParcel(parcel);
}

void MMNetServer::FastReply(uint16 type, uint16 status)
{
    ParcelEx parcel(0);
    parcel.header->length = uint32(sizeof(MMNetProto::PacketHeader));
    parcel.header->type = type;
    parcel.header->status = status;
    parcel.header->itemCount = 0;
    parcel.header->token = connToken;
    
    EnqueueParcel(parcel);
}

void MMNetServer::PacketDelivered()
{
    DVASSERT(!queue.empty());

    ParcelEx parcel = queue.front();
    queue.pop_front();
    
    if (parcel.header->type == MMNetProto::TYPE_REPLY_TOKEN)
    {
        tokenRequested = true;
        ::operator delete(parcel.buffer);
    }
    else if (parcel.header->type == MMNetProto::TYPE_REPLY_SNAPSHOT)
    {
        ::operator delete(parcel.buffer);
    }
    else if (parcel.header->type == MMNetProto::TYPE_AUTO_STAT)
    {
        ::operator delete(parcel.buffer);
    }
    else if (parcel.header->type == MMNetProto::TYPE_AUTO_SNAPSHOT)
    {
        UpdateSnapshotProgress(parcel);
    }

    if (!queue.empty())
    {
        SendParcel(queue.front());
    }
    if (!waitSnapshotAck)
    {
        CheckAndTransferSnapshot();
    }
}

void MMNetServer::EnqueueParcel(const ParcelEx& parcel)
{
    bool wasEmpty = queue.empty();
    queue.push_back(parcel);
    if (wasEmpty)
    {
        SendParcel(queue.front());
    }
}

void MMNetServer::SendParcel(ParcelEx& parcel)
{
    Send(parcel.buffer, parcel.header->length);
}

void MMNetServer::Cleanup()
{
    for (auto& parcel : queue)
    {
        if (parcel.header->type != MMNetProto::TYPE_AUTO_SNAPSHOT)
        {
            ::operator delete(parcel.buffer);
        }
    }
    queue.clear();
    CleanupSnapshot(false);
}

void MMNetServer::CleanupSnapshot(bool erase)
{
    if (snapshotFileHandle != nullptr)
    {
        fclose(snapshotFileHandle);
        snapshotFileHandle = nullptr;
        
        if (erase)
        {
            remove(curSnapshotInfo->filename.c_str());
            curSnapshotInfo = nullptr;
            {
                LockGuard<Spinlock> lock(snapshotMutex);
                readySnapshots.pop_front();
            }
        }
    }
}

void MMNetServer::UpdateSnapshotProgress(const ParcelEx& parcel)
{
    bool transferDone = true;
    if (parcel.header->status == MMNetProto::STATUS_SUCCESS)
    {
        const MMNetProto::PacketParamSnapshot* param = static_cast<const MMNetProto::PacketParamSnapshot*>(snapshotParcel.data);
        curSnapshotInfo->bytesTransferred += param->chunkSize;
        
        transferDone = curSnapshotInfo->bytesTransferred == curSnapshotInfo->fileSize;
    }
    
    if (transferDone)
    {
        CleanupSnapshot(true);
    }
    waitSnapshotAck = false;
}

void MMNetServer::CheckAndTransferSnapshot()
{
    if (snapshotFileHandle != nullptr)
    {
        ContinueSnapshotTransfer();
    }
    else
    {
        BeginNextSnapshotTransfer();
    }
}

void MMNetServer::BeginNextSnapshotTransfer()
{
    {
        LockGuard<Spinlock> lock(snapshotMutex);
        if (!readySnapshots.empty())
        {
            curSnapshotInfo = &readySnapshots.front();
        }
    }
    if (curSnapshotInfo != nullptr)
    {
        snapshotFileHandle = fopen(curSnapshotInfo->filename.c_str(), "rb");
        if (snapshotFileHandle != nullptr)
        {
            if (nullptr == snapshotParcel.buffer)
            {
                snapshotParcel = ParcelEx(sizeof(MMNetProto::PacketParamSnapshot) + SNAPSHOT_CHUNK_SIZE);
            }
            snapshotParcel.header->length = 0;
            snapshotParcel.header->type = MMNetProto::TYPE_AUTO_SNAPSHOT;
            snapshotParcel.header->status = MMNetProto::STATUS_SUCCESS;
            snapshotParcel.header->itemCount = 0;
            snapshotParcel.header->token = connToken;

            ContinueSnapshotTransfer();
        }
        else
        {
            remove(curSnapshotInfo->filename.c_str());
            curSnapshotInfo = nullptr;
            {
                LockGuard<Spinlock> lock(snapshotMutex);
                readySnapshots.pop_front();
            }
        }
    }
}

void MMNetServer::ContinueSnapshotTransfer()
{
    MMNetProto::PacketParamSnapshot* param = static_cast<MMNetProto::PacketParamSnapshot*>(snapshotParcel.data);
    void* readBuf = static_cast<void*>(param + 1);
    
    size_t chunkSize = Min(curSnapshotInfo->fileSize - curSnapshotInfo->bytesTransferred, SNAPSHOT_CHUNK_SIZE);
    size_t nread = fread(readBuf, 1, chunkSize, snapshotFileHandle);
    if (nread == chunkSize)
    {
        param->flags = 0;
        param->snapshotSize = static_cast<uint32>(curSnapshotInfo->fileSize);
        param->chunkSize = static_cast<uint32>(chunkSize);
        param->chunkOffset = static_cast<uint32>(curSnapshotInfo->bytesTransferred);
        
        snapshotParcel.header->length = uint32(sizeof(MMNetProto::PacketHeader) + sizeof(MMNetProto::PacketParamSnapshot) + chunkSize);
        EnqueueParcel(snapshotParcel);
    }
    else
    {
        FastReply(MMNetProto::TYPE_AUTO_SNAPSHOT, MMNetProto::STATUS_ERROR);
    }
    waitSnapshotAck = true;
}

bool MMNetServer::GetAndSaveSnapshot(uint64 curTimestamp)
{
    bool result = false;
    
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
            readySnapshots.emplace_back(std::forward<SnapshotInfo>(snapshotInfo));
            result = true;
        }
        fclose(file);
    }
    return result;
}

}   // namespace Net
}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)
