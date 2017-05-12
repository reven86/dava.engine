#include "RemoteTool/Private/DeviceListController.h"
#include "RemoteTool/Private/DeviceList/DeviceListWidget.h"
#include "RemoteTool/Private/DeviceLogController/DeviceLogController.h"
#include "RemoteTool/Private/MemoryTool/MemProfController.h"

#include <Logger/Logger.h>
#include <Base/TemplateHelpers.h>
#include <Network/NetworkCommon.h>
#include <Network/PeerDesription.h>

#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QUuid>
#include <QMessageBox>

using namespace DAVA;
using namespace DAVA::Net;

DeviceListController::DeviceListController(QObject* parent)
    : QObject(parent)
    , model(NULL)
    , loggerServiceCreatorAsync(MakeFunction(this, &DeviceListController::CreateLogger), NetCore::Instance()->GetNetEventsDispatcher())
    , profilerServiceCreatorAsync(MakeFunction(this, &DeviceListController::CreateMemProfiler), NetCore::Instance()->GetNetEventsDispatcher())
    , loggerServiceDeleterAsync(MakeFunction(this, &DeviceListController::DeleteLogger), NetCore::Instance()->GetNetEventsDispatcher())
    , profilerServiceDeleterAsync(MakeFunction(this, &DeviceListController::DeleteMemProfiler), NetCore::Instance()->GetNetEventsDispatcher())
{
    model = new QStandardItemModel(this);

    // Register network service for recieving logs from device
    NetCore::Instance()->RegisterService(NetCore::SERVICE_LOG, MakeFunction(&loggerServiceCreatorAsync, &ServiceCreatorDispatched::ServiceCreatorCall), MakeFunction(&loggerServiceDeleterAsync, &ServiceDeleterDispatched::ServiceDeleterCall), "Logger");
    NetCore::Instance()->RegisterService(NetCore::SERVICE_MEMPROF, MakeFunction(&profilerServiceCreatorAsync, &ServiceCreatorDispatched::ServiceCreatorCall), MakeFunction(&profilerServiceDeleterAsync, &ServiceDeleterDispatched::ServiceDeleterCall), "Memory profiler");

    // Create controller for discovering remote devices
    DAVA::Net::Endpoint endpoint(NetCore::defaultAnnounceMulticastGroup, NetCore::DEFAULT_UDP_ANNOUNCE_PORT);
    DAVA::Net::NetCore::Instance()->CreateDiscoverer(endpoint, DAVA::MakeFunction(this, &DeviceListController::DiscoverCallback));
}

DeviceListController::~DeviceListController()
{
    // Block until all controllers are destroyed
    NetCore::Instance()->DestroyAllControllersBlocked();
    // We need to unregister services as we register them on window creation and duplicate services are not allowed
    NetCore::Instance()->UnregisterAllServices();
}

void DeviceListController::SetView(DeviceListWidget* _view)
{
    view = _view;
    view->ItemView()->setModel(model);

    connect(view, &DeviceListWidget::connectClicked, this, &DeviceListController::OnConnectButtonPressed);
    connect(view, &DeviceListWidget::disconnectClicked, this, &DeviceListController::OnDisconnectButtonPressed);
    connect(view, &DeviceListWidget::showLogClicked, this, &DeviceListController::OnShowLogButtonPressed);
    connect(view, &DeviceListWidget::deviceDiscoverClicked, this, &DeviceListController::OnDeviceDiscover);
}

void DeviceListController::ShowView()
{
    if (!view.isNull())
    {
        // Here code to show view if hidden or restore view if minimized or hidden by main window
        view->show();
        view->activateWindow();
        view->raise();
    }
}

void DeviceListController::OnDeviceDiscover(const QString& addr)
{
    if (!NetCore::IsNetworkEnabled())
    {
        Logger::Warning("[DeviceListController] Network is disabled in this build");
        return;
    }

    // addr should be in form ipaddress:port, port is optional and by default is 9999

    uint16 port = NetCore::DEFAULT_TCP_ANNOUNCE_PORT;
    bool valid = true;
    int delimeter = addr.indexOf(QChar(':'));
    if (delimeter > 0)
    {
        port = addr.mid(delimeter + 1).toUShort(&valid);
    }
    if (valid)
    {
        String ipaddrPart = addr.mid(0, delimeter).toStdString().c_str();

        IPAddress ipaddr = IPAddress::FromString(ipaddrPart);
        valid = !ipaddr.IsUnspecified();
        if (valid)
        {
            Endpoint endp(ipaddr, port);
            if (!NetCore::Instance()->TryDiscoverDevice(endp))
            {
                Logger::Warning("Device discoverer is busy now, try later");
            }
        }
    }

    if (!valid)
    {
        QMessageBox::critical(view, "Invalid input", "Expected input: xxx.xxx.xxx.xxx[:port]\n"
                                                     "   xxx.xxx.xxx.xxx - destination IP address\n"
                                                     "   port - optional port, default is 9998");
    }
}

