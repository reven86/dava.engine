#ifndef __MEMPROFCONTROLLER_H__
#define __MEMPROFCONTROLLER_H__

#include <QObject>
#include <QPointer>

#include <Network/PeerDesription.h>
#include <Network/NetService.h>

#include "MemoryManager/MemoryManager.h"

class MemProfWidget;
class MemProfInfoModel;
struct MemoryProfDataChunk;
class MemProfController : public QObject
                        , public DAVA::Net::NetService
{
    Q_OBJECT
    
public:
    explicit MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget *parentWidget, QObject *parent = NULL);
    ~MemProfController();

    void ShowView();

    virtual void ChannelOpen();
    virtual void ChannelClosed(const DAVA::char8* message);
    virtual void PacketReceived(const void* packet, size_t length);

private:
    void Output(const DAVA::String& msg);

private:
    QPointer<MemProfWidget> view;
    QPointer<QWidget> parentWidget;
    DAVA::Net::PeerDescription peer;
    
    DAVA::Vector<MemoryProfDataChunk*> v;
    MemProfInfoModel * model;
};

#endif // __MEMPROFCONTROLLER_H__
