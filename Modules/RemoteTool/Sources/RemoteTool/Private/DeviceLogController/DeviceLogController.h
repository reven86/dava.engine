#pragma once

#include <Network/PeerDesription.h>
#include <Network/NetService.h>

#include <QObject>
#include <QPointer>

class LogWidget;

class DeviceLogController : public QObject,
                            public DAVA::Net::NetService
{
    Q_OBJECT

public:
    explicit DeviceLogController(const DAVA::Net::PeerDescription& peerDescr, QWidget* parentWidget, QObject* parent = NULL);
    ~DeviceLogController();

    void ShowView();

    virtual void ChannelOpen();
    virtual void ChannelClosed(const DAVA::char8* message);
    virtual void PacketReceived(const void* packet, size_t length);

private:
    void Output(const DAVA::String& msg);

private:
    QPointer<LogWidget> view;
    QPointer<QWidget> parentWidget;
    DAVA::Net::PeerDescription peer;
};
