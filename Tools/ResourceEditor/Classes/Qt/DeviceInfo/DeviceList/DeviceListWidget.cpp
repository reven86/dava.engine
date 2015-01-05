#include "DeviceListWidget.h"

#include "ui_DeviceListWidget.h"


#include <QDebug>


DeviceListWidget::DeviceListWidget( QWidget *parent )
    : QWidget( parent, Qt::Window )
    , ui( new Ui::DeviceListWidget() )
{
    ui->setupUi( this );

    connect( ui->addFake, &QPushButton::clicked, this, &DeviceListWidget::addClicked );
    connect( ui->removeFake, &QPushButton::clicked, this, &DeviceListWidget::removeClicked );
}

DeviceListWidget::~DeviceListWidget()
{
}

QTreeView* DeviceListWidget::ItemView()
{
    return ui->view;
}