IChannelListener* DeviceListController::CreateLogger(uint32 serviceId, void* context)
{
    // Service creator method is called each time when connection has been established
    // As network service was created when 'Connect' button has been pressed so here simply return
    // pointer to created service

    // Context holds index of discovered device
    int row = static_cast<int>(reinterpret_cast<intptr_t>(context));
    if (model != NULL && 0 <= row && row < model->rowCount())
    {
        QModelIndex index = model->index(row, 0);
        DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
        return services.log->GetAsyncChannelListener();
    }
    return NULL;
}

void DeviceListController::DeleteLogger(IChannelListener*, void* context)
{
    // Service deleter method is called before connector is destroyed

    // Context holds index of discovered device
    int row = static_cast<int>(reinterpret_cast<intptr_t>(context));
    if (model != NULL && 0 <= row && row < model->rowCount())
    {
        QModelIndex index = model->index(row, 0);

        QStandardItem* item = model->itemFromIndex(index);
        if (item != NULL)
        {
            DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
            services.log.reset();

            QVariant v;
            v.setValue(services);
            item->setData(v, ROLE_PEER_SERVICES);
        }
    }
}

IChannelListener* DeviceListController::CreateMemProfiler(uint32 serviceId, void* context)
{
    int row = static_cast<int>(reinterpret_cast<intptr_t>(context));
    if (model != NULL && 0 <= row && row < model->rowCount())
    {
        QModelIndex index = model->index(row, 0);
        DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
        return services.memprof->GetAsyncChannelListener();
    }
    return NULL;
}

void DeviceListController::DeleteMemProfiler(IChannelListener* obj, void* context)
{
    int row = static_cast<int>(reinterpret_cast<intptr_t>(context));
    if (model != NULL && 0 <= row && row < model->rowCount())
    {
        QModelIndex index = model->index(row, 0);

        QStandardItem* item = model->itemFromIndex(index);
        if (item != NULL)
        {
            DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
            services.memprof.reset();

            QVariant v;
            v.setValue(services);
            item->setData(v, ROLE_PEER_SERVICES);
        }
    }
}

void DeviceListController::ConnectDeviceInternal(QModelIndex& index, size_t ifIndex)
{
    // Check whether we have connection with device
    NetCore::TrackId trackId = static_cast<NetCore::TrackId>(index.data(ROLE_CONNECTION_ID).toULongLong());
    if (trackId != NetCore::INVALID_TRACK_ID)
    {
        DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
        services.log->ShowView();
        return;
    }

    Endpoint endp = index.data(ROLE_SOURCE_ADDRESS).value<Endpoint>();
    PeerDescription peer = index.data(ROLE_PEER_DESCRIPTION).value<PeerDescription>();
    {
        IPAddress addr = endp.Address(); // Use IP address from multicast packets
        NetConfig config = peer.NetworkConfig().Mirror(addr);
        const Vector<uint32>& servIds = config.Services();

        // Check whether remote device is under memory profiler and increase read timeout
        // Else leave it zero to allow underlying network system to choose timeout itself
        bool deviceUnderMemoryProfiler = std::find(servIds.begin(), servIds.end(), NetCore::SERVICE_MEMPROF) != servIds.end();
        uint32 readTimeout = deviceUnderMemoryProfiler ? 120 * 1000 : Net::DEFAULT_READ_TIMEOUT;

        QStandardItem* item = model->itemFromIndex(index);
        if (NULL == item)
            return;

        // Append prefix 'ACTIVE!' to distinguish active objects
        QString s = item->text();
        item->setText("ACTIVE! " + s);

        {
            DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
            // Check whether remote device has corresponding services
            auto iterService = std::find(servIds.begin(), servIds.end(), NetCore::SERVICE_LOG);
            if (iterService != servIds.end())
            {
                services.log.reset(new DeviceLogController(peer, view, this));
                services.log->Init();
            }
            iterService = std::find(servIds.begin(), servIds.end(), NetCore::SERVICE_MEMPROF);
            if (iterService != servIds.end())
            {
                services.memprof.reset(new MemProfController(peer, view, this));
            }

            QVariant v;
            v.setValue(services);
            item->setData(v, ROLE_PEER_SERVICES);
        }

        trackId = NetCore::Instance()->CreateController(config, reinterpret_cast<void*>(index.row()), readTimeout);
        if (trackId != NetCore::INVALID_TRACK_ID)
        {
            // Update item's ROLE_CONNECTION_ID and ROLE_PEER_SERVICES
            item->setData(QVariant(static_cast<qulonglong>(trackId)), ROLE_CONNECTION_ID);
        }
    }
}

