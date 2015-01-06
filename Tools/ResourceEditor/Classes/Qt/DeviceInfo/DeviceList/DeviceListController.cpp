#include "DeviceListController.h"


#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QUuid>

#include "DeviceListWidget.h"

#include "Classes/Qt/DeviceInfo/DeviceInfo/DeviceInfoController.h"
#include <Base/FunctionTraits.h>

#include <Network/PeerDesription.h>

using namespace DAVA;
using namespace DAVA::Net;

DeviceListController::DeviceListController( QObject* parent )
    : QObject(parent)
    , model( NULL )
    , idDiscoverer(DAVA::Net::NetCore::INVALID_TRACK_ID)
    , idDevice(DAVA::Net::NetCore::INVALID_TRACK_ID)
    , infoCtrl(NULL)
{
    initModel();

    static bool serviceRegisteres = false;
    if (!serviceRegisteres)
    {
        NetCore::Instance()->RegisterService(0, MakeFunction(this, &DeviceListController::CreateLogger), MakeFunction(this, &DeviceListController::DeleteLogger));
        serviceRegisteres = true;
    }

    DAVA::Net::Endpoint endpoint("239.192.100.1", 9999);
    idDiscoverer = DAVA::Net::NetCore::Instance()->CreateDiscoverer(endpoint, DAVA::MakeFunction(this, &DeviceListController::DiscoverCallback));
}

DeviceListController::~DeviceListController()
{
    if (idDiscoverer != DAVA::Net::NetCore::INVALID_TRACK_ID)
        DAVA::Net::NetCore::Instance()->DestroyController(idDiscoverer);
}

void DeviceListController::SetView( DeviceListWidget* _view )
{
    view = _view;
    view->ItemView()->setModel( model );

    connect( view, &DeviceListWidget::connectClicked, this, &DeviceListController::OnConnectDevice );
    connect( view, &DeviceListWidget::disconnectClicked, this, &DeviceListController::OnDisconnectDevice );
    connect( view, &DeviceListWidget::showInfoClicked, this, &DeviceListController::OnShowInfo );
}

void DeviceListController::initModel()
{
    delete model;
    model = new QStandardItemModel( this );
}

QStandardItem* DeviceListController::GetItemFromIndex( const QModelIndex& index )
{
    return model->itemFromIndex( index );
}

DAVA::Net::NetCore::TrackId DeviceListController::ConnectDeviceInternal(const DAVA::Net::PeerDescription& peer)
{
    IPAddress addr = peer.NetworkInterfaces()[0].Address();
    NetConfig config = peer.NetworkConfig().Mirror(addr);
    NetCore::TrackId id = NetCore::Instance()->CreateController(config);
    if (id != NetCore::INVALID_TRACK_ID)
    {
        infoCtrl = new DeviceInfoController(view, this);
    }
    return id;
}

void DeviceListController::DisonnectDeviceInternal(DAVA::Net::NetCore::TrackId id)
{
    NetCore::Instance()->DestroyController(id);
}

IChannelListener* DeviceListController::CreateLogger(uint32 serviceId)
{
    if (infoCtrl)
        return infoCtrl;
    return NULL;
}

void DeviceListController::DeleteLogger(IChannelListener*)
{
    if (idDevice == NetCore::INVALID_TRACK_ID)
    {
        delete infoCtrl;
        infoCtrl = NULL;
    }
}

void DeviceListController::AddDeviceInfo( QStandardItem* item )
{
    model->appendRow( item );
}

void DeviceListController::OnConnectDevice()
{
    const QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();

    for ( int i = 0; i < selection.size(); i++ )
    {
        const QModelIndex& index = selection[i];
        if ( index.parent().isValid() )
            continue;

        if (NetCore::INVALID_TRACK_ID == idDevice)
        {
            QVariant v = index.data(ROLE_PEER_DESCRIPTION);
            PeerDescription peer = v.value<PeerDescription>();
            curDescr = peer;
            idDevice = ConnectDeviceInternal(peer);
        }
    }
}

void DeviceListController::OnDisconnectDevice()
{
    const QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();

    for ( int i = 0; i < selection.size(); i++ )
    {
        const QModelIndex& index = selection[i];
        if ( index.parent().isValid() )
            continue;

        if (NetCore::INVALID_TRACK_ID != idDevice)
        {
            NetCore::TrackId temp = idDevice;
            idDevice = NetCore::INVALID_TRACK_ID;
            DisonnectDeviceInternal(temp);
        }
    }
}

void DeviceListController::OnShowInfo()
{
    if (infoCtrl)
    {
        infoCtrl->InitView();
    }
    /*const QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();
    QModelIndexList list;

    for ( int i = 0; i < selection.size(); i++ )
    {
        const QModelIndex& index = selection[i];
        if ( !index.parent().isValid() )
        {
            list << index;
        }
    }

    for ( int i = 0; i < list.size(); i++ )
    {
        DeviceInfoController *c = new DeviceInfoController( view, this );
    }*/
}

QStandardItem *DeviceListController::createDeviceItem(const Endpoint& endp, const PeerDescription& peerDescr)
{
    QStandardItem *item = new QStandardItem();

    //const QString text = QString("%1: %2 (%3)").arg(peerDescr.GetPlatformString().c_str()).arg(peerDescr.GetName().c_str()).arg(endp.Address().ToString().c_str());
    const QString text = QString("%1 - %2").arg(peerDescr.GetPlatformString().c_str()).arg(peerDescr.GetName().c_str());
    item->setText(text);

    {
        QVariant v;
        v.setValue(endp.Address());
        item->setData(v, ROLE_SOURCE_ADDRESS);
    }
    {
        QVariant v;
        v.setValue(peerDescr);
        item->setData(v, ROLE_PEER_DESCRIPTION);
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
        IPAddress aa;
        if (!peerDescr.NetworkInterfaces().empty())
            aa = peerDescr.NetworkInterfaces()[0].Address();
        QString a(aa.ToString().c_str());
        const Vector<NetConfig::TransportConfig>& tr = peerDescr.NetworkConfig().Transports();
        for (size_t i = 0, n = tr.size();i < n;++i)
        {
            const char* s = "";
            if (tr[i].type == TRANSPORT_TCP)
                s = "TCP";
            const QString text = QString("%1; %2:%3").arg(s).arg(a).arg(tr[i].endpoint.Port());
            QStandardItem *subitem = new QStandardItem();
            subitem->setText(text);
            item->appendRow(subitem);
        }
        const Vector<uint32>& serv = peerDescr.NetworkConfig().Services();
        QString s;
        for (size_t i = 0, n = serv.size();i < n;++i)
        {
            s += QString("; %1").arg(serv[i]);
        }
        {
            QStandardItem *subitem = new QStandardItem();
            subitem->setText(s);
            item->appendRow(subitem);
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

            if (view)
            {
                QTreeView *treeView = view->ItemView();
                treeView->expand(item->index());
            }
        }
    }
}

bool DeviceListController::AlreadyInModel(const Endpoint& endp) const
{
    IPAddress srcAddr = endp.Address();
    int n = model->rowCount();
    for (int i = 0;i < n;++i)
    {
        QStandardItem* item = model->item(i);

        QVariant v = item->data(ROLE_SOURCE_ADDRESS);
        IPAddress addr = v.value<IPAddress>();
        if (addr == srcAddr)
        {
            return true;
        }
    }
    return false;
}
