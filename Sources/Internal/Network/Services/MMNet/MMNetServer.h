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

#include "Concurrency/Spinlock.h"

#include "Network/NetService.h"
#include "Network/Services/MMNet/MMNetProto.h"

#include "MemoryManager/MemoryManagerTypes.h"

namespace DAVA
{
namespace Net
{

class MMBigDataTransferService;

class MMNetServer : public NetService
{
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
    void ProcessRequestSnapshot(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength);
    
    void AutoReplyStat(uint64 curTimestamp);
    void SendPacket(MMNetProto::Packet&& packet);

    MMNetProto::Packet ObtainStatPacket();
    MMNetProto::Packet CreateHeaderOnlyPacket(uint16 type, uint16 status);
    MMNetProto::Packet CreateReplyTokenPacket(bool newSession);
    MMNetProto::Packet CreateReplyStatPacket(uint32 maxItems);

    bool GetAndSaveSnapshot(uint64 curTimestamp);

private:
    bool tokenRequested = false;
    uint32 connToken = 0;
    uint64 baseTimePoint = 0;                       // Timestamp when server was started (it is approximate time when app was started)
    uint32 statGatherFreq = 100;                    // Memory statistics gathering frequency (zero means each frame), ms
    uint32 statSendFreq = 500;                      // Memory statistics sending frequency (zero means as soon as gathered), ms
    uint64 lastGatheredStatTimestamp = 0;           // Timestamp at which the latest memory statistics have been gathered
    uint64 statSentStatTimestamp = 0;               // Timestamp at which the latest memory statistics have been sent

    MMNetProto::Packet curStatPacket;               // Packet to collect memory stat to send it in one operation
    uint32 statItemsInPacket = 0;                   // Number of memory stat items already stored in curStatPacket
    uint32 maxStatItemsPerPacket = 0;               // Max number of stat items that curStatPacket can hold

    // Memory statistics are gathered at statGatherFreq frequency and placed into curStatPacket.
    // Memory statistics are sent over network when one of the following conditions is met:
    //  - difference between statSentStatTimestamp and current time exceeded statSendFreq, or
    //  - statItemsInPacket reached maxStatItemsPerPacket

    static const uint32 MAX_POOL_SIZE = 4;
    std::array<MMNetProto::Packet, MAX_POOL_SIZE> packetPool;   // Pool of packets for sending memory stat items to reduce memory allocations
                                                                // Each packet can hold MAX_STATITEMS_PER_PACKET stat items
    uint32 totalPoolEntries = 0;                                // Total number of entries in packet pool
    uint32 freePoolEntries = 0;                                 // Number of free entries in packet pool

    uint32 statItemSize = 0;                        // Size of MemoryManager's memory statistics item
    uint32 configSize = 0;                          // Size of MemoryManager's configuration block

    List<MMNetProto::Packet> packetQueue;                       // Queue of outgoing packets
    std::unique_ptr<MMBigDataTransferService> transferService;  // Special service for uploading memory snapshots and other big data
};

}   // namespace Net
}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)
#endif  // __DAVAENGINE_MMNETSERVER_H__
