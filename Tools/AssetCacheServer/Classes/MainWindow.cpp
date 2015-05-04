#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "RemoteAssetCacheServer.h"
#include <FileSystem/KeyedArchive.h>

#include <QFileDialog>
#include <QMenu>
#include <QCloseEvent>
#include <QSettings>
#include <QTimer>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , trayIcon(nullptr)
{
    ui->setupUi(this);

    connect(ui->addNewServerButton, &QPushButton::clicked,
            this, &MainWindow::OnAddNewServerWidget);
    connect(ui->selectFolderButton, &QPushButton::clicked,
            this, &MainWindow::OnSelectFolder);
    connect(ui->clearDirectoryButton, &QPushButton::clicked,
            ui->cachFolderLineEdit, &QLineEdit::clear);

    connect(ui->cachFolderLineEdit, &QLineEdit::textChanged,
            this, &MainWindow::CheckEnableClearButton);

    ShowTrayIcon();

    WriteSettings();
    ReadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e)
    hide();
    e->ignore();
    openAction->setEnabled(true);
}

void MainWindow::OnAddNewServerWidget()
{
    RemoteAssetCacheServer *server = new RemoteAssetCacheServer(this);
    servers << server;

    connect(server, &RemoteAssetCacheServer::RemoveLater,
            this, &MainWindow::OnRemoveServerWidget);

    ui->serversBox->layout()->addWidget(server);
    emit NewServerAdded(server->GetServerData());
}

void MainWindow::OnRemoveServerWidget()
{
    RemoteAssetCacheServer *w = qobject_cast<RemoteAssetCacheServer *>(sender());
    servers.removeOne(w);
    emit ServerRemoved(w->GetServerData());
    w->deleteLater();
    QTimer::singleShot(10, [this]{adjustSize();});
    adjustSize();
}

void MainWindow::OnSelectFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, "Choose directory", QDir::currentPath(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->cachFolderLineEdit->setText(directory);
    emit FolderChanged(directory);
}

void MainWindow::CheckEnableClearButton()
{
    ui->clearDirectoryButton->setEnabled(!ui->cachFolderLineEdit->text().isEmpty());
}

void MainWindow::OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::DoubleClick:
        {
            if (!this->isVisible())
            {
                this->show();
            }
            break;
        }
        default:
            break;
    }
}

void MainWindow::OnServerParametersChanged()
{
    QVector<ServerData> serversData;
    for (auto server : servers)
    {
        serversData << server->GetServerData();
    }
    emit ServersChanged(serversData);
}

void MainWindow::OnOpenAction()
{
    if (!this->isVisible())
    {
        this->show();
        openAction->setEnabled(false);
    }
}

void MainWindow::SetFolder(QString &folderPath)
{
    if (QString::compare(folderPath, ui->cachFolderLineEdit->text(), Qt::CaseInsensitive) != 0)
    {
        ui->cachFolderLineEdit->setText(folderPath);
    }
}

void MainWindow::SetFolderSize(qreal folderSize)
{
    ui->cachSizeSpinBox->setValue(folderSize);
}

void MainWindow::SetFilesCount(quint32 filesCounts)
{
    if (ui->numberOfFilesSpinBox->value() != filesCounts)
    {
        ui->numberOfFilesSpinBox->setValue(filesCounts);
    }
}

void MainWindow::AddServers(QVector<ServerData> &newServers)
{
    for (auto newServer : newServers)
    {
        AddServer(newServer);
    }
}

void MainWindow::AddServer(ServerData newServer)
{
    RemoteAssetCacheServer *server = new RemoteAssetCacheServer(newServer, this);
    servers << server;

    connect(server, &RemoteAssetCacheServer::RemoveLater,
            this, &MainWindow::OnRemoveServerWidget);

    ui->serversBox->layout()->addWidget(server);

    emit NewServerAdded(server->GetServerData());
}

void MainWindow::CreateTrayIconActions()
{
    QAction *quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    openAction = new QAction("Open server", this);
    openAction->setEnabled(false);
    connect(openAction, &QAction::triggered, this, &MainWindow::OnOpenAction);

    trayActionsMenu = new QMenu(this);
    trayActionsMenu->addAction(openAction);
    trayActionsMenu->addSeparator();
    trayActionsMenu->addAction(quitAction);
}

void MainWindow::ShowTrayIcon()
{
    trayIcon = new QSystemTrayIcon(this);
    QIcon trayImage(":/icon/TrayIcon.png");
    trayIcon->setIcon(trayImage);

    CreateTrayIconActions();
    trayIcon->setContextMenu(trayActionsMenu);

    trayIcon->setToolTip("Asset Cache Server");

    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::OnTrayIconActivated);

    trayIcon->show();
}

void MainWindow::ReadSettings()
{
    using namespace DAVA;
    KeyedArchive *arch = new KeyedArchive();
    FilePath path("ACS_settings.dat");
    File *f = File::Create(path, File::OPEN | File::READ);
    arch->Load(f);

    String folderPath = arch->GetString(String("FolderPath"));
    ui->cachFolderLineEdit->setText(folderPath.c_str());

    DAVA::float32 folderSize = arch->GetFloat(String("FolderSize"));
    ui->cachSizeSpinBox->setValue(folderSize);

    //DAVA::Vector<ServerData> servers;
    //servers = arch->GetByteArrayAsType<DAVA::Vector<ServerData>>(String("Servers"));
    //QString ip = servers[0].ip;

    arch->Release();
}

void MainWindow::WriteSettings()
{
    using namespace DAVA;
    KeyedArchive *arch = new KeyedArchive();

    arch->SetString(String("FolderPath"), String(ui->cachFolderLineEdit->text().toStdString()));
    arch->SetFloat(String("FolderSize"), static_cast<DAVA::float32>(ui->cachSizeSpinBox->value()));
    arch->SetInt32(String("NumberOfFiles"), static_cast<DAVA::int32>(ui->numberOfFilesSpinBox->value()));
    arch->SetBool(String("Startup"), ui->startupCheckBox->isChecked());
    arch->SetInt32(String("Port"), static_cast<DAVA::int32>(ui->portSpinBox->value()));

    DAVA::Vector<ServerData> servers;
    servers.push_back(ServerData("127.0.0.1", 80));
    arch->SetByteArrayAsType<DAVA::Vector<ServerData>>(String("Servers"), servers);

    FilePath path("ACS_settings.dat");
    File *f = File::Create(path, File::CREATE | File::WRITE);
    arch->Save(f);
    f->Release();
    arch->Release();
}
