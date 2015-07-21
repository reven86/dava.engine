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

#include "AssetCache/AssetCacheConstants.h"

#include "ApplicationSettings.h"
#include "RemoteAssetCacheServer.h"

#include "FileSystem/KeyedArchive.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/Logger.h"
#include "Job/JobManager.h"
#include "QtTools/FileDialog/FileDialog.h"

#include <QFileDialog>
#include <QMenu>
#include <QCloseEvent>
#include <QSettings>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>

String DEFAULT_REMOTE_IP = "127.0.0.1";
uint16 DEFAULT_REMOTE_PORT = DAVA::AssetCache::ASSET_SERVER_PORT;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , settings(nullptr)
    , serverCore(nullptr)
{
    ui->setupUi(this);

    connect(ui->cacheFolderLineEdit, &QLineEdit::textChanged, this, &MainWindow::OnFolderTextChanged);
    connect(ui->selectFolderButton, &QPushButton::clicked, this, &MainWindow::OnFolderSelected);
    connect(ui->clearDirectoryButton, &QPushButton::clicked, ui->cacheFolderLineEdit, &QLineEdit::clear);
    connect(ui->cacheSizeSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnCacheSizeChanged(double)));
    connect(ui->numberOfFilesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnNumberOfFilesChanged(int)));
    connect(ui->autoSaveTimeoutSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnAutoSaveTimeoutChanged(int)));
    connect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnPortChanged(int)));
    connect(ui->autoStartCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnAutoStartChanged(int)));

    connect(ui->addNewServerButton, &QPushButton::clicked, this, &MainWindow::OnRemoteServerAdded);

    connect(ui->applyButton, &QPushButton::clicked, this, &MainWindow::OnApplyButtonClicked);
    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::OnCloseButtonClicked);

    serversBoxLayout = new QVBoxLayout();
    ui->scrollAreaWidgetContents->setLayout(serversBoxLayout);
    ui->scrollAreaWidgetContents->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

    CreateTrayIcon();

    ChangeSettingsState(NOT_EDITED);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::CreateTrayIcon()
{
    startAction = new QAction("Start server", this);
    connect(startAction, &QAction::triggered, this, &MainWindow::OnStartAction);

    stopAction = new QAction("Stop server", this);
    connect(stopAction, &QAction::triggered, this, &MainWindow::OnStopAction);

    QAction *editAction = new QAction("Edit settings", this);
    connect(editAction, &QAction::triggered, this, &MainWindow::OnEditAction);

    QAction *quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, qApp, QApplication::quit);

    QMenu *trayActionsMenu = new QMenu(this);
    trayActionsMenu->addAction(startAction);
    trayActionsMenu->addAction(stopAction);
    trayActionsMenu->addSeparator();
    trayActionsMenu->addAction(editAction);
    trayActionsMenu->addSeparator();
    trayActionsMenu->addAction(quitAction);

    QIcon windowIcon(":/icon/TrayIcon.png");
    setWindowIcon(windowIcon);

    greenGreenTrayIcon.reset(new QIcon(":/icon/TrayIcon_green_green.png"));
    greenGrayTrayIcon.reset(new QIcon(":/icon/TrayIcon_green_gray.png"));
    greenRedTrayIcon.reset(new QIcon(":/icon/TrayIcon_green_red.png"));
    redGrayTrayIcon.reset(new QIcon(":/icon/TrayIcon_red_gray.png"));

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(*redGrayTrayIcon);
    trayIcon->setToolTip("Asset Cache Server");
    trayIcon->setContextMenu(trayActionsMenu);
    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::OnTrayIconActivated);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    hide();
    e->ignore();

    if (settingsState != NOT_EDITED)
    {
        LoadSettings(settings);
    }
}

void MainWindow::SetServerCore(ServerCore* server)
{
    serverCore = server;
    if (serverCore)
    {
        connect(serverCore, &ServerCore::ServerStateChanged, this, &MainWindow::OnServerStateChanged);
        LoadSettings(serverCore->GetSettings());
        OnServerStateChanged(serverCore);
    }
}

void MainWindow::ChangeSettingsState(SettingsState newState)
{
    settingsState = newState;
    ui->applyButton->setEnabled(settingsState == VERIFIED);
}

void MainWindow::OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::DoubleClick:
    {
        showNormal();
        activateWindow();
        raise();
        break;
    }
    default:
        break;
    }
}

void MainWindow::OnFolderSelected()
{
    QString directory = FileDialog::getExistingDirectory(this, "Choose directory", QDir::currentPath(),
                                                         QFileDialog::ShowDirsOnly);
    ui->cacheFolderLineEdit->setText(directory);
    
    VerifyData();
}

void MainWindow::OnFolderTextChanged()
{
    ui->clearDirectoryButton->setEnabled(!ui->cacheFolderLineEdit->text().isEmpty());
    ui->cacheFolderLineEdit->setFocus();
    VerifyData();
}

void MainWindow::OnCacheSizeChanged(double)
{
    VerifyData();
}

void MainWindow::OnNumberOfFilesChanged(int)
{
    VerifyData();
}

void MainWindow::OnAutoSaveTimeoutChanged(int)
{
    VerifyData();
}

void MainWindow::OnPortChanged(int)
{
    VerifyData();
}

void MainWindow::OnAutoStartChanged(int)
{
    VerifyData();
}

void MainWindow::OnRemoteServerAdded()
{
    AddRemoteServer(ServerData(DEFAULT_REMOTE_IP, DEFAULT_REMOTE_PORT, false));
    VerifyData();
}