void DeviceListController::DisonnectDeviceInternal(QModelIndex& index)
{
    // Check whether we have connection with device
    NetCore::TrackId trackId = static_cast<NetCore::TrackId>(index.data(ROLE_CONNECTION_ID).toULongLong());
    if (NetCore::INVALID_TRACK_ID == trackId)
        return;

    // Cleare item's ROLE_CONNECTION_ID to
    QStandardItem* item = model->itemFromIndex(index);
    if (NULL == item)
        return;

    item->setData(QVariant(static_cast<qulonglong>(NetCore::INVALID_TRACK_ID)), ROLE_CONNECTION_ID);
    // And destroy controller related to remote device
    DAVA::Net::NetCore::Instance()->DestroyControllerBlocked(trackId);

    QString s = item->text();
    int pos = s.indexOf('!');
    if (pos > 0)
    {
        item->setText(s.mid(pos + 2));
    }
}

void DeviceListController::OnConnectButtonPressed()
{
    if (!NetCore::IsNetworkEnabled())
    {
        Logger::Warning("[DeviceListController] Network is disabled in this build");
        return;
    }

    // 'Connect' button has been pressed
    QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();
    for (int i = 0; i < selection.size(); i++)
    {
        QModelIndex& index = selection[i];
        if (index.parent().isValid())
            continue;
        // Do actual connect to device which description is stored in item identified by index
        // and using device's first network interface
        ConnectDeviceInternal(index, 0);
    }
}

void DeviceListController::OnDisconnectButtonPressed()
{
    if (!NetCore::IsNetworkEnabled())
    {
        Logger::Warning("[DeviceListController] Network is disabled in this build");
        return;
    }

    // 'Disconnect' button has been pressed
    QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();
    for (int i = 0; i < selection.size(); i++)
    {
        QModelIndex& index = selection[i];
        if (index.parent().isValid())
            continue;
        DisonnectDeviceInternal(index);
    }
}

void DeviceListController::OnShowLogButtonPressed()
{
    if (!NetCore::IsNetworkEnabled())
    {
        Logger::Warning("[DeviceListController] Network is disabled in this build");
        return;
    }

    // 'Show log' button has been pressed
    QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();
    for (int i = 0; i < selection.size(); i++)
    {
        QModelIndex& index = selection[i];
        if (index.parent().isValid())
            continue;

        // If connection has been established then show log output window
        NetCore::TrackId trackId = static_cast<NetCore::TrackId>(index.data(ROLE_CONNECTION_ID).toULongLong());
        if (trackId != NetCore::INVALID_TRACK_ID)
        {
            DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
            if (services.log)
            {
                services.log->ShowView();
            }
            if (services.memprof)
            {
                services.memprof->ShowView();
            }
        }
    }
}

