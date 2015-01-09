#include "DeviceListController.h"


#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QUuid>

#include "DeviceListWidget.h"

#include "Classes/Qt/DeviceInfo/DeviceInfo/DeviceLogController.h"
#include <Base/FunctionTraits.h>

#include <Network/PeerDesription.h>

using namespace DAVA;
using namespace DAVA::Net;

DeviceListController::DeviceListController(QObject* parent)
    : QObject(parent)
    , model(NULL)
{
    initModel();

    NetCore::Instance()->RegisterService(0, MakeFunction(this, &DeviceListController::CreateLogger), MakeFunction(this, &DeviceListController::DeleteLogger), "Logger");

    DAVA::Net::Endpoint endpoint("239.192.100.1", 9999);
    DAVA::Net::NetCore::Instance()->CreateDiscoverer(endpoint, DAVA::MakeFunction(this, &DeviceListController::DiscoverCallback));
}

DeviceListController::~DeviceListController()
{
}

void DeviceListController::OnCloseEvent()
{
    NetCore::Instance()->DestroyAllControllers(MakeFunction(this, &DeviceListController::AllStopped));
}

void DeviceListController::AllStopped()
{
    // Call UnregisterAllServices here, not in destructor:
    //  when closing ResourceEditor's main window while DeviceListWidget is open
    //  NetCore destructor is called before DeviceListController's destructor
    NetCore::Instance()->UnregisterAllServices();
    view->deleteLater();
}

void DeviceListController::SetView(DeviceListWidget* _view)
{
    view = _view;
    view->ItemView()->setModel(model);

    connect(view, &DeviceListWidget::connectClicked, this, &DeviceListController::OnConnectDevice);
    connect(view, &DeviceListWidget::disconnectClicked, this, &DeviceListController::OnDisconnectDevice);
    connect(view, &DeviceListWidget::showLogClicked, this, &DeviceListController::OnShowLog);
    connect(view, &DeviceListWidget::closeRequest, this, &DeviceListController::OnCloseEvent);
}

void DeviceListController::initModel()
{
    delete model;
    model = new QStandardItemModel(this);
}

QStandardItem* DeviceListController::GetItemFromIndex(const QModelIndex& index)
{
    return model->itemFromIndex(index);
}

IChannelListener* DeviceListController::CreateLogger(uint32 serviceId, void* arg)
{
    int row = static_cast<int>(reinterpret_cast<intptr_t>(arg));
    DVASSERT(model != NULL && 0 <= row && row < model->rowCount());
    if (model != NULL && 0 <= row && row < model->rowCount())
    {
        QModelIndex index = model->index(row, 0);
        DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
        return services.log;
    }
    return NULL;
}

void DeviceListController::DeleteLogger(IChannelListener*, void* arg)
{
    int row = static_cast<int>(reinterpret_cast<intptr_t>(arg));
    DVASSERT(model != NULL && 0 <= row && row < model->rowCount());
    if (model != NULL && 0 <= row && row < model->rowCount())
    {
        QModelIndex index = model->index(row, 0);
        NetCore::TrackId trackId = static_cast<NetCore::TrackId>(index.data(ROLE_CONNECTION_ID).toULongLong());
        if (trackId == NetCore::INVALID_TRACK_ID)
        {
            DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
            SafeDelete(services.log);

            QStandardItem* item = model->itemFromIndex(index);
            {
                QVariant v;
                v.setValue(services);
                item->setData(v, ROLE_PEER_SERVICES);
            }
        }
    }
}

void DeviceListController::ConnectDeviceInternal(QModelIndex& index, size_t ifIndex)
{
    NetCore::TrackId trackId = static_cast<NetCore::TrackId>(index.data(ROLE_CONNECTION_ID).toULongLong());
    if (trackId != NetCore::INVALID_TRACK_ID) return;

    PeerDescription peer = index.data(ROLE_PEER_DESCRIPTION).value<PeerDescription>();
    if (ifIndex < peer.NetworkInterfaces().size())
    {
        IPAddress addr = peer.NetworkInterfaces()[ifIndex].Address();
        NetConfig config = peer.NetworkConfig().Mirror(addr);
        trackId = NetCore::Instance()->CreateController(config, reinterpret_cast<void*>(index.row()));
        if (trackId != NetCore::INVALID_TRACK_ID)
        {
            DeviceServices services;
            services.log = new DeviceLogController(peer, view, this);

            QStandardItem* item = model->itemFromIndex(index);
            item->setData(QVariant(trackId), ROLE_CONNECTION_ID);
            {
                QVariant v;
                v.setValue(services);
                item->setData(v, ROLE_PEER_SERVICES);
            }
        }
    }
}

void DeviceListController::DisonnectDeviceInternal(QModelIndex& index)
{
    NetCore::TrackId trackId = static_cast<NetCore::TrackId>(index.data(ROLE_CONNECTION_ID).toULongLong());
    if (NetCore::INVALID_TRACK_ID == trackId) return;

    QStandardItem* item = model->itemFromIndex(index);
    item->setData(QVariant(NetCore::INVALID_TRACK_ID), ROLE_CONNECTION_ID);
    DAVA::Net::NetCore::Instance()->DestroyController(trackId);
}

