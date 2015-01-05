#include "DeviceListController.h"


#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QUuid>

#include "DeviceListWidget.h"


DeviceListController::DeviceListController( QObject* parent )
    : QObject(parent)
    , model( NULL )
{
    initModel();
}

DeviceListController::~DeviceListController()
{
}

void DeviceListController::SetView( DeviceListWidget* _view )
{
    view = _view;
    view->ItemView()->setModel( model );

    connect( view, &DeviceListWidget::addClicked, this, &DeviceListController::OnAddDevice );
    connect( view, &DeviceListWidget::removeClicked, this, &DeviceListController::OnRemoveDevice );
}

void DeviceListController::initModel()
{
    delete model;
    model = new QStandardItemModel( this );
}

void DeviceListController::AddDeviceInfo( QStandardItem* item )
{
    model->appendRow( item );
}

void DeviceListController::OnAddDevice()
{
    const quintptr someId = model->rowCount() + 1;
    const QString someText = QUuid::createUuid().toString();
    QStandardItem *item = createDeviceItem( someId, someText );

    AddDeviceInfo( item );
}

void DeviceListController::OnRemoveDevice()
{
    QTreeView *tree = view->ItemView();
    const QModelIndexList selection = tree->selectionModel()->selectedRows();
    QList<QStandardItem *> items;

    for ( int i = 0; i < selection.size(); i++ )
    {
        items << model->takeRow( selection[i].row() );
    }

    qDeleteAll( items );
}

QStandardItem* DeviceListController::createDeviceItem( quintptr id, const QString& title )
{
    QStandardItem *item = new QStandardItem();

    item->setText( title );
    item->setData( id, DeviceId );

    return item;
}
