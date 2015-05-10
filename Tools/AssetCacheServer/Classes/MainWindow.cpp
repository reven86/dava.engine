/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "RemoteAssetCacheServer.h"
#include <FileSystem/KeyedArchive.h>
#include "FileSystem/FileSystem.h"
#include "FileSystem/Logger.h"

#include <QFileDialog>
#include <QMenu>
#include <QCloseEvent>
#include <QSettings>
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , trayIcon(nullptr)
{
    ui->setupUi(this);

    connect(ui->addNewServerButton, &QPushButton::clicked, this, &MainWindow::OnAddNewServerWidget);
    connect(ui->selectFolderButton, &QPushButton::clicked, this, &MainWindow::OnSelectFolder);
    connect(ui->clearDirectoryButton, &QPushButton::clicked, ui->cachFolderLineEdit, &QLineEdit::clear);

    connect(ui->cachFolderLineEdit, &QLineEdit::textChanged, this, &MainWindow::CheckEnableClearButton);

    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::OnSaveButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &MainWindow::OnCancelButtonClicked);

    connect(ui->startupCheckBox, &QCheckBox::stateChanged, this, &MainWindow::OnStartupChanged);

    boxLayout = new QVBoxLayout();
    ui->scrollAreaWidgetContents_2->setLayout(boxLayout);
    ui->scrollAreaWidgetContents_2->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

    ReadSettings();

    ShowTrayIcon();

    ui->saveButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    WriteSettings();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    hide();
    e->ignore();
}

void MainWindow::OnAddNewServerWidget()
{
    AddServer(ServerData());
    VerifyData();
}

void MainWindow::OnRemoveServerWidget()
{
    RemoteAssetCacheServer *w = qobject_cast<RemoteAssetCacheServer *>(sender());
    servers.removeOne(w);
    emit ServerRemoved(w->GetServerData());
    w->deleteLater();
    VerifyData();
}

void MainWindow::OnSelectFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, "Choose directory", QDir::currentPath(),
                                                           QFileDialog::ShowDirsOnly);
    ui->cachFolderLineEdit->setText(directory);
    emit FolderChanged(directory);
    VerifyData();
}

void MainWindow::CheckEnableClearButton()
{
    ui->clearDirectoryButton->setEnabled(!ui->cachFolderLineEdit->text().isEmpty());
    ui->cachFolderLineEdit->setFocus();
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
    for (auto &server : servers)
    {
        serversData << server->GetServerData();
    }
    emit ServersChanged(serversData);
    VerifyData();
}

void MainWindow::OnOpenAction()
{
    this->raise();
    this->show();
}

void MainWindow::OnStartupChanged(int state)
{
    if (state == Qt::Checked)
    {
#ifdef Q_OS_WIN
        QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                           QSettings::NativeFormat);
        settings.setValue(QDir::toNativeSeparators(qApp->applicationName()),
                          QDir::toNativeSeparators(qApp->applicationFilePath()));
#endif
    }
    else if (state == Qt::Unchecked)
    {
#ifdef Q_OS_WIN
        QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                           QSettings::NativeFormat);
        settings.remove(QDir::toNativeSeparators(qApp->applicationName()));
#endif
    }
    VerifyData();
}

void MainWindow::OnSaveButtonClicked()
{
    WriteSettings();
    this->hide();
}

void MainWindow::OnCancelButtonClicked()
{
    this->hide();
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
    for (auto &newServer : newServers)
    {
        AddServer(newServer);
    }
}

void MainWindow::AddServer(ServerData newServer)
{
    RemoteAssetCacheServer *server = new RemoteAssetCacheServer(newServer, this);
    servers << server;

    connect(server, &RemoteAssetCacheServer::RemoveLater, this, &MainWindow::OnRemoveServerWidget);
    connect(server, &RemoteAssetCacheServer::ParametersChanged, this, &MainWindow::OnServerParametersChanged);

    boxLayout->insertWidget(boxLayout->count() - 1, server);

    emit NewServerAdded(server->GetServerData());
    VerifyData();
}

void MainWindow::CreateTrayIconActions()
{
    QAction *quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, qApp, QApplication::quit);

    QAction *openAction = new QAction("Open server", this);
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

    FilePath path("~doc:/AssetServer/ACS_settings.dat");
    File *f = File::Create(path, File::OPEN | File::READ);
    //if (f == nullptr)
    {
        Logger::Error("File not open. %s. %s", String("MainWindow::ReadSettings").c_str(), path.GetAbsolutePathname().c_str());
        return;
    }

    KeyedArchive *arch = new KeyedArchive();
    arch->Load(f);

    String folderPath = arch->GetString(String("FolderPath"));
    ui->cachFolderLineEdit->setText(folderPath.c_str());

    DAVA::float32 folderSize = arch->GetFloat(String("FolderSize"));
    ui->cachSizeSpinBox->setValue(folderSize);

    DAVA::uint32 numberOfFiles = arch->GetUInt32(String("NumberOfFiles"));
    ui->numberOfFilesSpinBox->setValue(numberOfFiles);

    DAVA::uint32 port = arch->GetUInt32(String("Port"));
    ui->portSpinBox->setValue(port);

    auto size = arch->GetUInt32("ServersSize");
    for (int i = 0; i < size; ++i)
    {
        String ip = arch->GetString(Format("Server_%d_ip", i));
        DAVA::uint32 port = arch->GetUInt32(Format("Server_%d_port", i));
        ServerData sData(QString(ip.c_str()), static_cast<quint16>(port));
        AddServer(sData);
    }

    f->Release();
    arch->Release();

#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                       QSettings::NativeFormat);
    QString key = settings.value(QDir::toNativeSeparators(qApp->applicationName())).toString();
    if (!key.isEmpty())
    {
        ui->startupCheckBox->blockSignals(true);
        ui->startupCheckBox->setChecked(true);
        ui->startupCheckBox->blockSignals(false);
    }
#endif
}

void MainWindow::WriteSettings()
{
    using namespace DAVA;

    FileSystem::Instance()->CreateDirectory("~doc:/AssetServer", true);
    FilePath path("~doc:/AssetServer/ACS_settings.dat");
    File *f = File::Create(path, File::CREATE | File::WRITE);
    if (f == nullptr)
    {
        Logger::Error("File not open");
        return;
    }

    KeyedArchive *arch = new KeyedArchive();

    arch->SetString(String("FolderPath"), String(ui->cachFolderLineEdit->text().toStdString()));
    arch->SetFloat(String("FolderSize"), static_cast<DAVA::float32>(ui->cachSizeSpinBox->value()));
    arch->SetUInt32(String("NumberOfFiles"), static_cast<DAVA::uint32>(ui->numberOfFilesSpinBox->value()));
    arch->SetUInt32(String("Port"), static_cast<DAVA::uint32>(ui->portSpinBox->value()));

    auto size = servers.size();
    arch->SetUInt32("ServersSize", size);

    for (int i = 0; i < size; ++i)
    {
        auto sData = servers.at(i)->GetServerData();
        arch->SetString(Format("Server_%d_ip", i), String(sData.ip.toStdString()));
        arch->SetUInt32(Format("Server_%d_port", i), static_cast<DAVA::uint32>(sData.port));
    }

    arch->Save(f);
    f->Release();
    arch->Release();

    ui->saveButton->setEnabled(false);
}

void MainWindow::VerifyData()
{
    bool isCorrect = true;
    for (auto &server : servers)
    {
        if (!server->IsCorrectData())
        {
            isCorrect = false;
            break;
        }
    }
    ui->saveButton->setEnabled(isCorrect);
}
