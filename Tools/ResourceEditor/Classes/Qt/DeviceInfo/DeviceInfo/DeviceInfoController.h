#ifndef __DEVICEINFOCONTROLLER_H__
#define __DEVICEINFOCONTROLLER_H__


#include <QObject>
#include <QPointer>

#include <Network/NetService.h>

class DeviceInfoWidget;


class DeviceInfoController
    : public QObject
    , public DAVA::Net::NetService
{
    Q_OBJECT

public:
    explicit DeviceInfoController( QWidget *parentWidget, QObject *parent = NULL );
    ~DeviceInfoController();

    void InitView();

    virtual void ChannelOpen();
    virtual void ChannelClosed();
    virtual void PacketReceived(const void* packet, size_t length);

private:

    void DebugOutput();

    QPointer<DeviceInfoWidget> view;
    QPointer<QWidget> parentWidget;
};



#endif // __DEVICEINFOCONTROLLER_H__
