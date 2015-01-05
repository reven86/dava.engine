#ifndef __DEVICELISTCONTROLLER_H__
#define __DEVICELISTCONTROLLER_H__


#include <QObject>
#include <QPointer>
#include <vector>

#include <Network/NetCore.h>
#include <Network/Base/Endpoint.h>

class QStandardItemModel;
class QStandardItem;

class DeviceListWidget;


struct SomeInfo
{
    quint64 field1;
    std::vector< int > field2;
};

Q_DECLARE_METATYPE( SomeInfo );


class DeviceListController
    : public QObject
{
    Q_OBJECT

    enum DeviceDataRole
    {
        DeviceId = Qt::UserRole + 1,
        CustomData,
    };

public:
    explicit DeviceListController( QObject *parent = NULL );
    ~DeviceListController();

    void SetView( DeviceListWidget *view );
    void AddDeviceInfo( QStandardItem* item );

    void NewDeviceCallback();   // TODO: implement
    void DiscoverCallback(size_t buflen, const void* buffer, const DAVA::Net::Endpoint& endpoint);

private slots:
    void OnConnectDevice();
    void OnDisconnectDevice();
    void OnShowInfo();

private:
    void initModel();

    QStandardItem* GetItemFromIndex( const QModelIndex& index );

    void ConnectDeviceInternal( quintptr id );
    void DisonnectDeviceInternal( quintptr id );

    QPointer<QStandardItemModel> model;
    QPointer<DeviceListWidget> view;

    DAVA::Net::NetCore::TrackId idDiscoverer;

private:
    static QStandardItem *createDeviceItem( quintptr id, const QString& title );
};



#endif // __DEVICELISTCONTROLLER_H__
