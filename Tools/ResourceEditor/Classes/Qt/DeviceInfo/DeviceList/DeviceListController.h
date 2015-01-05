#ifndef __DEVICELISTCONTROLLER_H__
#define __DEVICELISTCONTROLLER_H__


#include <QObject>
#include <QPointer>
#include <QMap>


class QStandardItemModel;
class QStandardItem;

class DeviceListWidget;


class DeviceListController
    : public QObject
{
    Q_OBJECT

    enum DeviceDataRole
    {
        DeviceId = Qt::UserRole + 1,
    };

public:
    explicit DeviceListController( QObject *parent = NULL );
    ~DeviceListController();

    void SetView( DeviceListWidget *view );
    void AddDeviceInfo( QStandardItem* item );

    void NewDeviceCallback();   // TODO: implement

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

private:
    static QStandardItem *createDeviceItem( quintptr id, const QString& title );
};



#endif // __DEVICELISTCONTROLLER_H__
