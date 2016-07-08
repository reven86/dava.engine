#ifndef __DAVAENGINE_MMANOTHERSERVICE_H__
#define __DAVAENGINE_MMANOTHERSERVICE_H__

#include "Base/BaseTypes.h"
#include "Functional/Function.h"

#include "FileSystem/FilePath.h"

#include "Network/NetworkCommon.h"
#include "Network/Base/IPAddress.h"
#include "Network/NetService.h"

namespace DAVA
{
class Thread;
class File;
class FilePath;

namespace Net
{
class IOLoop;
class NetController;
class ServiceRegistrar;
struct IController;

class MMBigDataTransferService : public NetService
{
    struct SnapshotInfo
    {
        SnapshotInfo() = default;
        SnapshotInfo(const FilePath& fname)
            : filename(fname)
        {
        }
        SnapshotInfo(SnapshotInfo&& other)
            : filename(std::move(other.filename))
            , fileSize(other.fileSize)
            , bytesTransferred(other.bytesTransferred)
            , chunkSize(other.chunkSize)
        {
        }
        SnapshotInfo& operator=(SnapshotInfo&& other)
        {
            filename = std::move(other.filename);
            fileSize = other.fileSize;
            bytesTransferred = other.bytesTransferred;
            chunkSize = other.chunkSize;
            return *this;
        }

        FilePath filename;
        uint32 fileSize = 0;
        uint32 bytesTransferred = 0;
        uint32 chunkSize = 0;
    };

public:
    using SnapshotCallback = Function<void(uint32 totalSize, uint32 chunkOffset, uint32 chunkSize, const uint8* chunk)>;

public:
    MMBigDataTransferService(eNetworkRole role);
    virtual ~MMBigDataTransferService();

    void Start(bool newSession, uint32 connToken, const IPAddress& addr = IPAddress());
    void Stop();

    void TransferSnapshot(const FilePath& snapshotFile);
    void SetSnapshotCallback(SnapshotCallback callback);

    // Overriden methods from NetService
    void ChannelOpen() override;
    void ChannelClosed(const char8* message) override;
    void PacketReceived(const void* packet, size_t length) override;
    void PacketDelivered() override;

private:
    void ServerPacketDelivered();
    void DoTransferSnapshot(const FilePath& snapshotFile);
    void SendNextChunk(SnapshotInfo* snapshot);
    bool BeginNextSnapshot(SnapshotInfo* snapshot);

    void ClientPacketRecieved(const void* packet, size_t length);
    void ProcessAutoReplySnapshot(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength);

    void IOThread();
    void StartAnotherNetController();
    void OnNetControllerStopped(IController* controller);

    IChannelListener* NetServiceCreator(uint32 serviceId, void* context);
    void NetServiceDeleter(IChannelListener* service, void* context);

private:
    enum
    {
        SERVICE_ID = 0
    };
    enum
    {
        PORT = 55478
    };
    enum
    {
        OUTBUF_SIZE = 63 * 1024
    };

    eNetworkRole role;
    uint32 connToken = 0;
    std::unique_ptr<IOLoop> ioLoop;
    std::unique_ptr<ServiceRegistrar> registrar;
    std::unique_ptr<NetController> netController;
    RefPtr<Thread> ioThread;
    bool netServiceInUse = false;

    List<SnapshotInfo> snapshotQueue;

    File* fileHandle = nullptr;
    Array<uint8, OUTBUF_SIZE> outbuf;

    SnapshotCallback snapshotCallback;
};

} // namespace Net
} // namespace DAVA

#endif // __DAVAENGINE_MMANOTHERSERVICE_H__
