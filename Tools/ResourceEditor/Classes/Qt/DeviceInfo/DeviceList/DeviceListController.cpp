#include "DeviceListController.h"


#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QUuid>

#include "DeviceListWidget.h"

#include "Classes/Qt/DeviceInfo/DeviceInfo/DeviceInfoController.h"


DeviceListController::DeviceListController( QObject* parent )
    : QObject(parent)
    , model( NULL )
{
    initModel();

    NewDeviceCallback();
    NewDeviceCallback();
    NewDeviceCallback();
    NewDeviceCallback();
    NewDeviceCallback();
}

DeviceListController::~DeviceListController()
{
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

void DeviceListController::ConnectDeviceInternal( quintptr id )
{
    Q_UNUSED( id );
    qDebug() << "Connect: " << id;
    // TODO: connect device with id
}

void DeviceListController::DisonnectDeviceInternal( quintptr id )
{
    Q_UNUSED( id );
    qDebug() << "Disconnect: " << id;
    // TODO: disconnect device with id
}

void DeviceListController::AddDeviceInfo( QStandardItem* item )
{
    model->appendRow( item );
}

void DeviceListController::NewDeviceCallback()
{
    const quintptr someId = model->rowCount() + 1;
    const QString someText = QUuid::createUuid().toString();
    QStandardItem *item = createDeviceItem( someId, someText );

    AddDeviceInfo( item );

    if ( view )
    {
        QTreeView *treeView = view->ItemView();
        treeView->expand( item->index() );
    }
}

void DeviceListController::OnConnectDevice()
{
    const QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();

    for ( int i = 0; i < selection.size(); i++ )
    {
        const QModelIndex& index = selection[i];
        if ( index.parent().isValid() )
            continue;

        const quintptr id = index.data( DeviceId ).toULongLong();
        ConnectDeviceInternal( id );
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

        const quintptr id = index.data( DeviceId ).toULongLong();
        DisonnectDeviceInternal( id );
    }
}

void DeviceListController::OnShowInfo()
{
    const QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();
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
    }
}

// Передать сюда нужное количество аргументов и сформировать иерархию
QStandardItem* DeviceListController::createDeviceItem( quintptr id, const QString& title )
{
    QStandardItem *item = new QStandardItem();

    item->setText( title );
    item->setData( id, DeviceId );

    // Вложенные элементы:
    {
        const QString text = QString( "IP address: %1" ).arg( "127.0.0.1:27600" );
        QStandardItem *netInfo = new QStandardItem();
        netInfo->setText( text );
        item->appendRow( netInfo );
    }
    {
        const QString text = QString( "OS Version: %1" ).arg( "Android 5.0" );
        QStandardItem *osInfo = new QStandardItem();
        osInfo->setText( text );
        item->appendRow( osInfo );
    }

    SomeInfo info;
    QVariant v;
    v.setValue( info );

    item->setData( v, CustomData );

    v = item->data( CustomData );
    info = v.value<SomeInfo>();

    return item;
}
