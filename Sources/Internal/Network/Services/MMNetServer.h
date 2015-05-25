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

#ifndef __DAVAENGINE_MMNETSERVER_H__
#define __DAVAENGINE_MMNETSERVER_H__

#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include "Thread/Spinlock.h"

#include "Network/NetService.h"
#include "MemoryManager/MemoryManagerTypes.h"
#include "MMNetProto.h"

namespace DAVA
{

namespace Net
{

class MMNetServer : public NetService
{
    struct ParcelEx
    {
        ParcelEx()
            : bufferSize(0)
            , buffer(nullptr)
            , header(nullptr)
            , data(nullptr)
        {}
        ParcelEx(size_t dataSize)
            : bufferSize(sizeof(MMNetProto::PacketHeader) + dataSize)
            , buffer(::operator new(bufferSize))
            , header(static_cast<MMNetProto::PacketHeader*>(buffer))
            , data(static_cast<void*>(header + 1))
        {}

        size_t bufferSize;
        void* buffer;
        MMNetProto::PacketHeader* header;
        void* data;
    };
    
    struct DumpInfo
    {
        DumpInfo() {}
        DumpInfo(const String& fname)
            : filename(fname)
        {}
        DumpInfo(DumpInfo&& other)
            : filename(std::move(other.filename))
            , fileSize(other.fileSize)
            , bytesTransferred(other.bytesTransferred)
        {}
        DumpInfo& operator = (DumpInfo&& other)
        {
            filename = std::move(other.filename);
            fileSize = other.fileSize;
            bytesTransferred = other.bytesTransferred;
            return *this;
        }
        
        String filename;
        size_t fileSize = 0;
        size_t bytesTransferred = 0;
    };
    
    static const size_t DUMPCHUNK_SIZE = 63 * 1024;

public:
    MMNetServer();
    virtual ~MMNetServer();

    // Overriden methods from NetService
    void ChannelOpen() override;
    void ChannelClosed(const char8* message) override;
    void PacketReceived(const void* packet, size_t length) override;
    void PacketDelivered() override;
    
private:
    void OnUpdate();
    void OnTag(uint32 tag, bool entering);
    
    void ProcessRequestToken(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength);
    void ProcessRequestDump(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength);
    
    void AutoReplyStat(uint64 curTimestamp);
    void FastReply(uint16 type, uint16 status);

    void EnqueueParcel(const ParcelEx& parcel);
    void SendParcel(ParcelEx& parcel);
    
    void Cleanup();
    void CleanupDump(bool erase);

    void UpdateDumpProgress();
    void CheckAndTransferDump();
    void ContinueDumpTransfer();
    void BeginNextDumpTransfer();
    bool GetAndSaveDump(uint64 curTimestamp);

private:
    uint32 connToken = 0;
    bool tokenRequested = false;
    uint64 timerBegin;
    uint64 lastStatTimestamp = 0;
    uint64 statPeriod = 100;

    List<ParcelEx> queue;
    
    size_t statItemSize = 0;
    
    Spinlock dumpMutex;
    List<DumpInfo> readyDumps;
    ParcelEx dumpParcel;
    DumpInfo* curDumpInfo = nullptr;
    FILE* dumpFileHandle = nullptr;
    
    uint32 curDumpIndex = 0;
    uint64 lastManualDumpTimestamp = 0;
};

}   // namespace Net
}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)

#endif  // __DAVAENGINE_MMNETSERVER_H__
