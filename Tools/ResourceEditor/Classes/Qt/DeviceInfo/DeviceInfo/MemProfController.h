#ifndef __MEMPROFCONTROLLER_H__
#define __MEMPROFCONTROLLER_H__

#include <QObject>
#include <QPointer>

#include "Network/PeerDesription.h"
#include "MemoryManager/MemoryManagerTypes.h"

namespace DAVA {
class FilePath;
namespace Net {
struct IChannelListener;
class MMNetClient;
}}

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
    MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget *parentWidget, QObject *parent = nullptr);
    MemProfController(const DAVA::FilePath& srcDir, QWidget *parentWidget, QObject *parent = nullptr);
    ~MemProfController();

    void ShowView();

    DAVA::Net::IChannelListener* NetObject() const;

    int Mode() const;
    bool IsFileLoaded() const;

    void NetConnEstablished(bool resumed, const DAVA::MMStatConfig* config);
    void NetConnLost(const DAVA::char8* message);
    void NetStatRecieved(const DAVA::MMCurStat* stat, size_t count);
    void NetSnapshotRecieved(int stage, size_t totalSize, size_t recvSize, const void* data);

signals:
    void ConnectionEstablished(bool newConnection);
    void ConnectionLost(const DAVA::char8* message);
    void StatArrived(size_t itemCount);
    void SnapshotArrived(size_t sizeTotal, size_t sizeRecv);

public slots:
    void OnSnapshotPressed();
    
private:
    void ComposeFilePath(DAVA::FilePath& result);

private:
    int mode;
    QPointer<MemProfWidget> view;
    QPointer<QWidget> parentWidget;

    DAVA::Net::PeerDescription profiledPeer;
    std::unique_ptr<DAVA::Net::MMNetClient> netClient;
    std::unique_ptr<ProfilingSession> profilingSession;
};

//////////////////////////////////////////////////////////////////////////
inline int MemProfController::Mode() const
{
    return mode;
}

#endif // __MEMPROFCONTROLLER_H__
