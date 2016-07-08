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
    using ConnEstablishedCallback = Function<void(bool, const MMStatConfig* config)>;
    using ConnLostCallback = Function<void(const char8* message)>;
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

    List<MMNetProto::Packet> packetQueue; // Queue of outgoing packets
    std::unique_ptr<MMBigDataTransferService> transferService; // Special service for downloading memory snapshots and other big data
};

} // namespace Net
} // namespace DAVA

#endif // __DAVAENGINE_MMNetClient_H__
