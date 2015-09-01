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

#ifndef __DAVAENGINE_MMNetClient_H__
#define __DAVAENGINE_MMNetClient_H__

#include "Base/BaseTypes.h"
#include "Functional/Function.h"

#include "Network/NetService.h"
#include "Network/Services/MMNet/MMNetProto.h"

#include "MemoryManager/MemoryManagerTypes.h"

namespace DAVA
{
namespace Net
{

class MMBigDataTransferService;

class MMNetClient : public NetService
{
public:
    using ConnEstablishedCallback = Function<void (bool, const MMStatConfig* config)>;
    using ConnLostCallback = Function<void (const char8* message)>;
    using StatCallback = Function<void(const MMCurStat* stat, uint32 itemCount)>;
    using SnapshotCallback = Function<void(uint32 totalSize, uint32 chunkOffset, uint32 chunkSize, const uint8* chunk)>;

public:
    MMNetClient();
    virtual ~MMNetClient();

    void InstallCallbacks(ConnEstablishedCallback connEstablishedCallback, ConnLostCallback connLostCallback, StatCallback statCallback, SnapshotCallback snapshotCallback);

    void RequestSnapshot();

    // Overriden methods from NetService
    void ChannelOpen() override;
    void ChannelClosed(const char8* message) override;
    void PacketReceived(const void* packet, size_t length) override;
    void PacketDelivered() override;

private:
    void ProcessReplyToken(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength);
    void ProcessReplySnapshot(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength);
    void ProcessAutoReplyStat(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength);

    void SendPacket(MMNetProto::Packet&& packet);

    MMNetProto::Packet CreateHeaderOnlyPacket(uint16 type, uint16 status);

private:
    bool tokenRequested = false;
    uint32 connToken = 0;
    
    bool canRequestSnapshot = false;

    ConnEstablishedCallback connEstablishedCallback;
    ConnLostCallback connLostCallback;
    StatCallback statCallback;

    List<MMNetProto::Packet> packetQueue;                       // Queue of outgoing packets
    std::unique_ptr<MMBigDataTransferService> transferService;  // Special service for downloading memory snapshots and other big data
};

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_MMNetClient_H__
