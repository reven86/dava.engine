#ifndef __MEMPROFCONTROLLER_H__
#define __MEMPROFCONTROLLER_H__

#include <QObject>
#include <QPointer>

#include "Network/PeerDesription.h"
#include "Network/Services/MMNetClient.h"

class MemProfWidget;

class MemProfController : public QObject
{
    Q_OBJECT
    
public:
    explicit MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget *parentWidget, QObject *parent = NULL);
    ~MemProfController();

    void ShowView();

    void ChannelOpen();
    void ChannelClosed(DAVA::char8* message);
    void PacketReceived(const void* packet, size_t length);

    DAVA::Net::IChannelListener* NetObject() { return &netClient; }

private:
    void Output(const DAVA::String& msg);

private:
    QPointer<MemProfWidget> view;
    QPointer<QWidget> parentWidget;
    DAVA::Net::PeerDescription peer;
    DAVA::Net::MMNetClient netClient;
};

#endif // __MEMPROFCONTROLLER_H__
