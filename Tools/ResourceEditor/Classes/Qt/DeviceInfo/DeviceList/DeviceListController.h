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
class DeviceInfoController;

/*struct DiscoveredPeer
{
    DAVA::Net::Endpoint sourceEndpoint;
    DAVA::Net::PeerDescription descr;
};*/

Q_DECLARE_METATYPE(DAVA::Net::IPAddress);
Q_DECLARE_METATYPE(DAVA::Net::PeerDescription);

class DeviceListController
    : public QObject
{
    Q_OBJECT

    enum DeviceDataRole
    {
        ROLE_SOURCE_ADDRESS = Qt::UserRole + 1,
        ROLE_PEER_DESCRIPTION,
    };

public:
    explicit DeviceListController( QObject *parent = NULL );
    ~DeviceListController();

    void SetView( DeviceListWidget *view );
    void AddDeviceInfo( QStandardItem* item );

    void DiscoverCallback(size_t buflen, const void* buffer, const DAVA::Net::Endpoint& endpoint);

private slots:
    void OnConnectDevice();
    void OnDisconnectDevice();
    void OnShowInfo();

private:
    void initModel();

    QStandardItem* GetItemFromIndex( const QModelIndex& index );

    DAVA::Net::NetCore::TrackId ConnectDeviceInternal(const DAVA::Net::PeerDescription& peer);
    void DisonnectDeviceInternal(DAVA::Net::NetCore::TrackId id);

    DAVA::Net::IChannelListener* CreateLogger(DAVA::uint32 serviceId);
    void DeleteLogger(DAVA::Net::IChannelListener*);

    bool AlreadyInModel(const DAVA::Net::Endpoint& endp) const;

private:
    QPointer<QStandardItemModel> model;
    QPointer<DeviceListWidget> view;

    DAVA::Net::NetCore::TrackId idDiscoverer;
    DAVA::Net::NetCore::TrackId idDevice;
    DeviceInfoController* infoCtrl;
    DAVA::Net::PeerDescription curDescr;

private:
    static QStandardItem *createDeviceItem(const DAVA::Net::Endpoint& endp, const DAVA::Net::PeerDescription& peerDescr);
};



#endif // __DEVICELISTCONTROLLER_H__
