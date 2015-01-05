#include "DeviceListWidget.h"

#include "ui_DeviceListWidget.h"


#include <QDebug>


DeviceListWidget::DeviceListWidget( QWidget *parent )
    : QWidget( parent, Qt::Window )
    , ui( new Ui::DeviceListWidget() )
{
    ui->setupUi( this );

    connect( ui->connectDevice, &QPushButton::clicked, this, &DeviceListWidget::connectClicked );
    connect( ui->disconnectDevice, &QPushButton::clicked, this, &DeviceListWidget::disconnectClicked );
    connect( ui->showInfo, &QPushButton::clicked, this, &DeviceListWidget::showInfoClicked );
}

DeviceListWidget::~DeviceListWidget()
{
}

QTreeView* DeviceListWidget::ItemView()
{
    return ui->view;
}
