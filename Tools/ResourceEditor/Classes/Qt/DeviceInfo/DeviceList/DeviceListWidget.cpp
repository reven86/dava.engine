#include "DeviceListWidget.h"

#include "ui_DeviceListWidget.h"

#include <QDebug>
#include <QCloseEvent> 
#include <QFileDialog>
#include <QTreeView>
#include <QVBoxLayout>
#include <QTabWidget>

#include "FileSystem/FilePath.h"
#include "FileSystem/File.h"

#include "Classes/Qt/DeviceInfo/DeviceInfo/ProfilingSession.h"
#include "Classes/Qt/DeviceInfo/DeviceInfo/MemProfWidget.h"
#include "Classes/Qt/DeviceInfo/DeviceInfo/MemProfController.h"

using namespace DAVA;

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
    //DAVA::FilePath dumpDir("~doc:/memdumps/memory_dumps/2015-04-26 130711 win32-game-dumps");
    DAVA::FilePath dumpDir("/Users/max/projects/wot/xxx");
    //QString filename = QFileDialog::getOpenFileName(this, "Select dump file", dumpDir.GetAbsolutePathname().c_str(), "Snapshots (*.bin)");
    //if (!filename.isEmpty())
    {
        //std::string s = filename.toStdString();
        //FilePath srcDir(s);
        //new MemProfController(srcDir.GetDirectory(), this);
        new MemProfController(dumpDir, this);

        
        // ProfilingSession will be deleted on MemProfWidget destruction
        //ProfilingSession* profilingSession = new ProfilingSession(DAVA::FilePath(s));
        //MemProfWidget* w = new MemProfWidget(profilingSession, this);
        //w->setAttribute(Qt::WA_DeleteOnClose);
        //w->resize(800, 600);
        //w->show();
    }
}
