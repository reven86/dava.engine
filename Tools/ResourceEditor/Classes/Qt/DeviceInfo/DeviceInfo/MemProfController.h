#ifndef __MEMPROFCONTROLLER_H__
#define __MEMPROFCONTROLLER_H__

#include <QObject>
#include <QPointer>

#include "Network/PeerDesription.h"
#include "Network/Services/MMNetClient.h"

class MemProfWidget;
class DumpViewWidget;

class MemProfController : public QObject
{
    Q_OBJECT
    
public:
    explicit MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget *parentWidget, QObject *parent = NULL);
    ~MemProfController();

    void ShowView();

    void ChannelOpen(const DAVA::MMStatConfig* config);
    void ChannelClosed(const DAVA::char8* message);
    void CurrentStat(const DAVA::MMStat* stat);
    void Dump(size_t total, size_t recv);
    void DumpDone(const DAVA::MMDump* dump, size_t packedSize, DAVA::Vector<DAVA::uint8>& dumpV);

    DAVA::Net::IChannelListener* NetObject() { return &netClient; }

public slots:
    void OnDumpPressed();
    void OnViewDump();
    void OnViewFileDump();
    
private:
    void Output(const DAVA::String& msg);

private:
    QPointer<MemProfWidget> view;
    QPointer<DumpViewWidget> viewDump;
    QPointer<QWidget> parentWidget;
    DAVA::Net::PeerDescription peer;
    DAVA::Net::MMNetClient netClient;

    DAVA::Vector<DAVA::uint8> dumpData;
    //std::unordered_map<DAVA::uint64, DAVA::String> symbolMap;
    //std::unordered_map<DAVA::uint32, DAVA::MMBacktrace> traceMap;
};

#endif // __MEMPROFCONTROLLER_H__
