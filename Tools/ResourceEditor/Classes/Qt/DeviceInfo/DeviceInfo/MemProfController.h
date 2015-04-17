#ifndef __MEMPROFCONTROLLER_H__
#define __MEMPROFCONTROLLER_H__

#include <QObject>
#include <QPointer>

#include "Network/PeerDesription.h"
#include "Network/Services/MMNetClient.h"

class MemProfWidget;
class ProfilingSession;

class MemProfController : public QObject
{
    Q_OBJECT
    
public:
    explicit MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget *parentWidget, QObject *parent = NULL);
    ~MemProfController();

    void ShowView();

    void OnChannelOpen(const DAVA::MMStatConfig* config);
    void OnChannelClosed(const DAVA::char8* message);
    void OnCurrentStat(const DAVA::MMCurStat* stat);
    void OnDump(size_t total, size_t recv, DAVA::Vector<DAVA::uint8>* v);

    DAVA::Net::IChannelListener* NetObject() { return &netClient; }

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
    DAVA::Net::PeerDescription peer;
    DAVA::Net::MMNetClient netClient;

    DAVA::Vector<DAVA::uint8> dumpData;

    std::unique_ptr<ProfilingSession> profilingSession;
};

#endif // __MEMPROFCONTROLLER_H__
