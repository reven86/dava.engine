#include "DeviceListWidget.h"

#include "ui_DeviceListWidget.h"

#include <QDebug>
#include <QCloseEvent> 
#include <QFileDialog>
#include <QTreeView>
#include <QVBoxLayout>
#include <QTabWidget>

#include "FileSystem/FilePath.h"

#include "../DeviceInfo/DumpViewerWidget.h"
#include "../DeviceInfo/DumpSession.h"

#include "../DeviceInfo/Models/CallTreeModel.h"

#include "../DeviceInfo/ProfilingSession.h"
#include "../DeviceInfo/MemProfWidget.h"

DeviceListWidget::DeviceListWidget( QWidget *parent )
    : QWidget( parent, Qt::Window )
    , ui( new Ui::DeviceListWidget() )
{
    ui->setupUi( this );

    connect( ui->connectDevice, &QPushButton::clicked, this, &DeviceListWidget::connectClicked );
    connect( ui->disconnectDevice, &QPushButton::clicked, this, &DeviceListWidget::disconnectClicked );
    connect( ui->showLog, &QPushButton::clicked, this, &DeviceListWidget::showLogClicked );

    connect(ui->viewDump, &QPushButton::clicked, this, &DeviceListWidget::OnViewDump);
    connect(ui->viewDumpEnh, &QPushButton::clicked, this, &DeviceListWidget::OnViewDumpEnhanced);
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
    QString filename = QFileDialog::getOpenFileName(this, "Select dump file", "d:\\share\\dumps\\test", "Dumps (*.bin)");
    if (!filename.isEmpty())
    {
        std::string s = filename.toStdString();
        ProfilingSession* profilingSession = new ProfilingSession(DAVA::FilePath(s));
        MemProfWidget* w = new MemProfWidget(profilingSession, this);
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->resize(800, 600);
        w->show();
        /*
        QTabWidget* tab = new QTabWidget;
        {
            CallTreeModel* model = new CallTreeModel(s.c_str(), true);
            QTreeView* tree = new QTreeView;
            tree->setFont(QFont("Consolas", 10, 500));
            tree->setModel(model);

            tab->addTab(tree, "Back");
        }
        {
            CallTreeModel* model = new CallTreeModel(s.c_str(), false);
            QTreeView* tree = new QTreeView;
            tree->setFont(QFont("Consolas", 10, 500));
            tree->setModel(model);

            tab->addTab(tree, "Fwd");
        }
        {
            CallTreeModel* model = new CallTreeModel(s.c_str());
            QTreeView* tree = new QTreeView;
            tree->setFont(QFont("Consolas", 10, 500));
            tree->setModel(model);

            tab->addTab(tree, "Fwd Ex");
        }

        QVBoxLayout* l = new QVBoxLayout;
        l->addWidget(tab);

        QWidget* w = new QWidget(this, Qt::Window);
        w->setLayout(l);
        w->resize(800, 600);
        w->show();
        */
    }
}

void DeviceListWidget::OnViewDumpEnhanced()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select dump file", "d:\\share\\dumps\\test", "Dumps (*.bin)");
    if (!filename.isEmpty())
    {
        DumpViewerWidget* w = new DumpViewerWidget(filename.toStdString().c_str(), this);
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->resize(800, 600);
        w->show();
    }
}