void MainWindow::OnRemoteServerRemoved()
{
    RemoteAssetCacheServer *remoteServer = qobject_cast<RemoteAssetCacheServer *>(sender());
    remoteServers.remove(remoteServer);

    remoteServer->deleteLater();
    VerifyData();
}

void MainWindow::OnRemoteServerEdited()
{
    VerifyData();
}

void MainWindow::OnRemoteServerChecked(bool checked)
{
    if (checked)
    {
        RemoteAssetCacheServer *checkedServer = qobject_cast<RemoteAssetCacheServer *>(sender());
        for (auto& nextServer : remoteServers)
        {
            if (nextServer->IsChecked() && nextServer != checkedServer)
            {
                nextServer->SetChecked(false);
                break;
            }
        }
    }

    VerifyData();
}

void MainWindow::OnEditAction()
{
    this->show();
    this->raise();
}

void MainWindow::OnStartAction()
{
    if (serverCore)
    {
        serverCore->Start();
    }
}

void MainWindow::OnStopAction()
{
    if (serverCore)
    {
        serverCore->Stop();
    }
}

void MainWindow::AddRemoteServer(const ServerData & newServer)
{
    RemoteAssetCacheServer *server = new RemoteAssetCacheServer(newServer, this);
    remoteServers.push_back(server);

    connect(server, &RemoteAssetCacheServer::RemoveLater, this, &MainWindow::OnRemoteServerRemoved);
    connect(server, &RemoteAssetCacheServer::ParametersChanged, this, &MainWindow::OnRemoteServerEdited);
    connect(server, SIGNAL(ServerChecked(bool)), this, SLOT(OnRemoteServerChecked(bool)));

    serversBoxLayout->insertWidget(serversBoxLayout->count() - 1, server);

    VerifyData();
}

void MainWindow::RemoveServers()
{
    while (!remoteServers.empty())
    {
        remoteServers.front()->deleteLater();
        remoteServers.pop_front();
    }
}

void MainWindow::VerifyData()
{
    for (auto &server : remoteServers)
    {
        if (!server->IsCorrectData())
        {
            ChangeSettingsState(NOT_VERIFIED);
            return;
        }
    }

    ChangeSettingsState(VERIFIED);
}

void MainWindow::OnApplyButtonClicked()
{
    SaveSettings();
}

void MainWindow::OnCloseButtonClicked()
{
    this->hide();

    if (settingsState != NOT_EDITED)
    {
        LoadSettings(settings);
    }
}

void MainWindow::SaveSettings()
{
    settings->SetFolder(ui->cacheFolderLineEdit->text().toStdString());
    settings->SetCacheSizeGb(ui->cacheSizeSpinBox->value());
    settings->SetFilesCount(ui->numberOfFilesSpinBox->value());
    settings->SetAutoSaveTimeoutMin(ui->autoSaveTimeoutSpinBox->value());
    settings->SetPort(ui->portSpinBox->value());
    settings->SetAutoStart(ui->autoStartCheckBox->isChecked());

    settings->ResetServers();
    for (auto &server : remoteServers)
    {
        settings->AddServer(server->GetServerData());
    }

    settings->Save();
    ChangeSettingsState(NOT_EDITED);
}

void MainWindow::LoadSettings(ApplicationSettings *_settings)
{
    settings = _settings;
    if(settings == nullptr)
        return;
    
    bool blocked = this->blockSignals(true);

    ui->cacheFolderLineEdit->setText(settings->GetFolder().GetAbsolutePathname().c_str());
    ui->cacheSizeSpinBox->setValue(settings->GetCacheSizeGb());
    ui->numberOfFilesSpinBox->setValue(settings->GetFilesCount());
    ui->autoSaveTimeoutSpinBox->setValue(settings->GetAutoSaveTimeoutMin());
    ui->portSpinBox->setValue(settings->GetPort());
    ui->autoStartCheckBox->setChecked(settings->IsAutoStart());
    
    RemoveServers();
    auto& servers = settings->GetServers();
    for (auto& sd: servers)
    {
        AddRemoteServer(sd);
    }
    
    this->blockSignals(blocked);

    ChangeSettingsState(NOT_EDITED);
}

void MainWindow::OnServerStateChanged(const ServerCore* server)
{
    DVASSERT(serverCore);
    DVASSERT(serverCore == server && "Notification from alien server core");

    auto serverState = serverCore->GetState();
    auto remoteState = serverCore->GetRemoteState();

    switch(serverState)
    {
    case ServerCore::State::STARTED:
    {
        switch (remoteState)
        {
        case ServerCore::RemoteState::STARTED:
        {
            trayIcon->setIcon(*greenGreenTrayIcon);
            break;
        }
        case ServerCore::RemoteState::STARTING:
        {
            trayIcon->setIcon(*greenRedTrayIcon);
            break;
        }
        case ServerCore::RemoteState::STOPPED:
        {
            trayIcon->setIcon(*greenGrayTrayIcon);
            break;
        }
        default:
        {
            DVASSERT(false && "Unknown remote state");
        }
        }

        startAction->setDisabled(true);
        stopAction->setEnabled(true);
        break;
    }
    case ServerCore::State::STOPPED:
    {
        trayIcon->setIcon(*redGrayTrayIcon);
        startAction->setEnabled(true);
        stopAction->setDisabled(true);
        break;
    }
    }
}
