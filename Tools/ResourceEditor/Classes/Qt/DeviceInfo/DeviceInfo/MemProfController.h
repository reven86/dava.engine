#ifndef __MEMPROFCONTROLLER_H__
#define __MEMPROFCONTROLLER_H__

#include <QObject>
#include <QPointer>

#include "Network/PeerDesription.h"
#include "MemoryManager/MemoryManagerTypes.h"

namespace DAVA { namespace Net {

struct IChannelListener;
class MMNetClient;

}}

class MemProfWidget;
class ProfilingSession;

class MemProfController : public QObject
{
    Q_OBJECT
    
public:
    MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget *parentWidget, QObject *parent = nullptr);
    ~MemProfController();

    void ShowView();

    DAVA::Net::IChannelListener* NetObject() const;

    void NetConnEstablished(bool resumed, const DAVA::MMStatConfig* config);
    void NetConnLost(const DAVA::char8* message);
    void NetStatRecieved(const DAVA::MMCurStat* stat, size_t count);
    void NetDumpRecieved(int stage, size_t totalSize, size_t recvSize, const void* data);

signals:
    void ConnectionEstablished(bool newConnection, ProfilingSession* profSession);
    void ConnectionLost(const DAVA::char8* message);
    void StatArrived();
    void DumpArrived(size_t sizeTotal, size_t sizeRecv);

public slots:
    void OnDumpPressed();
    
private:
    QPointer<MemProfWidget> view;
    QPointer<QWidget> parentWidget;

    DAVA::Net::PeerDescription profiledPeer;
    std::unique_ptr<DAVA::Net::MMNetClient> netClient;
    std::unique_ptr<ProfilingSession> profilingSession;
};

#endif // __MEMPROFCONTROLLER_H__
