#include "DeviceListWidget.h"

#include "ui_DeviceListWidget.h"

#include <QDebug>
#include <QCloseEvent> 
#include <QFileDialog>

#include "../DeviceInfo/DumpViewWidget.h"

DeviceListWidget::DeviceListWidget( QWidget *parent )
    : QWidget( parent, Qt::Window )
    , ui( new Ui::DeviceListWidget() )
{
    ui->setupUi( this );

    connect( ui->connectDevice, &QPushButton::clicked, this, &DeviceListWidget::connectClicked );
    connect( ui->disconnectDevice, &QPushButton::clicked, this, &DeviceListWidget::disconnectClicked );
    connect( ui->showLog, &QPushButton::clicked, this, &DeviceListWidget::showLogClicked );

    connect(ui->viewDump, &QPushButton::clicked, this, &DeviceListWidget::OnViewDump);
}

DeviceListWidget::~DeviceListWidget()
{
}

QTreeView* DeviceListWidget::ItemView()
{
    return ui->view;
}

void DeviceListWidget::OnViewDump()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select dump file", "d:\\share\\dumps\\test");
    //QString filename = "d:\\share\\dumps\\test\\01. dump-login-debug.bin";
    if (!filename.isEmpty())
    {
        std::string s = filename.toStdString();
        DumpViewWidget* w = new DumpViewWidget(s.c_str(), this, Qt::Window);
        w->resize(800, 600);
        w->show();
    }
}
