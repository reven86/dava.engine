#include "DeviceInfoWidget.h"

#include "ui_DeviceInfoWidget.h"


#include <QDebug>


DeviceInfoWidget::DeviceInfoWidget( QWidget *parent )
    : QWidget( parent, Qt::Window )
    , ui( new Ui::DeviceInfoWidget() )
{
    ui->setupUi( this );
}

DeviceInfoWidget::~DeviceInfoWidget()
{
}
