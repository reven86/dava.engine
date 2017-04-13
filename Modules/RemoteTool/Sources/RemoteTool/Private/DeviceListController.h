#pragma once

#include <vector>

#include <Tools/NetworkHelpers/ServiceCreatorDispatched.h>
#include <Tools/NetworkHelpers/ServiceDeleterDispatched.h>

#include <Network/NetCore.h>
#include <Network/Base/Endpoint.h>
#include <Network/PeerDesription.h>

#include <QObject>
#include <QPointer>

class QStandardItemModel;
class QStandardItem;

class DeviceListWidget;
class DeviceLogController;
class MemProfController;

// Struct that holds network services for remote device
// For now only one service - log receiver
struct DeviceServices
{
    std::shared_ptr<DeviceLogController> log;
    std::shared_ptr<MemProfController> memprof;
};

// Register types for use with QVariant
Q_DECLARE_METATYPE(DAVA::Net::Endpoint);
Q_DECLARE_METATYPE(DAVA::Net::PeerDescription);
Q_DECLARE_METATYPE(DeviceServices);

class DeviceListController : public QObject
{
    Q_OBJECT

    enum DeviceDataRole
    {
        // Roles for each item in QStandardItemModel
        ROLE_CONNECTION_ID = Qt::UserRole + 1, // Store NetCore::TrackId to track whether device is connected or no
        ROLE_SOURCE_ADDRESS, // Store endpoint announce has arrived from
        ROLE_PEER_DESCRIPTION, // Store device description recieved from announce
        ROLE_PEER_SERVICES // Store network services to communicate with remote device
    };

public:
    explicit DeviceListController(QObject* parent = NULL);
    ~DeviceListController();

    void SetView(DeviceListWidget* view);
    void ShowView();

    // Method invoked when announce packet arrived
    void DiscoverCallback(size_t buflen, const void* buffer, const DAVA::Net::Endpoint& endpoint);
    // Method invoked when all network controllers were stopped
    void AllStopped();

private slots:
    void OnConnectButtonPressed();
    void OnDisconnectButtonPressed();
    void OnShowLogButtonPressed();
    void OnDeviceDiscover(const QString& addr);

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

    DAVA::Net::ServiceCreatorDispatched loggerServiceCreatorAsync;
    DAVA::Net::ServiceDeleterDispatched loggerServiceDeleterAsync;

    DAVA::Net::ServiceCreatorDispatched profilerServiceCreatorAsync;
    DAVA::Net::ServiceDeleterDispatched profilerServiceDeleterAsync;

private:
    static QStandardItem* CreateDeviceItem(const DAVA::Net::Endpoint& endp, const DAVA::Net::PeerDescription& peerDescr);
};
