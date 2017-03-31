#include "RemoteTool/Private/DeviceList/DeviceListWidget.h"
#include "ui_DeviceListWidget.h"

#include "RemoteTool/Private/MemoryTool/MemProfController.h"
#include "RemoteTool/Private/MemoryTool/ProfilingSession.h"
#include "RemoteTool/Private/MemoryTool/Widgets/MemProfWidget.h"

#include <FileSystem/FilePath.h>
#include <FileSystem/File.h>

#include <QDebug>
#include <QCloseEvent> 
#include <QFileDialog>
#include <QTreeView>
#include <QVBoxLayout>
#include <QTabWidget>

using namespace DAVA;

DeviceListWidget::DeviceListWidget(QWidget* parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::DeviceListWidget())
{
    ui->setupUi(this);

    connect(ui->connectDevice, &QPushButton::clicked, this, &DeviceListWidget::connectClicked);
    connect(ui->disconnectDevice, &QPushButton::clicked, this, &DeviceListWidget::disconnectClicked);
    connect(ui->showLog, &QPushButton::clicked, this, &DeviceListWidget::showLogClicked);

    connect(ui->viewDump, &QPushButton::clicked, this, &DeviceListWidget::OnViewDump);
    connect(ui->discoverDevice, &QPushButton::clicked, this, &DeviceListWidget::OnDeviceDiscover);
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
    DAVA::FilePath snapshotDir("~doc:/memory-profiling");
    QString filename = QFileDialog::getOpenFileName(this, "Select dump file", snapshotDir.GetAbsolutePathname().c_str(), "Memory logs (*.mlog)");
    if (!filename.isEmpty())
    {
        std::string s = filename.toStdString();
        MemProfController* obj = new MemProfController(FilePath(s), this);
        if (!obj->IsFileLoaded())
        {
            delete obj;
        }
    }
}

void DeviceListWidget::OnDeviceDiscover()
{
    QString s = ui->ipaddr->text().trimmed();
    if (!s.isEmpty())
    {
        emit deviceDiscoverClicked(s);
    }
}
