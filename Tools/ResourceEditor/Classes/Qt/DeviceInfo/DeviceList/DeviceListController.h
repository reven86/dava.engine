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
class MemProfController;

// Struct that holds network services for remote device
// For now only one service - log receiver
struct DeviceServices
{
    DeviceServices() : log(NULL) {}

    DeviceLogController* log;
    MemProfController* memprof;
};

// Register types for use with QVariant
Q_DECLARE_METATYPE(DAVA::Net::Endpoint);
Q_DECLARE_METATYPE(DAVA::Net::PeerDescription);
Q_DECLARE_METATYPE(DeviceServices);

class DeviceListController : public QObject
{
    Q_OBJECT

    enum
    {
        SERVICE_LOG = 0,
        SERVICE_MEMPROF = 1
    };

    enum DeviceDataRole
    {
        // Roles for each item in QStandardItemModel
        ROLE_CONNECTION_ID = Qt::UserRole + 1,  // Store NetCore::TrackId to track whether device is connected or no
        ROLE_SOURCE_ADDRESS,                    // Store endpoint announce has arrived from
        ROLE_PEER_DESCRIPTION,                  // Store device description recieved from announce
        ROLE_PEER_SERVICES                      // Store network services to communicate with remote device
    };

    static const DAVA::uint16 ANNOUNCE_PORT = 9999;
    static const DAVA::char8 announceMulticastGroup[];

public:
    explicit DeviceListController(QObject *parent = NULL);
    ~DeviceListController();

    void SetView(DeviceListWidget *view);
    void ShowView();

    // Method invoked when announce packet arrived
    void DiscoverCallback(size_t buflen, const void* buffer, const DAVA::Net::Endpoint& endpoint);
    // Method invoked when all network controllers were stopped
    void AllStopped();

private slots:
    void OnConnectButtonPressed();
    void OnDisconnectButtonPressed();
    void OnShowLogButtonPressed();

private:
    void ConnectDeviceInternal(QModelIndex& index, size_t ifIndex);
    void DisonnectDeviceInternal(QModelIndex& index);

    // Methods to create and delete network services
    DAVA::Net::IChannelListener* CreateLogger(DAVA::uint32 serviceId, void* context);
    void DeleteLogger(DAVA::Net::IChannelListener*, void* context);
    
    DAVA::Net::IChannelListener* CreateMemProfiler(DAVA::uint32 serviceId, void* context);
    void DeleteMemProfiler(DAVA::Net::IChannelListener* obj, void* context);

    // Check whether device already has been discovered
    bool AlreadyInModel(const DAVA::Net::Endpoint& endp) const;

private:
    QPointer<QStandardItemModel> model;
    QPointer<DeviceListWidget> view;

private:
    static QStandardItem *CreateDeviceItem(const DAVA::Net::Endpoint& endp, const DAVA::Net::PeerDescription& peerDescr);
};



#endif // __DEVICELISTCONTROLLER_H__
