#pragma once

#include <FileSystem/FilePath.h>
#include <Network/PeerDesription.h>
#include <MemoryManager/MemoryManagerTypes.h>

#include <QObject>
#include <QPointer>

namespace DAVA
{
class File;
namespace Net
{
struct IChannelListener;
class MMNetClient;
} // namespace Net
} // namespace DAVA

class MemProfWidget;
class ProfilingSession;

class MemProfController : public QObject
{
    Q_OBJECT

public:
    enum eMode
    {
        MODE_NETWORK = 0,
        MODE_FILE,
        MODE_SELFPROFILING
    };

public:
    MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget* parentWidget, QObject* parent = nullptr);
    MemProfController(const DAVA::FilePath& srcDir, QWidget* parentWidget, QObject* parent = nullptr);
    ~MemProfController();

    void ShowView();

    DAVA::Net::IChannelListener* NetObject() const;

    int Mode() const;
    bool IsFileLoaded() const;

    void NetConnEstablished(bool resumed, const DAVA::MMStatConfig* config);
    void NetConnLost(const DAVA::char8* message);
    void NetStatRecieved(const DAVA::MMCurStat* stat, DAVA::uint32 count);
    void NetSnapshotRecieved(DAVA::uint32 totalSize, DAVA::uint32 chunkOffset, DAVA::uint32 chunkSize, const DAVA::uint8* chunk);

signals:
    void ConnectionEstablished(bool newConnection);
    void ConnectionLost(const DAVA::char8* message);
    void StatArrived(DAVA::uint32 itemCount);
    void SnapshotProgress(DAVA::uint32 totalSize, DAVA::uint32 recvSize);
    void SnapshotSaved(const DAVA::FilePath* filePath);

public slots:
    void OnSnapshotPressed();

private slots:
    void OnSnapshotSaved(const DAVA::FilePath* filePath);

private:
    void ComposeFilePath(DAVA::FilePath& result);

private:
    int mode;
    QPointer<MemProfWidget> view;
    QPointer<QWidget> parentWidget;

    bool snapshotInProgress = false;
    DAVA::FilePath snapshotTempName;
    DAVA::File* snapshotFile = nullptr;
    DAVA::uint32 snapshotTotalSize = 0;
    DAVA::uint32 snapshotRecvSize = 0;

    DAVA::Net::PeerDescription profiledPeer;
    std::unique_ptr<DAVA::Net::MMNetClient> netClient;
    std::unique_ptr<ProfilingSession> profilingSession;
};

//////////////////////////////////////////////////////////////////////////
inline int MemProfController::Mode() const
{
    return mode;
}
