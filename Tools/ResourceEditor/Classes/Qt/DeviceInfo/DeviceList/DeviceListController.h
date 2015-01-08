#ifndef __DEVICELISTCONTROLLER_H__
#define __DEVICELISTCONTROLLER_H__


#include <QObject>
#include <QPointer>
#include <vector>

#include <Network/NetCore.h>
#include <Network/Base/Endpoint.h>

#include <Network/PeerDesription.h>

class QStandardItemModel;
class QStandardItem;

class DeviceListWidget;
class DeviceLogController;

struct DeviceServices
{
    DeviceServices() : log(NULL) {}

    DeviceLogController* log;
};

Q_DECLARE_METATYPE(DAVA::Net::NetCore::TrackId);
Q_DECLARE_METATYPE(DAVA::Net::Endpoint);
Q_DECLARE_METATYPE(DAVA::Net::PeerDescription);
Q_DECLARE_METATYPE(DeviceServices);

class DeviceListController
    : public QObject
{
    Q_OBJECT

    enum DeviceDataRole
    {
        ROLE_CONNECTION_ID = Qt::UserRole + 1,
        ROLE_SOURCE_ADDRESS,
        ROLE_PEER_DESCRIPTION,
        ROLE_PEER_SERVICES
    };

public:
    explicit DeviceListController(QObject *parent = NULL);
    ~DeviceListController();

    void SetView(DeviceListWidget *view);

    void DiscoverCallback(size_t buflen, const void* buffer, const DAVA::Net::Endpoint& endpoint);

private slots:
    void OnConnectDevice();
    void OnDisconnectDevice();
    void OnShowInfo();
    void OnCloseEvent();

private:
    void initModel();

    QStandardItem* GetItemFromIndex( const QModelIndex& index );

    void ConnectDeviceInternal(QModelIndex& index, size_t ifIndex);
    void DisonnectDeviceInternal(QModelIndex& index);

    DAVA::Net::IChannelListener* CreateLogger(DAVA::uint32 serviceId, void* arg);
    void DeleteLogger(DAVA::Net::IChannelListener*, void* arg);

    DAVA::Net::IChannelListener* CreateEcho(DAVA::uint32 serviceId, void* arg);
    void DeleteEcho(DAVA::Net::IChannelListener*, void* arg);

    bool AlreadyInModel(const DAVA::Net::Endpoint& endp) const;

private:
    QPointer<QStandardItemModel> model;
    QPointer<DeviceListWidget> view;

    DAVA::Net::NetCore::TrackId idDiscoverer;

private:
    static QStandardItem *createDeviceItem(const DAVA::Net::Endpoint& endp, const DAVA::Net::PeerDescription& peerDescr);
};



#endif // __DEVICELISTCONTROLLER_H__
