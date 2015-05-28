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

#include "ApplicationSettings.h"

#include "RemoteAssetCacheServer.h"
#include <FileSystem/KeyedArchive.h>
#include "FileSystem/FileSystem.h"
#include "FileSystem/Logger.h"
#include "QtTools/FileDialog/FileDialog.h"

#include "AssetCache/AssetCacheConstants.h"
#include "AssetCache/Test/AssetCacheTest.h"
#include "Job/JobManager.h"

#include <QFileDialog>
#include <QMenu>
#include <QCloseEvent>
#include <QSettings>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , trayIcon(nullptr)
    , settings(nullptr)
{
    ui->setupUi(this);

    connect(ui->addNewServerButton, &QPushButton::clicked, this, &MainWindow::OnAddNewServerWidget);
    connect(ui->selectFolderButton, &QPushButton::clicked, this, &MainWindow::OnSelectFolder);
    connect(ui->clearDirectoryButton, &QPushButton::clicked, ui->cacheFolderLineEdit, &QLineEdit::clear);

    connect(ui->cacheFolderLineEdit, &QLineEdit::textChanged, this, &MainWindow::CheckEnableClearButton);

    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::OnSaveButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &MainWindow::OnCancelButtonClicked);

    boxLayout = new QVBoxLayout();
    ui->scrollAreaWidgetContents_2->setLayout(boxLayout);
    ui->scrollAreaWidgetContents_2->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

    ui->portSpinBox->setValue(DAVA::AssetCache::ASSET_SERVER_PORT);
    ui->portSpinBox->setEnabled(false);
    
    ShowTrayIcon();

    ui->saveButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
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
    
    w->deleteLater();
    VerifyData();
}

void MainWindow::OnSelectFolder()
{
    QString directory = FileDialog::getExistingDirectory(this, "Choose directory", QDir::currentPath(),
                                                         QFileDialog::ShowDirsOnly);
    ui->cacheFolderLineEdit->setText(directory);
    
    VerifyData();
}

void MainWindow::CheckEnableClearButton()
{
    ui->clearDirectoryButton->setEnabled(!ui->cacheFolderLineEdit->text().isEmpty());
    ui->cacheFolderLineEdit->setFocus();
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

    VerifyData();
}

void MainWindow::OnOpenAction()
{
    this->raise();
    this->show();
    trayIcon->hide();
    trayIcon->show();
}


void MainWindow::OnCancelButtonClicked()
{
    this->hide();
}

void MainWindow::SetFolder(QString &folderPath)
{
    if (QString::compare(folderPath, ui->cacheFolderLineEdit->text(), Qt::CaseInsensitive) != 0)
    {
        ui->cacheFolderLineEdit->setText(folderPath);
    }
}

void MainWindow::SetFolderSize(qreal folderSize)
{
    ui->cacheSizeSpinBox->setValue(folderSize);
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

void MainWindow::AddServer(const ServerData & newServer)
{
    RemoteAssetCacheServer *server = new RemoteAssetCacheServer(newServer, this);
    servers << server;

    connect(server, &RemoteAssetCacheServer::RemoveLater, this, &MainWindow::OnRemoveServerWidget);
    connect(server, &RemoteAssetCacheServer::ParametersChanged, this, &MainWindow::OnServerParametersChanged);

    boxLayout->insertWidget(boxLayout->count() - 1, server);

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

void MainWindow::OnSaveButtonClicked()
{
    //get settings from UI
    settings->SetFolder(ui->cacheFolderLineEdit->text().toStdString());
    settings->SetCacheSize(ui->cacheSizeSpinBox->value() * 1024 * 1024 * 1024);
    settings->SetFilesCount(ui->numberOfFilesSpinBox->value());
    
    settings->ResetServers();
    for (auto &server : servers)
    {
        settings->AddServer(server->GetServerData());
    }
    
    settings->Save();
    
    this->hide();
}

void MainWindow::SetSettings(ApplicationSettings *_settings)
{
    settings = _settings;
    if(settings == nullptr)
        return;
    
    bool blocked = this->blockSignals(true);

    ui->cacheFolderLineEdit->setText(settings->GetFolder().GetAbsolutePathname().c_str());
    ui->cacheSizeSpinBox->setValue(settings->GetCacheSize() / (1 * 1024 * 1024 * 1024));
    ui->numberOfFilesSpinBox->setValue(settings->GetFilesCount());
    
    auto & servers = settings->GetServers();
    for (auto & sd: servers)
    {
        AddServer(sd);
    }
    
    this->blockSignals(blocked);
    
    VerifyData();
}