QStandardItem* DeviceListController::CreateDeviceItem(const Endpoint& endp, const PeerDescription& peerDescr)
{
    // Item text in the form of <name> - <platform> - <ip address>
    // E.g., 9f5656fd - Android - 192.168.0.24
    const QString caption = QString("%1 - %2 - %3")
                            .arg(peerDescr.GetName().c_str())
                            .arg(peerDescr.GetPlatformString().c_str())
                            .arg(endp.Address().ToString().c_str());
    QStandardItem* item = new QStandardItem();
    item->setText(caption);

    // Set item's properties:
    //  - empty connection id
    //  - zero active flag
    //  - endpoint from which device description has been obtained
    //  - obtained device description
    //  - empty network services
    item->setData(QVariant(static_cast<qulonglong>(NetCore::INVALID_TRACK_ID)), ROLE_CONNECTION_ID);
    {
        QVariant v;
        v.setValue(endp);
        item->setData(v, ROLE_SOURCE_ADDRESS);
    }
    {
        QVariant v;
        v.setValue(peerDescr);
        item->setData(v, ROLE_PEER_DESCRIPTION);
    }
    {
        QVariant v;
        v.setValue(DeviceServices());
        item->setData(v, ROLE_PEER_SERVICES);
    }

    {
        // Add subitem with text: <manufacturer> <model> <platform> <version>
        // E.g. Samsung SM-G900F Android 4.4.2
        const QString text = QString("%1 %2 %3 %4")
                             .arg(peerDescr.GetManufacturer().c_str())
                             .arg(peerDescr.GetModel().c_str())
                             .arg(peerDescr.GetPlatformString().c_str())
                             .arg(peerDescr.GetVersion().c_str());
        QStandardItem* subitem = new QStandardItem();
        subitem->setText(text);
        item->appendRow(subitem);
    }
    {
        // Add list of available network interfaces
        QStandardItem* top = new QStandardItem();
        top->setText("Available interfaces");
        item->appendRow(top);

        DVASSERT(false == peerDescr.NetworkInterfaces().empty());
        const Vector<IfAddress>& v = peerDescr.NetworkInterfaces();
        for (size_t i = 0, n = v.size(); i < n; ++i)
        {
            char8 sphys[30];
            const IfAddress::PhysAddress& phys = v[i].PhysicalAddress();
            Snprintf(sphys, COUNT_OF(sphys), "%02X:%02X:%02X:%02X:%02X:%02X"
                     ,
                     phys.data[0]
                     ,
                     phys.data[1]
                     ,
                     phys.data[2]
                     ,
                     phys.data[3]
                     ,
                     phys.data[4]
                     ,
                     phys.data[5]);
            const QString text = QString("IP=%1, MAC=%2")
                                 .arg(v[i].Address().ToString().c_str())
                                 .arg(sphys);
            QStandardItem* subitem = new QStandardItem();
            subitem->setText(text);
            top->appendRow(subitem);
        }
    }
    {
        // Add list of available transports
        QStandardItem* top = new QStandardItem();
        top->setText("Available transports");
        item->appendRow(top);

        const Vector<NetConfig::TransportConfig>& tr = peerDescr.NetworkConfig().Transports();
        for (size_t i = 0, n = tr.size(); i < n; ++i)
        {
            const char* str = "!!!";
            if (tr[i].type == TRANSPORT_TCP)
                str = "TCP";
            const QString text = QString("%1 - %2")
                                 .arg(str)
                                 .arg(tr[i].endpoint.ToString().c_str());
            QStandardItem* subitem = new QStandardItem();
            subitem->setText(text);
            top->appendRow(subitem);
        }
    }
    {
        // Add list of available services, here service name becomes useful
        QStandardItem* top = new QStandardItem();
        top->setText("Available services");
        item->appendRow(top);

        const Vector<uint32>& serv = peerDescr.NetworkConfig().Services();
        for (size_t i = 0, n = serv.size(); i < n; ++i)
        {
            const char8* name = NetCore::Instance()->ServiceName(serv[i]);
            const QString text = name != NULL ? QString(name)
                                                :
                                                QString("Unknown service %1").arg(serv[i]);
            QStandardItem* subitem = new QStandardItem();
            subitem->setText(text);
            top->appendRow(subitem);
        }
    }
    return item;
}

void DeviceListController::DiscoverCallback(size_t buflen, const void* buffer, const DAVA::Net::Endpoint& endpoint)
{
    // This method is called when announce packet has arrived

    // Check whether device has been already announced, check by address from which packet recieved
    if (!AlreadyInModel(endpoint))
    {
        PeerDescription peer;
        if (peer.Deserialize(buffer, buflen) > 0)
        {
            QStandardItem* item = CreateDeviceItem(endpoint, peer);
            model->appendRow(item);
            if (view)
            {
                QTreeView* treeView = view->ItemView();
                treeView->expand(item->index());
            }
        }
    }
}

bool DeviceListController::AlreadyInModel(const Endpoint& endp) const
{
    for (int i = 0, n = model->rowCount(); i < n; ++i)
    {
        QVariant v = model->item(i)->data(ROLE_SOURCE_ADDRESS);
        if (endp == v.value<Endpoint>())
        {
            return true;
        }
    }
    return false;
}