void DeviceListController::OnConnectDevice()
{
    QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();
    for (int i = 0; i < selection.size(); i++)
    {
        QModelIndex& index = selection[i];
        if (index.parent().isValid())
            continue;
        ConnectDeviceInternal(index, 0);
    }
}

void DeviceListController::OnDisconnectDevice()
{
    QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();
    for (int i = 0; i < selection.size(); i++)
    {
        QModelIndex& index = selection[i];
        if (index.parent().isValid())
            continue;
        DisonnectDeviceInternal(index);
    }
}

void DeviceListController::OnShowLog()
{
    QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();
    for (int i = 0; i < selection.size(); i++)
    {
        QModelIndex& index = selection[i];
        if (index.parent().isValid())
            continue;

        NetCore::TrackId trackId = static_cast<NetCore::TrackId>(index.data(ROLE_CONNECTION_ID).toULongLong());
        if (trackId != NetCore::INVALID_TRACK_ID)
        {
            DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
            services.log->ShowView();
        }
    }
}

QStandardItem *DeviceListController::createDeviceItem(const Endpoint& endp, const PeerDescription& peerDescr)
{
    const QString caption = QString("%1 - %2")
        .arg(peerDescr.GetName().c_str())
        .arg(peerDescr.GetPlatformString().c_str());
    QStandardItem *item = new QStandardItem();
    item->setText(caption);

    item->setData(QVariant(NetCore::INVALID_TRACK_ID), ROLE_CONNECTION_ID);
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
        const QString text = QString("%1 %2 %3 %4")
            .arg(peerDescr.GetManufacturer().c_str())
            .arg(peerDescr.GetModel().c_str())
            .arg(peerDescr.GetPlatformString().c_str())
            .arg(peerDescr.GetVersion().c_str());
        QStandardItem *subitem = new QStandardItem();
        subitem->setText(text);
        item->appendRow(subitem);
    }
    {
        QStandardItem* top = new QStandardItem();
        top->setText("Available interfaces");
        item->appendRow(top);

        DVASSERT(false == peerDescr.NetworkInterfaces().empty());
        const Vector<IfAddress>& v = peerDescr.NetworkInterfaces();
        for (size_t i = 0, n = v.size();i < n;++i)
        {
            char8 sphys[30];
            const IfAddress::PhysAddress& phys = v[i].PhysicalAddress();
            Snprintf(sphys, COUNT_OF(sphys), "%02X:%02X:%02X:%02X:%02X:%02X"
                , phys.data[0]
                , phys.data[1]
                , phys.data[2]
                , phys.data[3]
                , phys.data[4]
                , phys.data[5]);
            const QString text = QString("IP=%1, MAC=%2")
                .arg(v[i].Address().ToString().c_str())
                .arg(sphys);
            QStandardItem *subitem = new QStandardItem();
            subitem->setText(text);
            top->appendRow(subitem);
        }
    }
    {
        QStandardItem* top = new QStandardItem();
        top->setText("Available transports");
        item->appendRow(top);

        const Vector<NetConfig::TransportConfig>& tr = peerDescr.NetworkConfig().Transports();
        for (size_t i = 0, n = tr.size();i < n;++i)
        {
            const char* str = "!!!";
            if (tr[i].type == TRANSPORT_TCP)
                str = "TCP";
            const QString text = QString("%1 - %2")
                .arg(str)
                .arg(tr[i].endpoint.ToString().c_str());
            QStandardItem *subitem = new QStandardItem();
            subitem->setText(text);
            top->appendRow(subitem);
        }
    }
    {
        QStandardItem* top = new QStandardItem();
        top->setText("Available services");
        item->appendRow(top);

        const Vector<uint32>& serv = peerDescr.NetworkConfig().Services();
        for (size_t i = 0, n = serv.size();i < n;++i)
        {
            const char8* name = NetCore::Instance()->ServiceName(serv[i]);
            const QString text = name != NULL ? QString(name)
                                              : QString("Unknown service %1").arg(serv[i]);
            QStandardItem *subitem = new QStandardItem();
            subitem->setText(text);
            top->appendRow(subitem);
        }
    }
    return item;
}

void DeviceListController::DiscoverCallback(size_t buflen, const void* buffer, const DAVA::Net::Endpoint& endpoint)
{
    if (!AlreadyInModel(endpoint))
    {
        PeerDescription peer;
        if (peer.Deserialize(buffer, buflen) > 0)
        {
            QStandardItem *item = createDeviceItem(endpoint, peer);
            model->appendRow(item);
            /*if (view)
            {
                QTreeView *treeView = view->ItemView();
                treeView->expand(item->index());
            }*/
        }
    }
}

bool DeviceListController::AlreadyInModel(const Endpoint& endp) const
{
    IPAddress srcAddr = endp.Address();
    for (int i = 0, n = model->rowCount();i < n;++i)
    {
        QVariant v = model->item(i)->data(ROLE_SOURCE_ADDRESS);
        if (endp == v.value<Endpoint>())
        {
            return true;
        }
    }
    return false;
}
